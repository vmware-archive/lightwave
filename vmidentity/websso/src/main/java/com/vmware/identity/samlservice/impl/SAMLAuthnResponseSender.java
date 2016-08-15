/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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

package com.vmware.identity.samlservice.impl;

import java.io.IOException;
import java.security.cert.CertPath;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Locale;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.saml2.core.Response;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.SamlTokenSpec;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.TokenAuthority;
import com.vmware.identity.saml.SamlTokenSpec.Confirmation;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.DefaultSamlServiceFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SAMLResponseSender;
import com.vmware.identity.samlservice.SamlService;
import com.vmware.identity.samlservice.SamlServiceFactory;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.AuthnRequestStateKerbAuthenticationFilter.KerbAuthnType;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.util.TimePeriod;

/**
 * @author schai
 *
 */
public class SAMLAuthnResponseSender implements SAMLResponseSender {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(SAMLAuthnResponseSender.class);

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private SessionManager sessionManager;
    private final IdmAccessor idmAccessor;

    private ValidationResult validationResult;
    private HttpServletResponse response;
    private Locale locale;
    private String inResponseTo;  //optional, used in sp-initiated sso request.  the request id.
    private String relayState;

    private Date reguestStartTime;   //optional, used in sp-initiated sso request.
    private boolean isRenewable;
    private boolean isDelegable;
    private String sessionId;
    private String identityFormat;
    private PrincipalId principalId;
    private AuthnMethod authnMethod;
    private KerbAuthnType kerbAuthnType; //for local kerb authntication only

    /**
     * @param tenant    none null.
     * @param response  none null.
     * @param locale    none null.
     * @param relayState  optional
     * @param reqState    optional.
     * @param authMethod  required if reqState == null
     * @param sessionId    required if reqState == null
     * @param pId          required if reqState == null
     * @param messageSrc   none null.
     * @param sessionMgr   none null.
     */
    public SAMLAuthnResponseSender(String tenant,
            HttpServletResponse response,
            Locale locale,
            String relayState,
            AuthnRequestState reqState,
            AuthnMethod authMethod,
            String sessionId,
            PrincipalId pId,
            MessageSource messageSrc,
            SessionManager sessionMgr) {

        Validate.notEmpty(tenant, "tenant");
        Validate.notNull(response, "response obj for service provider");
        Validate.notNull(sessionMgr, "SessionManager");
        Validate.notNull(messageSrc, "messageSrc");
        Validate.notNull(locale, "locale");

        this.response = response;
        this.locale = locale;
        this.idmAccessor = new DefaultIdmAccessorFactory().getIdmAccessor();
        idmAccessor.setTenant(tenant);
        this.relayState = relayState;
        this.messageSource = messageSrc;
        this.sessionManager = sessionMgr;

        this.authnMethod = (authMethod == null)? reqState.getAuthnMethod() : authMethod;
        this.sessionId = (sessionId == null)? reqState.getSessionId() : sessionId;
        this.principalId = (pId == null)? reqState.getPrincipalId() : pId;


        if (reqState != null) {
            //SP initiated
            this.identityFormat = reqState.getIdentityFormat();
            this.reguestStartTime = reqState.getStartTime();
            AuthnRequest authnReq = reqState.getAuthnRequest();
            this.inResponseTo = authnReq == null? null: authnReq.getID();
            this.isRenewable = reqState.isRenewable();
            this.isDelegable = reqState.isDelegable();
            this.validationResult = reqState.getValidationResult();
            this.kerbAuthnType = reqState.getKerbAuthnType();
        } else {
            //External IDP_Initiated
            this.identityFormat = OasisNames.IDENTITY_FORMAT_UPN;
            this.reguestStartTime = new GregorianCalendar().getTime();;
            this.inResponseTo = null;
            this.validationResult = new ValidationResult();
            this.isRenewable = false;
            this.isDelegable = true;
        }
    }

