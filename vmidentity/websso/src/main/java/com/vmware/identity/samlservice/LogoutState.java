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
import java.security.cert.CertPath;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Locale;
import java.util.UUID;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.Issuer;
import org.opensaml.saml2.core.LogoutRequest;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.xml.security.SecurityException;
import org.opensaml.xml.util.Base64;
import org.springframework.context.MessageSource;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.LogoutStateValidator;
import com.vmware.identity.session.LogoutRequestData;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.session.SessionParticipant;

/**
 * Object that encapsulates a lifetime of LogoutRequest/Response
 *
 */
public class LogoutState {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(LogoutState.class);

    private final DefaultIdmAccessorFactory factory;
    private IdmAccessor idmAccessor;
    private final SamlValidator<LogoutState> validator;
    private RequestCache requestCache;

    private HttpServletRequest request;
	private HttpServletResponse response;
    private MessageSource messageSource;
	private Locale locale;
    private SessionManager sessionManager;
    private SignableSAMLObject samlObject;
    private LogoutRequest logoutRequest;
    private LogoutResponse logoutResponse;
    private String samlRequest;
    private String samlResponse;
    private String signedMessage;
    private String relayState;
    private String sigAlg;
    private String signature;
    private ProcessingState processingState;
    private ValidationResult validationResult;
    private String issuerValue;
    private String sessionId;
    private final String correlationId;


    /**
     * Construct logout state object
     *
     * @param request
     * @param response2
     * @param sessionManager
     * @param locale
     */
    public LogoutState(HttpServletRequest request, HttpServletResponse response
    			, SessionManager sessionManager, Locale locale, MessageSource messageSource) {
        log.debug("Constructing from request " + request.toString());

        Validate.notNull(request);
        Validate.notNull(sessionManager);
        this.processingState = ProcessingState.UNKNOWN;
        this.setRequest(request);
        this.setResponse(response);
        this.setLocale(locale);
        this.setMessageSource(messageSource);
        this.sessionManager = sessionManager;
        //TODO - check for correlation id in the headers PR1561606
        this.correlationId = UUID.randomUUID().toString();
        this.factory = new DefaultIdmAccessorFactory(this.correlationId);
        Validate.notNull(factory);
        this.idmAccessor = factory.getIdmAccessor();
        this.validator = new LogoutStateValidator();
        RequestCacheFactory requestFactory = new DefaultRequestCacheFactory();
        this.requestCache = requestFactory.getRequestCache();

        this.relayState = request.getParameter(Shared.RELAY_STATE_PARAMETER);
        this.signature = request.getParameter(Shared.SIGNATURE_PARAMETER);
        this.sigAlg = request
                .getParameter(Shared.SIGNATURE_ALGORITHM_PARAMETER);
        this.samlRequest = request.getParameter(Shared.SAML_REQUEST_PARAMETER);
        this.samlResponse = request.getParameter(Shared.SAML_RESPONSE_PARAMETER);
        this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_FORBIDDEN, "Forbidden", null);
        Validate.isTrue(this.samlRequest != null
                || this.samlResponse != null);

        // construct message that was supposed to be signed
        if (this.signature != null && this.sigAlg != null) {
            try {
                if (this.samlRequest != null) {
                    this.signedMessage = Shared.SAML_REQUEST_PARAMETER + "="
                            + URLEncoder.encode(this.samlRequest, "UTF-8");
                } else if (this.samlResponse != null) {
                    this.signedMessage = Shared.SAML_RESPONSE_PARAMETER + "="
                            + URLEncoder.encode(this.samlResponse, "UTF-8");
                }
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
                log.debug("Could not reconstruct signed message");
                this.signedMessage = null;
            }
        }

