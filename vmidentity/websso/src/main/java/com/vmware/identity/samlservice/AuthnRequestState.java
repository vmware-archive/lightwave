/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

package com.vmware.identity.samlservice;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertPath;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Locale;
import java.util.UUID;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.IDPEntry;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.Response;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.xml.security.SecurityException;
import org.opensaml.xml.util.Base64;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.AuthnPolicy;
import com.vmware.identity.idm.IDMReferralException;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.AuthnRequestStateCookieWrapper;
import com.vmware.identity.samlservice.impl.AuthnRequestStateKerbAuthenticationFilter.KerbAuthnType;
import com.vmware.identity.samlservice.impl.AuthnRequestStateRsaAmAuthenticationFilter;
import com.vmware.identity.samlservice.impl.AuthnRequestStateValidator;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.util.TimePeriod;


/**
 * Object that encapsulates a lifetime of SAML AuthnRequest
 *
 */
public class AuthnRequestState {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(AuthnRequestState.class);

    private final DefaultIdmAccessorFactory factory;
    private DefaultSamlAuthorityFactory samlAuthFactory;
    private final IdmAccessor idmAccessor;
    private final SamlValidator<AuthnRequestState> validator;
    private final RequestCache requestCache;

    private final HttpServletRequest request;
    private final HttpServletResponse response;
    private final SessionManager sessionManager;
    private Locale locale;
    private MessageSource messageSource;
    private AuthnRequest authnRequest;
    private final String samlRequest;
    private String signedMessage;
    private final String relayState;
    private final String sigAlg;
    private final String signature;
    private ProcessingState processingState;
    private ValidationResult validationResult;
    private String wwwAuthenticate;
    private KerbAuthnType kerbAuthnType;
    private PrincipalId principalId;
    private String identityFormat;
    private String issuerValue;
    private String sessionId;
    private final String correlationId;
    private AuthnMethod authnMethod;
    private Date startTime;
    private boolean isRenewable;
    private boolean isDelegable;
    private boolean isExistingRequest;
    private Integer proxyCount;  //could be null: means not set
    private boolean isProxying;
    private Boolean needLoginView;
    private Boolean needChooseIDPView;
    private List<IDPEntry> idpList;
    private List<String> idpSelectionList; //list of entityId of idps for chooseIDPView
    private IDPConfig extIDPToUse;  //external IDP used if isProxying is true
    private Boolean isIDPSelectionEnabled;
    private AuthnTypesSupported authnTypesSupported;  //Authentication methods allowed. It is combination of
                                                      //tenant policy and RequestedAuthnContext with a SAMLRequest.
    private String acsUrl; // assertion consumer service URL to be used in
                           // constructing response
    /**
     * Accessor for processing state
     *
     * @return
     */
    public ProcessingState getProcessingState() {
        return this.processingState;
    }

