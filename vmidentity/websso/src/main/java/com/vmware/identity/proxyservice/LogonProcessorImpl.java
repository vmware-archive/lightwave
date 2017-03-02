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

package com.vmware.identity.proxyservice;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;

import javax.servlet.RequestDispatcher;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.Validate;
import org.opensaml.common.SignableSAMLObject;
import org.opensaml.saml2.core.LogoutResponse;
import org.opensaml.saml2.core.Response;
import org.opensaml.ws.message.decoder.MessageDecodingException;
import org.opensaml.xml.ConfigurationException;
import org.opensaml.xml.io.UnmarshallingException;
import org.opensaml.xml.security.SecurityException;
import org.opensaml.xml.util.Base64;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RelyingParty;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.InvalidSignatureException;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlAuthorityFactory;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.SystemException;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.ServerValidatableSamlTokenFactory;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.DefaultIdmAccessorFactory;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SAMLResponseSender;
import com.vmware.identity.samlservice.SAMLResponseSenderFactory;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.ConfigExtractorFactoryImpl;
import com.vmware.identity.samlservice.impl.SAMLAuthnResponseSenderFactory;
import com.vmware.identity.samlservice.impl.SamlServiceImpl;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.websso.client.AuthnData;
import com.vmware.identity.websso.client.LogonProcessorEx;
import com.vmware.identity.websso.client.Message;
import com.vmware.identity.websso.client.SamlUtils;
import com.vmware.identity.websso.client.SharedUtils;

/**
 * Proxying service Logon Processor Implementation
 *
 */
public class LogonProcessorImpl implements LogonProcessorEx {

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private SessionManager sessionManager;

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(LogonProcessorImpl.class);

    private static final String RPSelectionParam_RPNameList = "rp_display_name_list";
    private static final String RPSelectionParam_RPEntityIDList = "rp_entity_id_list";
    private static final String RPSelectionJSP = "/WEB-INF/views/chooseRP.jsp";
    private static final String RPSelectionParam_SAMLResponse = "trusted_saml_response";
    private static final String RPSelectionParam_DialogTittle = "tittle";
    private static final String ExternalIDPErrorMessageName = "ExternalIDPErrorMessage";

    public static final int THRESHHOLD_HOURS_FOR_MAP_CHECK = 1;

    public static final int THRESHHOLD_SIZE_FOR_MAP_CHECK = 5000;

    private final ServerValidatableSamlTokenFactory tokenFactory = new ServerValidatableSamlTokenFactory();

    private final SamlAuthorityFactory samlAuthFactory = new DefaultSamlAuthorityFactory(SignatureAlgorithm.RSA_SHA256, new IdmPrincipalAttributesExtractorFactory(Shared.IDM_HOSTNAME),
            new ConfigExtractorFactoryImpl());

    // When using nested synchronization, please doing so with same order to
    // avoid deadlock. The order is sync outgoingReqToIncomingReqMap first, followed by authnReqStateMap.
    private final Map<String, AuthnRequestState> authnReqStateMap = Collections.synchronizedMap(new LinkedHashMap<String, AuthnRequestState>());
    private final Map<String, String> outgoingReqToIncomingReqMap = Collections.synchronizedMap(new LinkedHashMap<String, String>());