    /**
     * Send authentication response form for the given relying party.
     * @param spEntId
     * @throws IOException
     */
    /* (non-Javadoc)
     * @see com.vmware.identity.samlservice.SAMLResponseSender#sendResponseToSP(java.lang.String, org.w3c.dom.Document)
     */
    @Override
    public void sendResponseToRP( String spEntId, Document token) throws IOException {

        Response samlResponse = generateResponse(
                spEntId, token);
        if (samlResponse == null) {
            // use validation result code to return redirect or error to
            // client
            Validate.notNull(validationResult, "Null validation result.");
            if (validationResult.isRedirect()) {
                response.sendRedirect(this.validationResult.getStatus());
                log.info("Responded with REDIRECT {} target {}",validationResult.getResponseCode(),
                        validationResult.getStatus());
            } else if (this.kerbAuthnType == KerbAuthnType.IWA) {
                response.sendError(this.validationResult.getResponseCode(),
                        this.validationResult.getStatus());
                log.info("Requested kerb negotiation with browser. Response code: {}, Status: {}",
                        validationResult.getResponseCode(), validationResult.getStatus());
            } else {
                String message = this.validationResult.getMessage(messageSource, locale);
                response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(message));
                response.sendError(this.validationResult.getResponseCode(), message);
                log.info("Responded with ERROR {} message {}",this.validationResult.getResponseCode(), message);
            }
        } else {
            String samlResponseForm =
            generateResponseForm(samlResponse, spEntId);
            log.trace("SAML Response Form is {}", samlResponseForm);
            // write response
            Shared.sendResponse(response, Shared.HTML_CONTENT_TYPE, samlResponseForm);
            log.info("Posting successful authentication response to: "+spEntId);
        }
    }

    /**
     * Create Response object representing the SAML response to be included in POST message.
     * @param spEntId  relying party url.
     * @param tokenDoc - could be null.  Available in a successful and completed authentication.
     * @return
     */
    private Response generateResponse(String spEntId, Document tokenDoc) {

        Response retval = null;

        Validate.notNull(this.validationResult);

        if (this.validationResult.getResponseCode() == HttpServletResponse.SC_OK) {
            try {
                // we should reply with Saml response
                Validate.notNull(this.idmAccessor, "idmAccessor");

                SamlService service = this.createSamlServiceForTenant(
                        spEntId);

                String translatedMessage = this.validationResult.getMessage(messageSource,
                        locale);

                String acsUrl = idmAccessor.getAcsForRelyingParty(spEntId, null, null, SAMLNames.HTTP_POST_BINDING, false);

                retval = service
                        .createSamlResponse(
                                this.inResponseTo,
                                acsUrl,
                                this.validationResult.getStatus(),
                                this.validationResult.getSubstatus(),
                                translatedMessage, tokenDoc);
            } catch (Exception e) {
                log.error("Caught exception while generating response "
                        + ", will respond with error 500",e);
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_INTERNAL_SERVER_ERROR, OasisNames.RESPONDER, null);
                return null;
            }
        }
        return retval;

    }

    /**
     * @param spEntId  required.
     * @return
     */
    private SamlService createSamlServiceForTenant(String spEntId) {

        Validate.notEmpty(spEntId, "spEntId");

        CertPath certPath = this.idmAccessor
                .getCertificatesForRelyingParty(spEntId);

        RelyingParty rp = idmAccessor.getRelyingPartyByUrl(spEntId);

        Validate.notNull(rp, "Fail to fetch RelyingParty configuration: "+spEntId);

        SamlServiceFactory factory = new DefaultSamlServiceFactory();
        SamlService service = factory.createSamlService(
                idmAccessor.getSAMLAuthorityPrivateKey(),
                null, /* will not use this service to sign messages */
                null,
                this.idmAccessor.getIdpEntityId(), certPath);
        return service;
    }

    @Override
    public Document generateTokenForResponse(String spEntId) {
        String tenant = this.idmAccessor.getTenant();
        DefaultSamlAuthorityFactory samlAuthFactory = new DefaultSamlAuthorityFactory(
                SignatureAlgorithm.RSA_SHA256,
                new IdmPrincipalAttributesExtractorFactory(Shared.IDM_HOSTNAME),
                new ConfigExtractorFactoryImpl());

        TokenAuthority authority = samlAuthFactory.createTokenAuthority(tenant);
        SamlTokenSpec tokenSpec = this.createTokenSpec(spEntId,
            this.principalId, this.identityFormat,
            this.authnMethod,
            this.sessionManager.get(this.sessionId),
            idmAccessor.getAcsForRelyingParty(spEntId, null, null, SAMLNames.HTTP_POST_BINDING, false),
            spEntId
                );
        Document tokenDoc = null;
        try {
            tokenDoc = authority.issueToken(tokenSpec).getDocument();
        } catch (SystemException e) {
            log.error("Caught SystemException in TokenAuthority.issueToken. ",e);
            throw e;
        }
        // call SSO Health Statistic to increment generated token count.
        this.idmAccessor.incrementGeneratedTokens(tenant);

        return tokenDoc;
    }

    /**
     * create saml token spec data (with default settings for now)
     *
     * @param relyingPartyUrl
     * @param principalId
     * @param identityFormat
     * @param authnMethod
     * @param session
     * @param recipient
     * @param audience
     * @return      SamlTokenSpec
     */
    private SamlTokenSpec createTokenSpec(String relyingPartyUrl,
            PrincipalId principalId, String identityFormat,
            AuthnMethod authnMethod, Session session,
            String recipient, String audience) {
        Validate.notEmpty(relyingPartyUrl,  "relyingPartyUrl");
        Validate.notNull(principalId,  "principalId");
        Validate.notNull(session,  "session");

        log.debug("create token spec for principal " + principalId);
        log.debug("relying party url " + relyingPartyUrl + ", identityFormat "
                + identityFormat);
        log.debug("authn method " + authnMethod + " session " + session);
        log.debug("inResponseTo " + inResponseTo + " recipient " + recipient);
        log.debug("audience " + audience);

        Calendar calendar = new GregorianCalendar();
        Date startTime = this.reguestStartTime; // use startTime from request
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
            this.sessionManager.update(session);
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
        if (this.isRenewable) {
            // add renew spec
            SamlTokenSpec.RenewSpec renewSpec =
                    new SamlTokenSpec.RenewSpec(true);
            builder.setRenewSpec(renewSpec);
        }
        if (this.isDelegable) {
            // add delegation spec
            SamlTokenSpec.DelegationSpec delegationSpec =
                    new SamlTokenSpec.DelegationSpec(null, true);
            builder.setDelegationSpec(delegationSpec);
        }
        SamlTokenSpec spec = builder.createSpec();
        return spec;
    }

    /**
     * Generate authentication response form for the given relying party.
     * @param samlResponse
     * @param spEntId   Entity id of the targeted relying party (service provider).
     * @return
     */
    private String generateResponseForm(Response samlResponse, String spEntId) {
        Validate.notEmpty(spEntId, "spEntId");

        //Create samlService for response signing
        SamlService service = createSamlServiceForTenant(spEntId);

        String acsUrl = idmAccessor.getAcsForRelyingParty(spEntId,
                null,  //ACS index
                null, //ACS URL
                SAMLNames.HTTP_POST_BINDING, false);

        return service.buildPostResponseForm(samlResponse, this.relayState, acsUrl);
    }

}