    /**
     * Accessor for authn request
     *
     * @return
     */
    public AuthnRequest getAuthnRequest() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.authnRequest;
    }

    /**
     * Accessor for saml request
     *
     * @return
     */
    public String getSamlRequest() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.samlRequest;
    }

    /**
     * Accessor for relay state
     *
     * @return
     */
    public String getRelayState() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.relayState;
    }

    /**
     * Accessor for signature algorithm
     *
     * @return
     */
    public String getSignatureAlgorithm() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.sigAlg;
    }

    /**
     * Accessor for signature
     *
     * @return
     */
    public String getSignature() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.signature;
    }

    /**
     * Accessor for request
     *
     * @return
     */
    public HttpServletRequest getRequest() {
        return this.request;
    }

    /**
     * Accessor for session manager
     *
     * @return
     */
    public SessionManager getSessionManager() {
        return this.sessionManager;
    }

    /**
     * Accessor for validation result
     *
     * @return
     */
    public ValidationResult getValidationResult() {
        return this.validationResult;
    }

    /**
     * Setter for validation result
     *
     * @param vr
     */
    public void setValidationResult(ValidationResult vr) {
        this.validationResult = vr;
    }

    /**
     * Accessor for Idm Accessor
     *
     * @return
     */
    public IdmAccessor getIdmAccessor() {
        return this.idmAccessor;
    }

    public String getTenantIDPCookie() {
        return Shared.getCookieValue(request.getCookies(),
                Shared.getTenantIDPCookieName(idmAccessor.getTenant()), null);
    }

    public String getTenantIDPSelectHeader() {
        return this.request.getHeader(Shared.IDP_SELECTION_HEADER);
    }

    public List<String> getIDPSelectionEntityIdList() {
        Validate.noNullElements(idpSelectionList);
        return idpSelectionList;
    }

    public List<String> getIDPSelectionDisplayNameList(List<String> entityIdList) {
        Validate.noNullElements(entityIdList);
        // use idp alias as display name on choose idp web page
        List<String> displayNameList = new ArrayList<>();
        for (String entityId : entityIdList) {
            String displayName = idmAccessor.getIDPAlias(idmAccessor.getTenant(), entityId);
            if (displayName != null && !displayName.isEmpty()) {
                displayNameList.add(displayName);
            } else {
                displayNameList.add(entityId);
            }
        }
        return displayNameList;
    }

    public void setIDPEntityIdList(List<String> idpSelectionList) {
        Validate.noNullElements(idpSelectionList);
        this.idpSelectionList = idpSelectionList;
    }

    /**
     * Accessor for signed message
     *
     * @return
     */
    public String getSignedMessage() throws IllegalStateException {
        if (this.processingState == ProcessingState.UNKNOWN) {
            throw new IllegalStateException("Object state is unknown");
        }
        return this.signedMessage;
    }

    public AuthnRequestState(HttpServletRequest request, HttpServletResponse response,
            SessionManager sessionManager, String tenant) {

        Validate.notNull(request);
        Validate.notNull(response);
        Validate.notNull(sessionManager);
        this.processingState = ProcessingState.UNKNOWN;
        this.request = request;
        this.response = response;
        this.sessionManager = sessionManager;
        //TODO - check for correlation id in the headers PR1561606
        this.correlationId = UUID.randomUUID().toString();
        this.factory = new DefaultIdmAccessorFactory(this.correlationId);
        Validate.notNull(this.factory);
        this.idmAccessor = this.factory.getIdmAccessor();
        this.validator = new AuthnRequestStateValidator();
        RequestCacheFactory requestFactory = new DefaultRequestCacheFactory();
        this.requestCache = requestFactory.getRequestCache();

        this.relayState = request.getParameter(Shared.RELAY_STATE_PARAMETER);
        this.signature = request.getParameter(Shared.SIGNATURE_PARAMETER);
        this.sigAlg = request
                .getParameter(Shared.SIGNATURE_ALGORITHM_PARAMETER);
        this.samlRequest = request.getParameter(Shared.SAML_REQUEST_PARAMETER);

        //initialize authnTypesSupported based only on tenant policy.
        AuthnPolicy authnPolicy = this.idmAccessor.getAuthnPolicy(tenant);
        this.authnTypesSupported = new AuthnTypesSupported(authnPolicy.IsPasswordAuthEnabled()
                , authnPolicy.IsWindowsAuthEnabled(), authnPolicy.IsTLSClientCertAuthnEnabled(), authnPolicy.IsRsaSecureIDAuthnEnabled());
        Validate.notNull(this.samlRequest);

        // construct message that was supposed to be signed
        if (this.signature != null && this.sigAlg != null) {
            try {
                this.signedMessage = Shared.SAML_REQUEST_PARAMETER + "="
                        + URLEncoder.encode(this.samlRequest, "UTF-8");
                if (this.relayState != null) {
                    this.signedMessage = this.signedMessage + "&"
                            + Shared.RELAY_STATE_PARAMETER + "="
                            + URLEncoder.encode(this.relayState, "UTF-8");
                    // print out decoded relay state. Note that we do not need
                    // to
                    // store decoded value.
                    byte[] relayStateBytes = Base64.decode(this.relayState);
                    log.debug("Relay state specified was "
                            + new String(relayStateBytes));
                }
                this.signedMessage = this.signedMessage + "&"
                        + Shared.SIGNATURE_ALGORITHM_PARAMETER + "="
                        + URLEncoder.encode(this.sigAlg, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                log.error("Could not reconstruct signed message, exception: ",e);
                this.signedMessage = null;
            }
        }

        this.processingState = ProcessingState.INITIALIZED;
    }

    public void parseRequestForTenant(String tenant,
            AuthenticationFilter<AuthnRequestState> authenticator)
            throws IllegalStateException {
        log.debug("parseRequestForTenant, tenant " + tenant);

        Validate.notNull(this.idmAccessor);
        Validate.notNull(this.request);

        if (!(authenticator instanceof AuthnRequestStateRsaAmAuthenticationFilter ||
               ( authenticator instanceof AuthnRequestStateCookieWrapper &&
                       ((AuthnRequestStateCookieWrapper) authenticator).getAuthenticator() instanceof AuthnRequestStateRsaAmAuthenticationFilter)) ){
            // check for replays and resent request
            if (this.requestCache.shouldDenyRequest(this.samlRequest)) {
                log.info("Replay attack detected - DENYING authentication request");
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_FORBIDDEN, "Forbidden", "Replay");
                throw new IllegalStateException("Forbidden");
            } else {
                this.setIsExistingRequest(this.requestCache.isExistingRequest(this.samlRequest));
                this.requestCache.storeRequest(this.samlRequest);
            }
        }

        // relying party unknown at this point, specify null
        SamlService service = this.createSamlServiceForTenant(tenant, null);

        try {
            this.idmAccessor.setTenant(tenant);
            authenticator.preAuthenticate(this);
        } catch (SamlServiceException e) {
            log.error("Caught Saml Service Exception from preAuthenticate ", e);
            if (this.validationResult == null) {
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_FORBIDDEN, "Forbidden", null);
            }
            throw new IllegalStateException(e);
        }

        // decode request
        try {
            this.authnRequest = service.decodeSamlAuthnRequest(this.request);
        } catch (MessageDecodingException e) {
            // fail the validation with specific error code and rethrow
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            log.error("Unable to decode message " ,e);
            throw new IllegalStateException(e);
        } catch (SecurityException e) {
            // fail the validation with specific error code and rethrow
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            log.error("Unable to validate the authentication request ",e);
            throw new IllegalStateException(e);
        }

        // if signature was specified along with signing algorithm, verify
        // signature
        Issuer issuer = this.authnRequest.getIssuer();
        if (issuer == null || issuer.getValue() == null || this.idmAccessor.getRelyingPartyByUrl(issuer.getValue()) == null) {
            service = null;
        } else {
            this.setIssuerValue(issuer.getValue());
            service = this.createSamlServiceForTenant(tenant, this.getIssuerValue());
        }
        if (service == null) {
            // return 400 to the caller and throw
        	log.error("Could not recognize issuer.");
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", "Issuer");
            throw new IllegalStateException("Issuer not recognized");
        }
        if (this.sigAlg != null && this.signature != null) {
            try {
                service.verifySignature(this.signedMessage, this.signature);
            } catch (IllegalStateException e) {
                // fail the validation with specific error code and rethrow
                log.error("Could not validate the signature against message.",e);
                this.validationResult = new ValidationResult(
                        OasisNames.RESPONDER, OasisNames.REQUEST_DENIED);
                throw new IllegalStateException(e);
            }
        }

        this.setSamlAuthFactory(new DefaultSamlAuthorityFactory(
                SignatureAlgorithm.RSA_SHA256,
                authenticator
                        .getPrincipalAttributeExtractorFactory(Shared.IDM_HOSTNAME),
                authenticator
                        .getConfigExtractorFactory(Shared.IDM_HOSTNAME))); // TODO
                                                                          // use
                                                                          // actual
                                                                          // tenant
                                                                          // settings

        this.validationResult = this.validator.validate(this);

        if (this.validationResult.isValid()) {
            // mark as parsed and retrieve cookie
            this.processingState = ProcessingState.PARSED;
        }
    }

    /**
     * Perform authentication if needed and prepare a Document with a saml token
     * in it. Should not throw, but will set ValidationResult on exception. -
     * 401 UNAUTHORIZED if more auth data is needed - Response:Responder,
     * Internal processing error for unexpected exceptions - ...
     *
     * @param tenant
     * @return
     */
    public Document authenticate(String tenant,
            AuthenticationFilter<AuthnRequestState> authenticator) {
        Document retval = null;

        log.debug("authenticate, tenant " + tenant);

        Validate.notNull(this.idmAccessor);
        Validate.notNull(this.authnRequest);
        Validate.notNull(authenticator);

        // authentication call
        try {
            this.idmAccessor.setTenant(tenant);
            authenticator.authenticate(this);
        } catch (SamlServiceException e) {
            // more auth data is required
            log.error("Caught Saml Service Exception from authenticate "
                    + e.toString());
            if (this.getValidationResult() == null || this.getValidationResult().isValid()) {
                this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, "Unauthorized", null);
            }
            return null;
        } catch (Exception e) {
            // unexpected processing error
            log.error("Caught Exception from authenticate " + e.toString());
            if (this.getValidationResult() == null || this.getValidationResult().isValid()) {
                this.validationResult = new ValidationResult(OasisNames.RESPONDER);
            }
            return null;
        }

        if (this.getPrincipalId() == null || this.getIdentityFormat() == null
                || this.getSessionId() == null) {
            // assume that user could not be authenticated and validation result
            // was set by other code
            return null;
        }

        // get our token authority
        try {
            if (!this.isProxying) {
                this.idmAccessor.setTenant(tenant);
                retval = createToken();
            }
        } catch (SamlServiceException e) {
            log.error("Caught Saml Service Exception in creating token." + e);
            this.validationResult = new ValidationResult(OasisNames.RESPONDER);
        } catch (com.vmware.identity.saml.SystemException e) {
            log.error("Caught SystemException in creating token. ", e);
            if (e.getCause() instanceof IDMReferralException) {
                this.validationResult = new ValidationResult(HttpServletResponse.SC_NOT_IMPLEMENTED, "LdapReferralNotSupported", null);
            } else {
                this.validationResult = new ValidationResult(OasisNames.RESPONDER);
            }
        }

        return retval;
    }
    public Document createToken() throws SamlServiceException
    {
        Validate.notNull(this.idmAccessor);
        Validate.notNull(this.authnRequest);
        Validate.notNull(this.authnRequest.getIssuer());

        String relyingParty = this.authnRequest.getIssuer().getValue();

        String tenant = this.idmAccessor.getTenant();
        TokenAuthority authority = this.createTokenAuthorityForTenant(tenant);
        SamlTokenSpec tokenSpec = this.createTokenSpec(this.getIssuerValue(),
            this.getPrincipalId(), this.getIdentityFormat(),
            this.getAuthnMethod(),
            this.getSessionManager().get(this.getSessionId()),
            this.authnRequest.getID(),
            this.getAcsUrl(),
            relyingParty
                );
        Document retval = null;
        try {
            retval = authority.issueToken(tokenSpec).getDocument();
        } catch (SystemException e) {
            log.error("Caught SystemException in TokenAuthority.issueToken. ",e);
            throw e;
        }
        // call SSO Health Statistic to increment generated token count.
        this.idmAccessor.incrementGeneratedTokens(tenant);
        return retval;
    }
    // create saml token spec data (with default settings for now)
    private SamlTokenSpec createTokenSpec(String relyingPartyUrl,
            PrincipalId principalId, String identityFormat,
            AuthnMethod authnMethod, Session session,
            String inResponseTo, String recipient, String audience) {
        log.debug("create token spec for principal " + principalId);
        log.debug("relying party url " + relyingPartyUrl + ", identityFormat "
                + identityFormat);
        log.debug("authn method " + authnMethod + " session " + session);
        log.debug("inResponseTo " + inResponseTo + " recipient " + recipient);
        log.debug("audience " + audience);

        Calendar calendar = new GregorianCalendar();
        Date startTime = this.getStartTime(); // use startTime from request
        if (startTime == null) {
            // if request didn't set start time, use default value
            // default value is hardcoded for the following reason:
            //   if clock tolerance setting for the tenant is really high (e.g. 10 minutes),
            //   and we set notBefore to current time minus half clock tolerance (5 minutes)
            // then we can end up with notOnOrAfter setting in the past
            //   (since token lifetime is restricted to 5 minutes by saml authority in some cases)
            int toleranceSec = Shared.NOTBEFORE_ADJUSTMENT_SECONDS;
            calendar.add(Calendar.SECOND, -(toleranceSec/2));
            startTime = calendar.getTime();
        }
        calendar = new GregorianCalendar();
        Date authnTime = calendar.getTime();
        calendar = new GregorianCalendar();
        calendar.add(Calendar.MINUTE, Shared.TOKEN_LIFETIME_MINUTES);
        Date endTime = calendar.getTime();

        TimePeriod lifespan = new TimePeriod(startTime, endTime);
        Confirmation confirmation =
            new Confirmation(inResponseTo, recipient);
        Collection<String> attributeList = Shared.buildTokenAttributeList(identityFormat);

        // ensure participant session here
        String participantSessionId = null;
        try {
            session.getLock().lock();
            participantSessionId = session
                    .ensureSessionParticipant(relyingPartyUrl);
            this.getSessionManager().update(session);
        } catch (Exception e) {
            participantSessionId = null;
        } finally {
            session.getLock().unlock();
        }

        if (participantSessionId == null) {
            log.debug("WARNING: unable to create an SSO session");
        }

        SamlTokenSpec.Builder builder = new SamlTokenSpec.Builder(lifespan, confirmation,
                new SamlTokenSpec.AuthenticationData(principalId, authnTime,
                        authnMethod, identityFormat, participantSessionId, session.getExpireDate()),
                attributeList);
        builder.addAudience(audience);
        if (this.isRenewable()) {
            // add renew spec
            SamlTokenSpec.RenewSpec renewSpec =
                    new SamlTokenSpec.RenewSpec(true);
            builder.setRenewSpec(renewSpec);
        }
        if (this.isDelegable()) {
            // add delegation spec
            SamlTokenSpec.DelegationSpec delegationSpec =
                    new SamlTokenSpec.DelegationSpec(null, true);
            builder.setDelegationSpec(delegationSpec);
        }
        SamlTokenSpec spec = builder.createSpec();
        return spec;
    }


    // create saml token authority object
    private TokenAuthority createTokenAuthorityForTenant(String tenant)
            throws SamlServiceException {
        log.debug("create token authority for tenant " + tenant);

        return this.getSamlAuthFactory().createTokenAuthority(tenant);
    }

    public void addResponseHeaders(HttpServletResponse response) {
        Shared.addNoCacheHeader(response);
        if (this.wwwAuthenticate != null) {
            // add WWW-Authenticate header
            if (this.kerbAuthnType == KerbAuthnType.CIP) {
                response.addHeader(Shared.RESPONSE_AUTH_HEADER, this.wwwAuthenticate);
            } else {
               response.addHeader(Shared.IWA_AUTH_RESPONSE_HEADER, this.wwwAuthenticate);
            }
        }
        if (this.getSessionId() != null) {
            // set session cookie
            String tenantSessionCookieName = Shared.getTenantSessionCookieName(this.getIdmAccessor().getTenant());
            Shared.addSessionCookie(tenantSessionCookieName, this.getSessionId(), response);
        }
    }

    public void addTenantIDPCookie(String cookieValue, HttpServletResponse response) {
        String tenantIDPCookieName = Shared.getTenantIDPCookieName(this.getIdmAccessor().getTenant());
        Shared.addSessionCookie(tenantIDPCookieName, cookieValue, response);
    }


    // create SamlService object
    private SamlService createSamlServiceForTenant(String tenant,
            String relyingParty) {
        this.idmAccessor.setTenant(tenant);
        CertPath certPath = this.idmAccessor
                .getCertificatesForRelyingParty(relyingParty);

        SignatureAlgorithm checkAlgorithm;
        if (this.sigAlg == null) {
            checkAlgorithm = null;
        } else {
            checkAlgorithm = SignatureAlgorithm.getSignatureAlgorithmForURI(this.sigAlg);
            if (checkAlgorithm == null) {
                this.validationResult = new ValidationResult(HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
                throw new IllegalStateException("authn request has invalid signature algorithm");
            }
        }

        SamlServiceFactory factory = new DefaultSamlServiceFactory();
        return factory.createSamlService(
                null,
                null, /* will not use this service to sign messages */
                checkAlgorithm,
                this.idmAccessor.getIdpEntityId(), certPath);
    }

    public String getWwwAuthenticate() {
        return this.wwwAuthenticate;
    }

    public void setWwwAuthenticate(String wwwAuthenticate) {
        this.wwwAuthenticate = wwwAuthenticate;
    }

    public KerbAuthnType getKerbAuthnType() {
        return this.kerbAuthnType;
    }

    public void setKerbAuthnType(KerbAuthnType kerbAuthnType) {
        this.kerbAuthnType = kerbAuthnType;
    }
    /**
     * @return the identityFormat
     */
    public String getIdentityFormat() {
        return this.identityFormat;
    }

    /**
     * @param identityFormat
     *            the identityFormat to set
     */
    public void setIdentityFormat(String identityFormat) {
        this.identityFormat = identityFormat;
    }

    /**
     * @return the issuerValue
     */
    private String getIssuerValue() {
        return this.issuerValue;
    }

    /**
     * @param issuerValue
     *            the issuerValue to set
     */
    private void setIssuerValue(String issuerValue) {
        this.issuerValue = issuerValue;
    }

    /**
     * @return the sessionId
     */
    public String getSessionId() {
        return this.sessionId;
    }

    /**
     * @param sessionId
     *            the sessionId to set
     */
    public void setSessionId(String sessionId) {
        this.sessionId = sessionId;
    }

    /**
     * @return the authnMethod
     */
    public AuthnMethod getAuthnMethod() {
        return this.authnMethod;
    }

    /**
     * @param authnMethod
     *            the authnMethod to set
     */
    public void setAuthnMethod(AuthnMethod authnMethod) {
        this.authnMethod = authnMethod;
    }

    /**
     * @return the principalId
     */
    public PrincipalId getPrincipalId() {
        return this.principalId;
    }

    /**
     * @param principalId
     *            the principalId to set
     */
    public void setPrincipalId(PrincipalId principalId) {
        this.principalId = principalId;
    }

    /**
     * @return the startTime
     */
    public Date getStartTime() {
        return this.startTime;
    }

    /**
     * @param startTime the startTime to set
     */
    public void setStartTime(Date startTime) {
        this.startTime = startTime;
    }

    /**
     * @return the isRenewable
     */
    public boolean isRenewable() {
        return this.isRenewable;
    }

    /**
     * @param isRenewable the isRenewable to set
     */
    public void setRenewable(boolean isRenewable) {
        this.isRenewable = isRenewable;
    }

    /**
     * @return the isDelegable
     */
    public boolean isDelegable() {
        return this.isDelegable;
    }

    /**
     * @param isDelegable the isDelegable to set
     */
    public void setDelegable(boolean isDelegable) {
        this.isDelegable = isDelegable;
    }

    /**
     * @return the isExistRequest
     */
    public boolean isExistingRequest() {
    	return this.isExistingRequest;
    }

    /**
     * @param isExistRequest the isExistRequest to set
     */
    public void setIsExistingRequest(boolean isExistingRequest) {
    	this.isExistingRequest = isExistingRequest;
    }

    /**
     * @return  proxyCount null, 0 or possitive int value
     */
    public Integer getProxyCount() {
        return proxyCount;
    }

    /**
     * @param  proxyCount null, 0 or possitive int value
     */
   public void setProxyCount(Integer proxyCount) {
        this.proxyCount = proxyCount;
    }

    public boolean isProxying() {
        return isProxying;
    }

    public void setProxying(boolean isProxying) {
        this.isProxying = isProxying;
    }

    public boolean isIDPSelectionEnabled(String tenantName) {
        if (this.isIDPSelectionEnabled == null) {
            this.isIDPSelectionEnabled = this.idmAccessor.getTenantIDPSelectionFlag(tenantName);
        }

        return this.isIDPSelectionEnabled;
    }

    public List<IDPEntry> getIdpList() {
        return idpList;
    }

    /**
     * @param  idpList null, or a list of allowed IDPs
     */
    public void setIdpList(List<IDPEntry> idpList) {
        this.idpList = idpList;
    }

    /*
     * @param ProviderID/EntityID
     */
    public IDPConfig getExtIDPToUse() {
        return extIDPToUse;
    }

    public void setExtIDPToUse(IDPConfig idpConfig) {
        this.extIDPToUse = idpConfig;
    }

    public Boolean isLoginViewRequired() {
        return needLoginView;
    }

    public void setNeedLoginView(boolean needLoginView) {
        this.needLoginView = new Boolean(needLoginView);
    }

    public Boolean isChooseIDPViewRequired() {
        return needChooseIDPView;
    }

    public void setNeedChooseIDPView(boolean needChooseIDPView) {
        this.needChooseIDPView = new Boolean(needChooseIDPView);
    }

    /**
     * @return the response
     */
    public HttpServletResponse getResponse() {
        return response;
    }

    /**
     * Processing state for AuthnRequest
     */
    public static enum ProcessingState {
        /**
         * Unknown / not initialized
         */
        UNKNOWN,
        /**
         * initialized, not parsed
         */
        INITIALIZED,
        /**
         * parsed
         */
        PARSED
    }

    public void setLocal(Locale locale) {
        this.locale = locale;

    }

    /**
     * @return the messageSource
     */
    public MessageSource getMessageSource() {
        return messageSource;
    }

    /**
     * @param messageSource the messageSource to set
     */
    public void setMessageSource(MessageSource messageSource) {
        this.messageSource = messageSource;
    }

    /**
     * @return the locale
     */
    public Locale getLocale() {
        return locale;
    }

    /**
     * @param locale the locale to set
     */
    public void setLocale(Locale locale) {
        this.locale = locale;
    }

    /**
     * @return the samlAuthFactory
     */
    public DefaultSamlAuthorityFactory getSamlAuthFactory() {
        return samlAuthFactory;
    }

    /**
     * @param samlAuthFactory the samlAuthFactory to set
     */
    public void setSamlAuthFactory(DefaultSamlAuthorityFactory samlAuthFactory) {
        this.samlAuthFactory = samlAuthFactory;
    }

	public AuthnTypesSupported getAuthTypesSupportecd() {
		return authnTypesSupported;
	}

	public void setAuthnTypesSupported(AuthnTypesSupported authTypesSupportecd) {
		this.authnTypesSupported = authTypesSupportecd;
	}

    /**
     * @return the acsUrl
     */
    public String getAcsUrl() {
        return acsUrl;
    }

    /**
     * @param acsUrl the acsUrl to set
     */
    public void setAcsUrl(String acsUrl) {
        this.acsUrl = acsUrl;
    }

    /**
     * @param externalIDPsessionIndex   optional, need with external authentication only.
     * @param externalIdpId     optional, need with external authentication only.
     * @throws SamlServiceException
     */
    public void createSession(String externalIDPsessionIndex, String externalIdpId) throws SamlServiceException {

        Session currentSession = this.sessionManager.createSession(
                this.principalId,this.authnMethod, externalIDPsessionIndex,externalIdpId);
        this.setSessionId(currentSession.getId());
    }

}