    /*
     * WebSSO client library authn error handler. At failure of authentication
     * by IDP, SSO server should notify the initiating service provider or
     * client browser.
     *
     * @see
     * com.vmware.identity.websso.client.LogonProcessor#authenticationError(
     * com.vmware.identity.websso.client.Message,
     * javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void authenticationError(Message arg0, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse response) {

        Validate.notNull(arg0, "arg0");
        Validate.notNull(response, "response");
        try {

            ValidationResult vrExtIdpSsoResponse = retrieveValidationResult(arg0);

            // retrieve current VR
            AuthnRequestState requestState = findOriginalRequstState(request);
            if (!arg0.isIdpInitiated() && requestState != null) {
                ValidationResult vr = requestState.getValidationResult();
                if (!(null != vr && !vr.isValid())) {
                    requestState.setValidationResult(vrExtIdpSsoResponse);
                }
            } else {
                Validate.notEmpty(tenant, "tenant");
                Validate.notNull(locale, "locale");

            }

            // retrieve browser locale
            if (null == locale) {
                locale = requestState == null ? Locale.getDefault() : requestState.getLocale();
            }

            int responseCode = vrExtIdpSsoResponse.getResponseCode();

            // default code is HttpServletResponse.SC_UNAUTHORIZED
            if (responseCode == HttpServletResponse.SC_OK) {
                responseCode = HttpServletResponse.SC_UNAUTHORIZED;
            }

            String message = vrExtIdpSsoResponse.getMessage(messageSource, locale);

            if (OasisNames.RESPONDER.equals(vrExtIdpSsoResponse.getStatus())) {
                message = messageSource.getMessage(ExternalIDPErrorMessageName, null, locale) + message;
            }
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(message));
            response.sendError(responseCode, message);
            log.info("External IDP responded with ERROR {} message {}", vrExtIdpSsoResponse.getResponseCode(), message);

        } catch (Exception e) {
            log.error("Caught unexpect exception in processing authentication error {}", e.toString());

            throw new IllegalStateException(e);
        }
    }

    @Override
    public void authenticationError(Message message, HttpServletRequest request, HttpServletResponse response) {
        authenticationError(message, null, null, request, response);
    }

    @Override
    public void authenticationSuccess(Message message, HttpServletRequest request, HttpServletResponse response) {
        authenticationSuccess(message, null, null, request, response);
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.LogonProcessor#authenticationSuccess
     * (com.vmware.identity.websso.client.Message,
     * javax.servlet.http.HttpServletResponse)
     *
     * We got a validated external token from client lib. This function does the
     * following: 1. validate the subject in lotus, 2. create session. 3. a)
     * issue response to requesting SP (in SP-initiated request) or b) redirect
     * to resource URI (in IDP-Initiated request)
     */
    @Override
    public void authenticationSuccess(Message arg0, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse response) {

        AuthnRequestState requestState = null;
        try {
            Validate.notNull(arg0, "arg0");
            Validate.notNull(response, "response obj for service provider");
            Validate.notNull(request, "request obj for service provider");
            Validate.notEmpty(tenant, "tenant");

            SAMLResponseSenderFactory responseSenderFactory = new SAMLAuthnResponseSenderFactory();

            IdmAccessor idmAccessor = new DefaultIdmAccessorFactory().getIdmAccessor();
            idmAccessor.setTenant(tenant);

            if (arg0.isIdpInitiated()) {
                log.debug("IDP-Initiated: Begin processing authentication response.");
                Validate.notNull(locale, "locale");

                PrincipalId principal = validateExternalUser(idmAccessor, arg0);
                if (principal == null) {
                    authenticationError(arg0, locale, tenant, request, response);
                    return;
                }

                Session currentSession = Shared.getSession(sessionManager, request, tenant);
                if (currentSession == null) {
                    currentSession = this.sessionManager.createSession(principal, AuthnMethod.ASSERTION, arg0.getSessionIndex(), arg0.getSource());

                    String tenantSessionCookieName = Shared.getTenantSessionCookieName(tenant);
                    log.trace("Setting cookie " + tenantSessionCookieName + " value " + currentSession.getId());

                    Shared.addSessionCookie(tenantSessionCookieName, currentSession.getId(), response);
                }

                String rpID = chooseSignInRPSite(idmAccessor, request, response, locale);

                if (null == rpID) {
                    // skip for this request. There will be a new request with
                    // RP selected.
                    log.debug("No Relying party was selected yet. Skip sending the SAML response!");
                } else {
                    SAMLResponseSender responseSender = responseSenderFactory.buildResponseSender(tenant, response, locale, null, // for IDP initiated, no relaystate in response to SP
                            null, // no request
                            currentSession.getAuthnMethod(), currentSession.getId(), currentSession.getPrincipalId(), messageSource, sessionManager);

                    Document token = responseSender.generateTokenForResponse(rpID);

                    responseSender.sendResponseToRP(rpID, token);
                }

                log.info("IDP-Initiated: End processing authentication response. Session was created.");
            } else {

                log.info("SP-Initiated: Begin processing authentication response.");
                requestState = findOriginalRequstState(request);

                if (null == requestState) {
                    throw new SamlServiceException("No source authentication request was matched to the outgoing request");
                }

                locale = requestState.getLocale();
                Validate.notNull(locale, "locale");

                tenant = requestState.getIdmAccessor().getTenant();

                PrincipalId principal = validateExternalUser(idmAccessor, arg0);
                if (principal == null) {
                    authenticationError(arg0, locale, tenant, request, response);
                    return;
                }

                // send response to initiating relying party
                requestState.addResponseHeaders(response);
                requestState.setPrincipalId(principal);
                requestState.setAuthnMethod(AuthnMethod.ASSERTION);
                requestState.createSession(arg0.getSessionIndex(), arg0.getSource());

                String tenantSessionCookieName = Shared.getTenantSessionCookieName(tenant);
                log.trace("Setting cookie " + tenantSessionCookieName + " value " + requestState.getSessionId());

                Shared.addSessionCookie(tenantSessionCookieName, requestState.getSessionId(), response);

                SAMLResponseSender responseSender = responseSenderFactory.buildResponseSender(tenant, response, locale, null,
                        requestState, AuthnMethod.ASSERTION, requestState.getSessionId(), principal, messageSource, sessionManager);

                String rpID = requestState.getAuthnRequest().getIssuer().getValue();

                Validate.notEmpty(rpID, "No Relying party ID in SAML authentication request!");
                Document token = responseSender.generateTokenForResponse(rpID);
                responseSender.sendResponseToRP(rpID, token);

                log.info("SP-Initiated: End processing  Authentication response. Session was created.");
            }
        } catch (InvalidTokenException f) {
            log.error("Caught InvalidTokenException in validating external token {}", f.toString());
            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_NOT_ACCEPTABLE, OasisNames.RESPONDER,
                    "Unable to validate token issued by external IDP");
            arg0.setValidationResult(extResponseVr);
            authenticationError(arg0, locale, tenant, request, response);

        } catch (SamlServiceException e) {
            log.error("Caught Saml Service Exception in creating WebSSO session or issuring token. ", e.toString());
            if (!arg0.isIdpInitiated() && requestState != null) {
                requestState.setValidationResult(new ValidationResult(OasisNames.RESPONDER, e.getMessage()));
            }

            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_NOT_ACCEPTABLE, OasisNames.RESPONDER,
                    "External logon response was validated successfully. However local PSC was unable to send response to Relying party.");
            arg0.setValidationResult(extResponseVr);
            authenticationError(arg0, locale, tenant, request, response); // server
        } catch (IOException g) {
            log.error("Caught IOException in sending out response or redirecting to relying party {}", g.toString());
            internalError(g, locale, tenant, request, response);
        }
    }

    /*
     * Error condition that we are not able notify SP. This error tipically
     * encountered during sending response to SP. So all we can do is send
     * browser error message.
     *
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.websso.client.LogonProcessor#internalError(java.lang
     * .Exception, javax.servlet.http.HttpServletRequest)
     */
    @Override
    public void internalError(Exception error, Locale locale, String tenant, HttpServletRequest request, HttpServletResponse response) {
        log.error("Caught internalError in authenticating with external IDP! {}", error.toString());
        Validate.notNull(response, "response");
        try {

            int httpErrorCode = HttpServletResponse.SC_INTERNAL_SERVER_ERROR;
            String messageStr = error.getMessage();
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(messageStr));
            response.sendError(httpErrorCode, messageStr);
            log.info("Send client browser error: " + httpErrorCode + ", message: " + messageStr);

        } catch (IOException e) {
            log.error("Caught IOException in sending out response to relying party! {}", e.toString());
        }
    }

    @Override
    public void internalError(Exception internalError, HttpServletRequest request, HttpServletResponse response) {
        internalError(internalError, null, null, request, response);
    }

    /**
     * register AuthnRequestState of the initial request.
     *
     * @param incomingReqID
     *            The request ID of Authentication request came to PSC.
     * @param outGoingReqID
     *            The request ID of Authentication request sending out to
     *            external IDP.
     * @param requestState
     *            AuthnRequestState object for the authentication request came
     *            to PSC.
     */
    public void registerRequestState(String incomingReqID, String outGoingReqID, AuthnRequestState requestState) {
        Validate.notEmpty(incomingReqID, "incomingReqID");
        Validate.notEmpty(outGoingReqID, "outGoingReqID");
        if (requestState == null) {
            log.warn("Skip null requestState registration.");
            return;
        }

        if (requestState.getStartTime() == null) {
            requestState.setStartTime(new Date());
        }
        Date noOlderThanDate = new Date(System.currentTimeMillis() - THRESHHOLD_HOURS_FOR_MAP_CHECK * 60 * 60 * 1000);
        this.cleanStaledLogon(noOlderThanDate);
        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.authnReqStateMap) {
                this.authnReqStateMap.put(incomingReqID, requestState);
                this.outgoingReqToIncomingReqMap.put(outGoingReqID, incomingReqID);
            }
        }
    }

    private void cleanStaledLogon(Date noOlderThanDate) {
        if (this.authnReqStateMap.size() < THRESHHOLD_SIZE_FOR_MAP_CHECK) {
            return;
        }

        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.authnReqStateMap) {
                Iterator<Entry<String, String>> it = this.outgoingReqToIncomingReqMap.entrySet().iterator();
                while (it.hasNext()) {
                    Entry<String, String> idMapEntry = it.next();
                    String inReqID = idMapEntry.getValue();
                    AuthnRequestState state = this.authnReqStateMap.get(inReqID);
                    if (state.getStartTime().before(noOlderThanDate)) {
                        this.authnReqStateMap.remove(inReqID);
                        this.outgoingReqToIncomingReqMap.remove(idMapEntry.getKey());
                    }
                    else {
                        break;
                    }
                }
            }
        }
    }

    // =============== private functions =====================
    /**
     * Choose a relying party site to send the assertion. 1. Check RelayState 2.
     * Check if the selection is made by user. 3. If not promp user for
     * selection and return null from this function. The user selection of RP
     * will come in as a new request.
     *
     * @param tenant
     * @param request
     * @param response
     * @param locale
     * @return EntityID string of the selected RP. Null if Choose RP dialog is
     *         called.
     * @throws SamlServiceException
     *             not able to dispatch request to RP selection jsp.
     */
    private String chooseSignInRPSite(IdmAccessor idmAccessor, HttpServletRequest request, HttpServletResponse response, Locale locale) throws SamlServiceException {

        Validate.notNull(idmAccessor, "idmAccessor");
        Validate.notNull(request, "request");
        Validate.notNull(response, "response");

        String tenant = idmAccessor.getTenant();
        Validate.notNull(tenant, "tenant");

        // Use RelayState if available.
        String rp_selected = request.getParameter(Shared.RELAY_STATE_PARAMETER);
        if (rp_selected != null && !rp_selected.isEmpty()) {
            String rpID;
            try {
                rpID = URLDecoder.decode(rp_selected, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                throw new SamlServiceException("Failed in URL-decoding RelayState: " + rp_selected, e);
            }
            log.debug("IDP_Initiated response has relay state which will be used for" + " identifying Relying Party. RelyingPartyID: " + rpID);
            return rpID;
        }

        // Check "CastleRPSelection" header
        rp_selected = request.getHeader(Shared.RP_SELECTION_ENTITYID);

        if (rp_selected != null && !rp_selected.isEmpty()) {
            log.debug("IDP_Initiated response has \"CastleRPSelection\" header which will be used for identifying Relying Party.");
            return rp_selected;
        }

        Collection<RelyingParty> rpCollection = idmAccessor.getRelyingParties(tenant);

        // Prepare lists of RP URL and name for the selection dialog
        Iterator<RelyingParty> it = rpCollection.iterator();

        // Skip the Relying party selection if only one RP is registered
        if (rpCollection.size() == 1) {
            return it.next().getUrl();
        }

        List<String> rpEntityIdList = new ArrayList<String>();
        List<String> rpNameList = new ArrayList<String>();
        while (it.hasNext()) {
            RelyingParty rp = it.next();
            rpEntityIdList.add(rp.getUrl());
            rpNameList.add(rp.getName());
        }

        // dispatch selection form
        request.setAttribute(RPSelectionParam_RPNameList, rpNameList);
        request.setAttribute(RPSelectionParam_RPEntityIDList, rpEntityIdList);

        String samlResponseStr = request.getParameter(Shared.SAML_RESPONSE_PARAMETER);
        request.setAttribute(RPSelectionParam_SAMLResponse, StringEscapeUtils.escapeJavaScript(samlResponseStr));
        request.setAttribute(RPSelectionParam_DialogTittle, messageSource.getMessage("ChooseService.Title", null, locale));
        RequestDispatcher dispatcher = request.getRequestDispatcher(RPSelectionJSP);
        try {
            dispatcher.forward(request, response);
        } catch (Exception e) {
            throw new SamlServiceException("Failed in dispatch request to RPSelection.jsp", e);
        }

        return null;
    }

    /**
     * Validate principal being asserted by IDP. Provision and update JIT user
     * as needed. 1. Validate external token from the response. 2. Provision JIT
     * user if the feature is on and the subject not found. 3. Update JIT user
     * attributes if it is existing JIT user.
     *
     * @param tenant
     * @param arg0
     * @return PrincipalId or null if not able to link account to the incoming
     *         token
     * @throws InvalidSignatureException
     * @throws InvalidTokenException
     * @throws SystemException
     */
    private PrincipalId validateExternalUser(IdmAccessor idmAccessor, Message arg0) throws InvalidSignatureException, InvalidTokenException, SystemException {

        Validate.notNull(idmAccessor, "idmAccessor");
        Validate.notNull(arg0, "Message");
        String tenant = idmAccessor.getTenant();
        Validate.notNull(tenant, "tenant");

        AuthnData authnData = (AuthnData) arg0.getMessageData();
        Validate.notNull(authnData, "authnData");

        Element tokenEle = (Element) authnData.getToken().getElementsByTagNameNS(SAMLNames.NS_VAL_SAML_SAML, SAMLNames.ASSERTION).item(0);
        ServerValidatableSamlToken servToken = tokenFactory.parseToken(tokenEle);

        final TokenValidator authnOnlyTokenValidator = samlAuthFactory.createAuthnOnlyTokenValidator(tenant);
        final ServerValidatableSamlToken validatedToken = authnOnlyTokenValidator.validate(servToken);

        Validate.isTrue(validatedToken.isExternal());

        IDPConfig extIdp = getIssuerIDPConfig(arg0, idmAccessor);
        boolean isJitEnabled = extIdp.getJitAttribute();
        PrincipalId subjectUpn = validatedToken.getSubject().subjectUpn();
        boolean isSubjectValidated = subjectUpn != null && validatedToken.getSubject().subjectValidation() != SubjectValidation.None;

        if (!isJitEnabled && !isSubjectValidated) {
            log.error("Not able to validate the subject in external token ");
            // Failed in creating our own token. Report authentication error
            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_NOT_ACCEPTABLE, OasisNames.REQUEST_DENIED,
                    "Subject validation of external token." + " Unable to validate the subject.");
            arg0.setValidationResult(extResponseVr);
            return null;
        }

        if (isJitEnabled && !isSubjectValidated) {
            try {
                subjectUpn = idmAccessor.createUserAccountJustInTime(servToken.getSubject(), tenant, extIdp, authnData.getAttributes());
            } catch (Exception e) {
                log.error("Failure to create a temporary user.", e);
                com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_UNAUTHORIZED, OasisNames.RESPONDER,
                        null);
                arg0.setValidationResult(extResponseVr);
                return null;
            }
        }

        if (isJitEnabled) {
            // update or add group information based on attribute info in new
            // token
            try {
                idmAccessor.updateJitUserGroups(subjectUpn, tenant, extIdp.getTokenClaimGroupMappings(), authnData.getAttributes());
            } catch (Exception e) {
                log.error("Encountered an error while updating Jit user groups", e);
                com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                        OasisNames.RESPONDER, null);
                arg0.setValidationResult(extResponseVr);
                return null;
            }
        }

        return subjectUpn;
    }

    /**
     *
     * @param arg0
     *            Message object from successful validation of SAML response
     *            message
     * @return IDPConfig for the issuer of the message, null if not found.
     */
    private IDPConfig getIssuerIDPConfig(Message arg0, IdmAccessor accessor) {

        Validate.notNull(arg0, "arg0");
        Validate.notNull(accessor, "accessor.");

        IDPConfig idpConfig = null;
        Collection<IDPConfig> extIdps = accessor.getExternalIdps();

        Validate.notEmpty(extIdps, "No IDP registration found.");

        for (IDPConfig entry : extIdps) {
            if (entry.getEntityID().equals(arg0.getSource())) {
                idpConfig = entry;
            }
        }

        return idpConfig;
    }

    /**
     * Propagate authentication status to websso client requestState. check IDP
     * side returned status first. If succeeded, check our validation status of
     * the external IDP response.
     *
     * @param arg0
     *            Message object passed in to the error callback.
     */

    private ValidationResult retrieveValidationResult(Message arg0) {
        Validate.notNull(arg0, "Message object");

        com.vmware.identity.websso.client.ValidationResult extResponseVr = arg0.getValidationResult();
        Validate.notNull(extResponseVr);
        ValidationResult vr = null;

        if (arg0.getStatus() != null && !arg0.getStatus().equals(OasisNames.SUCCESS)) {
            // If it is external IDP authentication error, popogated the error
            // back to RP.
            vr = new ValidationResult(extResponseVr.getResponseCode(), arg0.getStatus(), arg0.getSubstatus());

        } else {
            // Else send the validation error to RP
            vr = new ValidationResult(extResponseVr.getResponseCode(), extResponseVr.getStatus(), extResponseVr.getSubstatus());
        }
        return vr;
    }

    /**
     * Helper function to decode HttpServletRequest to Response
     *
     * @param parameter
     * @return Response null possible.
     * @throws SecurityException
     * @throws NoSuchAlgorithmException
     * @throws MessageDecodingException
     * @throws ConfigurationException
     * @throws UnmarshallingException
     * @throws IOException
     * @throws SAXException
     * @throws ParserConfigurationException
     */
    private Response decodeResponse(HttpServletRequest request) throws UnmarshallingException, ConfigurationException, ParserConfigurationException, SAXException, IOException {
        Validate.notNull(request, "request");

        String samlResponseStr = request.getParameter(SamlUtils.SAML_RESPONSE_PARAMETER);

        Validate.notEmpty(samlResponseStr, "Empty SSO response string");
        String decodedResponseStr = Shared.decodeString(samlResponseStr);
        Document responseDoc = SharedUtils.createDOM(decodedResponseStr);

        Response samlResponse = (Response) SamlUtils.unmarshallSAMLObj(responseDoc);

        return samlResponse;
    }

    /**
     * This function find a match of SAML Authn Response received to an
     * AuthnRequestState for the original authentication request from relying
     * party. It also removes the AuthnRequestState and associated map entries.
     *
     * @param response
     *            HttpServletRequest that contain SAMLResponse
     * @return AuthnRequestState return null if unable to decode the response,
     *         or there is no inResponseTo attribute, or there was no match
     */
    private AuthnRequestState findOriginalRequstState(HttpServletRequest request) {

        Response samlResponse;
        AuthnRequestState authnRequestState;

        try {
            samlResponse = decodeResponse(request);
        } catch (Exception e) {
            return null;
        }

        if (null == samlResponse) {
            return null;
        }

        String outReqID = samlResponse.getInResponseTo();

        if (null == outReqID) {
            // could be idp initiated request.
            return null;
        }

        synchronized (this.outgoingReqToIncomingReqMap) {
            synchronized (this.authnReqStateMap) {
                String incomingReqID = this.outgoingReqToIncomingReqMap.remove(outReqID);

                if (null == incomingReqID) {
                    log.debug("No source authentication request was matched to the outgoing request id " + outReqID);
                    return null;
                }
                log.info("Removing request state for request id " + incomingReqID);

                authnRequestState = this.authnReqStateMap.remove(incomingReqID);
            }
        }
        return authnRequestState;

    }

}