        this.processingState = ProcessingState.INITIALIZED;

    }

    /**
     * Initial parsing of the request Includes signature check and validation
     *
     * @param tenant
     * @param processor
     */
    public void parseRequestForTenant(String tenant,
            ProcessingFilter<LogoutState> processor) {
        log.debug("parseRequestForTenant, tenant " + tenant);

        Validate.notNull(this.idmAccessor);
        Validate.notNull(this.request);

        // check for replays
        if (this.samlRequest != null) {
            if (this.requestCache.shouldDenyRequest(this.samlRequest)) {
                log.debug("Replay attack detected - DENYING logout request");
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_FORBIDDEN, "Forbidden", null);
                throw new IllegalStateException("Forbidden");
            } else {
                this.requestCache.storeRequest(this.samlRequest);
            }
        } else if (this.samlResponse != null) {
            if (this.requestCache.shouldDenyRequest(this.samlResponse)) {
                log.debug("Replay attack detected - DENYING logout response");
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_FORBIDDEN, "Forbidden", null);
                throw new IllegalStateException("Forbidden");
            } else {
                this.requestCache.storeRequest(this.samlResponse);
            }
        }

        try {
            processor.preProcess(this);
        } catch (SamlServiceException e) {
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_FORBIDDEN, "Forbidden", null);
            throw new IllegalStateException(e);
        }

        SamlService service = createSamlServiceForTenant(tenant, null); // relying
        // party
        // unknown
        // at
        // this
        // point
        // decode request
        try {
            setSamlObject(service.decodeSamlRequest(this.request));
            if (samlObject instanceof LogoutRequest) {
                setLogoutRequest((LogoutRequest) samlObject);
            } else if (samlObject instanceof LogoutResponse) {
                setLogoutResponse((LogoutResponse) samlObject);
            }
        } catch (MessageDecodingException e) {
            // fail the validation with specific error code and rethrow
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            log.debug("Caught exception " + e.toString());
            throw new IllegalStateException(e);
        } catch (SecurityException e) {
            // fail the validation with specific error code and rethrow
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            log.debug("Caught exception " + e.toString());
            throw new IllegalStateException(e);
        }
        // if signature was specified along with signing algorithm, verify
        // signature
        Issuer issuer = getIssuer();
        if (issuer == null || issuer.getValue() == null) {
            service = null;
        } else {
            this.setIssuerValue(issuer.getValue());
            service = createSamlServiceForTenant(tenant, this.getIssuerValue());
        }
        if (service == null) {
            // return 400 to the caller and throw
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", "Issuer");
            throw new IllegalStateException("Issuer not recognized");
        }
        if (this.sigAlg != null && this.signature != null) {
            try {
                service.verifySignature(this.signedMessage, this.signature);
            } catch (IllegalStateException e) {
                // fail the validation with specific error code and rethrow
                this.validationResult = new ValidationResult(
                        OasisNames.RESPONDER, OasisNames.REQUEST_DENIED);
                throw new IllegalStateException(e);
            }
        }

        this.validationResult = validator.validate(this);

        if (this.validationResult.isValid()) {
            // mark as parsed
            this.processingState = ProcessingState.PARSED;
        }
    }

    /**
     * Extract issuer value (== relying party URL) from LogoutRequest/Response
     *
     * @return
     */
    private Issuer getIssuer() {
        Issuer retval = null;
        if (getLogoutRequest() != null) {
            retval = getLogoutRequest().getIssuer();
        } else if (getLogoutResponse() != null) {
            retval = getLogoutResponse().getIssuer();
        }
        return retval;
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
                throw new IllegalStateException("logout request has invalid signature algorithm");
            }
        }

        SamlServiceFactory factory = new DefaultSamlServiceFactory();
        return factory.createSamlService(
                this.idmAccessor.getSAMLAuthorityPrivateKey(),
                SignatureAlgorithm.RSA_SHA256, // TODO use real settings
                checkAlgorithm,
                this.idmAccessor.getIdpEntityId(),
                certPath);
    }

    /**
     * Return validation result
     *
     * @return
     */
    public ValidationResult getValidationResult() {
        return validationResult;
    }

    /**
     * Wrapper around ProcessingFilter call
     *
     * @param tenant
     * @param processor
     */
    public void process(String tenant, ProcessingFilter<LogoutState> processor) {
        log.debug("process, tenant " + tenant);

        Validate.notNull(this.idmAccessor);
        Validate.isTrue(this.logoutRequest != null
                || this.logoutResponse != null);
        Validate.notNull(processor);

        // processing call
        try {
            this.idmAccessor.setTenant(tenant);
            processor.process(this);
        } catch (SamlServiceException e) {
            // not enough data
            log.debug("Caught Saml Service Exception from process "
                    + e.toString());
            this.validationResult = new ValidationResult(OasisNames.RESPONDER);
        } catch (Exception e) {
            // unexpected processing error
            log.debug("Caught Exception from process " + e.toString());
            this.validationResult = new ValidationResult(OasisNames.RESPONDER);
        }

        // add log out session cookie for tls client auth
        try {
            addLogoutSessionCookie();
        } catch (Exception e) {
            log.warn("Failed to add logout session cookie for TLS Client auth.", e);
        }
    }

    private void addLogoutSessionCookie() throws UnsupportedEncodingException {
        Session session = sessionManager.get(getSessionId());
        if (session != null && session.getAuthnMethod() == AuthnMethod.TLSCLIENT) {
            // set logout session cookie
            String cookieName = Shared.getLogoutCookieName(this.getIdmAccessor().getTenant());
            java.util.Date date= new java.util.Date();
            String timestamp = new Timestamp(date.getTime()).toString();
            String encodedTimestamp = Shared.encodeString(timestamp);
            log.debug("Setting cookie " + cookieName
                    + " value " + encodedTimestamp);
            Cookie sessionCookie = new Cookie(cookieName, encodedTimestamp);
            sessionCookie.setPath("/");
            sessionCookie.setSecure(true);
            sessionCookie.setHttpOnly(true);
            response.addCookie(sessionCookie);
        }
    }

    /**
     * Remove the session cookie as needed
     * Note: the cookie should be set as exactly as the added in
     * logonState.addResponseHeaders except value.
     * @param response
     */
    public void removeResponseHeaders() {
        log.debug("removeResponseHeaders, response " + response);
        if (getSessionId() != null && response != null) {
            // session identified, remove session cookie
            String cookieName = Shared.getTenantSessionCookieName(this.getIdmAccessor().getTenant());
            removeSessionCookie(cookieName, response);
        }
    }

    private void removeSessionCookie(String cookieName, HttpServletResponse response) {
        Validate.notNull(response);
        if (cookieName == null || cookieName.isEmpty() ) {
            log.warn("Cookie name is null or empty. Ignoring.");
            return;
        }
        log.debug("Removing cookie " + cookieName);
        Cookie sessionCookie = new Cookie(cookieName, "");
        sessionCookie.setPath("/");
        sessionCookie.setSecure(true);
        sessionCookie.setHttpOnly(true);
        sessionCookie.setMaxAge(0);
        response.addCookie(sessionCookie);
    }

    /**
     * Create LogoutResponse based on out ValidationResult
     *
     * @param tenant
     * @param messageSource
     * @param locale
     * @return
     */
    public LogoutResponse generateResponseForTenant(String tenant,
            MessageSource messageSource, Locale locale) {
        LogoutResponse retval = null;

        log.debug("generateResponseForTenant, tenant " + tenant);

        Validate.notNull(this.validationResult);

        // We should reply with Saml response for slo request.
        // For a slo response received (which happens only multi-service logged on scenario),
        // no action is needed further.
        if (this.validationResult.getResponseCode() == HttpServletResponse.SC_OK
                && this.logoutRequest != null) {
            try {
                Validate.notNull(this.idmAccessor);
                Validate.notNull(this.getIssuerValue());

                this.idmAccessor.setTenant(tenant);
                String relyingParty = this.getIssuerValue();
                // however if we had more than one relying party,
                //  we need to get original requester from the session instead
                Session session = this.getSession();
                if (session != null) {
                    LogoutRequestData logoutRequestData = session.getLogoutRequestData();
                    if (logoutRequestData != null) {
                        relyingParty = logoutRequestData.getInitiator();
                    }
                }

                String sloEndpoint = this.idmAccessor.getSloForRelyingParty(relyingParty, OasisNames.HTTP_REDIRECT);
                if (sloEndpoint == null) {
                    log.warn(String.format("SLO service for relying party %s does not exist.", relyingParty));
                    // logout response can be be created since SLO end point does not exist.
                    return null;
                }

                SamlService service = createSamlServiceForTenant(tenant,
                        relyingParty);
                retval = service
                        .createSamlLogoutResponse(
                                this.getID(),
                                sloEndpoint,
                                this.validationResult.getStatus(),
                                this.validationResult.getSubstatus(),
                                this.validationResult.getMessage(messageSource,
                                        locale));

                // At this point, we have sent out SLO requests to non-initiating participants, and is about
                // to send out SLO response to the intiating participant.  So now can kill the session.
                if (getSessionId() != null) {
                    getSessionManager().remove(getSessionId());
                }

                Validate.notNull(this.response, "this.response");
                Shared.addNoCacheHeader(this.response);

            } catch (Exception e) {
                log.debug("Caught exception while generating response "
                        + e.toString() + ", will return 400");
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
                return null;
            }
        }
        return retval;
    }

    /**
     * Extract message ID from logout request/response
     *
     * @return
     */
    private String getID() {
        String retval = null;
        if (getLogoutRequest() != null) {
            retval = getLogoutRequest().getID();
        } else if (getLogoutResponse() != null) {
            retval = getLogoutResponse().getID();
        }
        return retval;
    }

    /**
     * Based on all processing, encode and sign response and generate a redirect
     *
     * @param samlResponse
     * @param tenant
     * @return
     */
    public String generateResponseUrlForTenant(LogoutResponse samlResponse,
            String tenant) {
        String retval = null;

        log.debug("generateResponseUrlForTenant, tenant " + tenant
                + ", SLO response " + samlResponse);

        try {
            // 1. encode response into a string
            Validate.notNull(this.idmAccessor);
            Validate.isTrue(this.logoutRequest != null
                    || this.logoutResponse != null);
            Validate.notNull(this.getIssuerValue());

            this.idmAccessor.setTenant(tenant);
            String relyingParty = this.getIssuerValue();
            SamlService service = createSamlServiceForTenant(tenant,
                    relyingParty);
            String encodedResponse = service.encodeSAMLObject(samlResponse);

            // 2. calculate the message that needs to be signed
            String toBeSigned = service
                    .generateRedirectUrlQueryStringParameters(null,
                            encodedResponse, this.getRelayState(),
                            SignatureAlgorithm.RSA_SHA256.toString(), // TODO
                            // use
                            // real
                            // settings
                            null);

            // 3. calculate signature
            String signature = service.signMessage(toBeSigned);

            // 4. combine everything into a redirect URL
            retval = this.idmAccessor.getSloForRelyingParty(relyingParty,
                    OasisNames.HTTP_REDIRECT);

            if (retval == null) {
                log.warn(String.format("SLO service for relying party %s does not exist.", relyingParty));
                // logout response url can be be created since SLO end point does not exist.
                return null;
            }

            String queryString = service
                    .generateRedirectUrlQueryStringParameters(null,
                            encodedResponse, this.getRelayState(),
                            SignatureAlgorithm.RSA_SHA256.toString(), // TODO
                            // use
                            // real
                            // settings
                            signature);
            retval = retval + "?" + queryString;

        } catch (Exception e) {
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            retval = null;
        }

        return retval;
    }

    /**
     * @return the idmAccessor
     */
    public IdmAccessor getIdmAccessor() {
        return idmAccessor;
    }

    /**
     * @param idmAccessor
     *            the idmAccessor to set
     */
    public void setIdmAccessor(IdmAccessor idmAccessor) {
        this.idmAccessor = idmAccessor;
    }

    /**
     * @return the requestCache
     */
    public RequestCache getRequestCache() {
        return requestCache;
    }

    /**
     * @param requestCache
     *            the requestCache to set
     */
    public void setRequestCache(RequestCache requestCache) {
        this.requestCache = requestCache;
    }

    /**
     * @return the request
     */
    public HttpServletRequest getRequest() {
        return request;
    }

    /**
     * @param request
     *            the request to set
     */
    public void setRequest(HttpServletRequest request) {
        this.request = request;
    }

    /**
     * @return the sessionManager
     */
    public SessionManager getSessionManager() {
        return sessionManager;
    }

    /**
     * @param sessionManager
     *            the sessionManager to set
     */
    public void setSessionManager(SessionManager sessionManager) {
        this.sessionManager = sessionManager;
    }

    /**
     * @return the logoutRequest
     */
    public LogoutRequest getLogoutRequest() {
        return logoutRequest;
    }

    /**
     * @param logoutRequest
     *            the logoutRequest to set
     */
    public void setLogoutRequest(LogoutRequest logoutRequest) {
        this.logoutRequest = logoutRequest;
    }

    /**
     * @return the logoutResponse
     */
    public LogoutResponse getLogoutResponse() {
        return logoutResponse;
    }

    /**
     * @param logoutResponse
     *            the logoutResponse to set
     */
    public void setLogoutResponse(LogoutResponse logoutResponse) {
        this.logoutResponse = logoutResponse;
    }

    /**
     * @return the samlRequest
     */
    public String getSamlRequest() {
        return samlRequest;
    }

    /**
     * @param samlRequest
     *            the samlRequest to set
     */
    public void setSamlRequest(String samlRequest) {
        this.samlRequest = samlRequest;
    }

    /**
     * @return the relayState
     */
    public String getRelayState() {
        return relayState;
    }

    /**
     * @param relayState
     *            the relayState to set
     */
    public void setRelayState(String relayState) {
        this.relayState = relayState;
    }

    /**
     * @return the sigAlg
     */
    public String getSigAlg() {
        return sigAlg;
    }

    /**
     * @param sigAlg
     *            the sigAlg to set
     */
    public void setSigAlg(String sigAlg) {
        this.sigAlg = sigAlg;
    }

    /**
     * @return the signature
     */
    public String getSignature() {
        return signature;
    }

    /**
     * @param signature
     *            the signature to set
     */
    public void setSignature(String signature) {
        this.signature = signature;
    }

    /**
     * @return the processingState
     */
    public ProcessingState getProcessingState() {
        return processingState;
    }

    /**
     * @param processingState
     *            the processingState to set
     */
    public void setProcessingState(ProcessingState processingState) {
        this.processingState = processingState;
    }

    /**
     * @param validationResult
     *            the validationResult to set
     */
    public void setValidationResult(ValidationResult validationResult) {
        this.validationResult = validationResult;
    }

    /**
     * @return the issuerValue
     */
    public String getIssuerValue() {
        return issuerValue;
    }

    /**
     * @param issuerValue
     *            the issuerValue to set
     */
    public void setIssuerValue(String issuerValue) {
        this.issuerValue = issuerValue;
    }

    /**
     * @return the sessionId
     */
    public String getSessionId() {
        return sessionId;
    }

    /**
     * @param sessionId
     *            the sessionId to set
     */
    public void setSessionId(String sessionId) {
        this.sessionId = sessionId;
    }

    /**
     * @return the samlObject
     */
    public SignableSAMLObject getSamlObject() {
        return samlObject;
    }

    /**
     * @param samlObject
     *            the samlObject to set
     */
    public void setSamlObject(SignableSAMLObject samlObject) {
        this.samlObject = samlObject;
    }

    /**
     * Processing state
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

    /**
     * get Session object or null if session cannot be obtained
     * @return
     */
    private Session getSession() {
        Session retval = null;
        if (this.getSessionManager() != null && this.getSessionId() != null) {
            retval = this.getSessionManager().get(
                    this.getSessionId());
        }

        return retval;
    }

    /**
     * Returns true if we need to send out additional LogoutRequest message(s)
     * before ending a session.
     *
     * @return
     */
    public boolean needLogoutRequest() {
        boolean retval = false;
        if (this.getValidationResult().isValid()) {
            // check the session to see that it has more than one participant
            Session session = this.getSession();
            if (session != null) {
                Collection<SessionParticipant> participants = session
                        .getSessionParticipants();
                if (participants != null && participants.size() > 1) {
                    retval = true; // will need to log out other SPs
                }
            }
        }

        return retval;
    }
    /**
     * Returns true if we need to send out LogoutResponse message
     * before ending a session.
     *
     * @return
     */
    public boolean needLogoutResponse() {
        return (this.getSamlRequest() != null);
    }
    /**
     * Generate SAML request redirect url
     * @param samlRequest
     * @param relyingparty
     * @param service
     * @param tenant
     * @return
     */
    public String generateRequestUrlForTenant(LogoutRequest samlRequest, String relyingParty, SamlService service,
            String tenant) {
        String retval = null;

        log.debug("generateRequestUrlForTenant, tenant " + tenant
                + ", request " + samlRequest);

        try {
            // 1. encode response into a string
            Validate.notNull(this.idmAccessor);
            Validate.isTrue(this.logoutRequest != null
                    || this.logoutResponse != null);
            Validate.notNull(this.getIssuerValue());

            this.idmAccessor.setTenant(tenant);
            String encodedRequest = service.encodeSAMLObject(samlRequest);

            // 2. calculate the message that needs to be signed
            String toBeSigned = service
                    .generateRedirectUrlQueryStringParameters(encodedRequest,
                            null, this.getRelayState(),
                            SignatureAlgorithm.RSA_SHA256.toString(), // TODO
                            // use
                            // real
                            // settings
                            null);
            log.debug("Message to sign " + toBeSigned);

            // 3. calculate signature
            String signature = service.signMessage(toBeSigned);
            log.debug("Signature " + signature);

            // 4. combine everything into a redirect URL

            retval = samlRequest.getDestination();
            Validate.notNull(retval);
            String queryString = service
                    .generateRedirectUrlQueryStringParameters(encodedRequest,
                            null, this.getRelayState(),
                            SignatureAlgorithm.RSA_SHA256.toString(), // TODO
                            // use
                            // real
                            // settings
                            signature);
            retval = retval + "?" + queryString;
            log.debug("Generated URL " + retval);

        } catch (Exception e) {
            this.validationResult = new ValidationResult(
                    HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
            retval = null;
        }

        return retval;
    }

    /**
     * Create LogoutRequest redirect URLs for the session participants
     * @param tenant
     * @param messageSource
     * @param locale
     * @return
     */
    public Collection<String> generateRequestUrlsForTenant(String tenant,
            MessageSource messageSource, Locale locale) {
        List<String> requestUrls = new ArrayList<String>();

        log.debug("generateRequestsForTenant, tenant " + tenant);

        Validate.notNull(this.validationResult);

        if (this.validationResult.getResponseCode() == HttpServletResponse.SC_OK) {
            try {
                Validate.notNull(this.idmAccessor);
                Validate.isTrue(this.logoutRequest != null
                        || this.logoutResponse != null);
                Validate.notNull(this.getIssuerValue());
                Session session = this.getSession();
                Validate.notNull(session);

                try {
                    session.getLock().lock();
                    Collection<SessionParticipant> participants = session.getSessionParticipants();
                    LogoutRequestData logoutRequestData = session.getLogoutRequestData();
                    String originalRelyingParty = logoutRequestData != null ?
                            logoutRequestData.getInitiator() : this.getIssuerValue();
                            String targetRelyingParty = null;
                            SessionParticipant targetParticipant = null;

                            // Determine other SP which needs to be logged out. Generate slo request URL for each and add to the list.
                            for (SessionParticipant participant : participants) {
                                if (!originalRelyingParty.equals(participant.getRelyingPartyUrl())) {
                                    //found a participant that is not the original slo initiator.
                                    targetParticipant = participant;
                                    targetRelyingParty = participant.getRelyingPartyUrl();
                                    Validate.notNull(targetParticipant);
                                    Validate.notNull(targetRelyingParty);

                                    // sloEndpoint can be null if relying party does not support SLO.
                                    String sloEndpoint = this.idmAccessor.getSloForRelyingParty(
                                            targetRelyingParty, OasisNames.HTTP_REDIRECT);
                                    if (sloEndpoint != null) {
                                        // Create a request for this SP
                                        this.idmAccessor.setTenant(tenant);
                                        SamlService service = createSamlServiceForTenant(tenant,
                                                targetRelyingParty);
                                        PrincipalId principal = session.getPrincipalId();
                                        Validate.notNull(principal);
                                        LogoutRequest request = service
                                                .createSamlLogoutRequest(
                                                        null,
                                                        sloEndpoint,
                                                        OasisNames.UNSPECIFIED,
                                                        principal.getName() + "@" + principal.getDomain(),
                                                        targetParticipant.getSessionId());

                                        String redirectUrl = generateRequestUrlForTenant(request, targetRelyingParty, service, tenant);
                                        requestUrls.add(redirectUrl);
                                        // Create/update logout request data in the session
                                        if (logoutRequestData == null) {
                                            logoutRequestData = new LogoutRequestData(originalRelyingParty, this.getID());
                                        }
                                        logoutRequestData.setCurrent(targetRelyingParty);
                                        logoutRequestData.setCurrentRequestId(request.getID());
                                        session.setLogoutRequestData(logoutRequestData);
                                        this.getSessionManager().update(session);
                                    }
                                }
                            }
                } finally {
                    session.getLock().unlock();
                }

            } catch (Exception e) {
                log.debug("Caught exception while generating request "
                        + e.toString() + ", will return 400");
                this.validationResult = new ValidationResult(
                        HttpServletResponse.SC_BAD_REQUEST, "BadRequest", null);
                return null;
            }
        }
        return requestUrls;
    }

    /**
     * @return the samlResponse
     */
    public String getSamlResponse() {
        return samlResponse;
    }

    /**
     * @param samlResponse the samlResponse to set
     */
    public void setSamlResponse(String samlResponse) {
        this.samlResponse = samlResponse;
    }

	public HttpServletResponse getResponse() {
		return response;
	}

	public void setResponse(HttpServletResponse response) {
		this.response = response;
	}

	/**
	 * checkIsExternalAuthenticated
	 * @return true if the current session is authenticated using external IDP
	 */
	public boolean checkIsExternalAuthenticated() {
        Validate.notNull(sessionManager);
        Validate.notNull(sessionId);
		Session session = sessionManager.get(sessionId);
		Validate.notNull(session);

		return session.isUsingExtIDP();
	}

	public Locale getLocale() {
		return locale;
	}

	public void setLocale(Locale locale) {
		this.locale = locale;
	}

	public MessageSource getMessageSource() {
		return messageSource;
	}

	public void setMessageSource(MessageSource messageSource) {
		this.messageSource = messageSource;
	}
}
