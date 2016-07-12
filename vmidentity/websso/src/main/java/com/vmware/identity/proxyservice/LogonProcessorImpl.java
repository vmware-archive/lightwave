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

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;

import javax.servlet.RequestDispatcher;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.Validate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

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
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.session.Session;
import com.vmware.identity.session.SessionManager;
import com.vmware.identity.websso.client.AuthnData;
import com.vmware.identity.websso.client.LogonProcessorEx;
import com.vmware.identity.websso.client.Message;

/**
 * Proxying service  Logon Processor Implementation
 *
 */
public class LogonProcessorImpl implements LogonProcessorEx {

    @Autowired
    private MessageSource messageSource;

    @Autowired
    private SessionManager sessionManager;

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(LogonProcessorImpl.class);

    private static final String RPSelectionParam_RPNameList = "rp_display_name_list";
    private static final String RPSelectionParam_RPEntityIDList = "rp_entity_id_list";
    private static final String RPSelectionJSP = "/WEB-INF/views/chooseRP.jsp";
    private static final String RPSelectionParam_SAMLResponse = "trusted_saml_response";
    private static final String RPSelectionParam_DialogTittle = "tittle";
    private static final String ExternalIDPErrorMessageName = "ExternalIDPErrorMessage";

    private final ServerValidatableSamlTokenFactory tokenFactory =
            new ServerValidatableSamlTokenFactory();

    private final SamlAuthorityFactory samlAuthFactory = new DefaultSamlAuthorityFactory(
            SignatureAlgorithm.RSA_SHA256,
            new IdmPrincipalAttributesExtractorFactory(Shared.IDM_HOSTNAME),
            new ConfigExtractorFactoryImpl());

    private AuthnRequestState requestState;

    /*
     * WebSSO client library authn error handler.
     * At failure of authentication by IDP, SSO server should notify the initiating service provider or client browser.
     * @see com.vmware.identity.websso.client.LogonProcessor#authenticationError(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void authenticationError(Message arg0, Locale locale,  String tenant, HttpServletRequest request,
            HttpServletResponse response) {

        Validate.notNull(arg0, "arg0");
        Validate.notNull(response, "response");
        try {

            ValidationResult vrExtIdpSsoResponse = retrieveValidationResult(arg0);

            //retrieve current VR
            if (!arg0.isIdpInitiated() && requestState != null ) {
                ValidationResult vr =  requestState.getValidationResult();
                if ( !( null != vr && !vr.isValid())) {
                    requestState.setValidationResult(vrExtIdpSsoResponse);
                }
            } else {
                Validate.notEmpty(tenant, "tenant");
                Validate.notNull(locale, "locale");

              }

            //retrieve browser locale
            if (null == locale) {
                locale = requestState == null? Locale.getDefault():requestState.getLocale();
            }

            int responseCode = vrExtIdpSsoResponse.getResponseCode();

            //default code is  HttpServletResponse.SC_UNAUTHORIZED
            if (responseCode == HttpServletResponse.SC_OK) {
                responseCode = HttpServletResponse.SC_UNAUTHORIZED;
            }

            String message = vrExtIdpSsoResponse.getMessage(messageSource, locale);

            if (OasisNames.RESPONDER.equals(vrExtIdpSsoResponse.getStatus()) ) {
                message = messageSource.getMessage(ExternalIDPErrorMessageName, null, locale) + message;
            }
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(message));
            response.sendError(responseCode, message);
            log.info("External IDP responded with ERROR {} message {}",vrExtIdpSsoResponse.getResponseCode(), message);

        } catch (Exception e) {
            log.error("Caught unexpect exception in processing authentication error {}", e.toString());

            throw new IllegalStateException(e);
        }
    }

    @Override
    public void authenticationError(Message message,  HttpServletRequest request, HttpServletResponse response) {
        authenticationError(message, null, null, request, response);
    }

    @Override
    public void authenticationSuccess(Message message, HttpServletRequest request, HttpServletResponse response) {
        authenticationSuccess(message, null,null, request, response);
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogonProcessor#authenticationSuccess(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     *
     * We got a validated external token from client lib.  This function does the following:
     * 1. validate the subject in lotus,
     * 2. create session.
     * 3. a) issue response to requesting SP (in SP-initiated request) or
     *    b) redirect to resource URI (in IDP-Initiated request)
     *
     */
    @Override
    public void authenticationSuccess(Message arg0, Locale locale, String tenant, HttpServletRequest request,
            HttpServletResponse response) {

        try {
            Validate.notNull(arg0, "arg0");
            Validate.notNull(response, "response obj for service provider");
            Validate.notNull(request, "request obj for service provider");
            Validate.notEmpty(tenant, "tenant");

            SAMLResponseSenderFactory responseSenderFactory =
                    new SAMLAuthnResponseSenderFactory();

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
                    currentSession = this.sessionManager.createSession(
                            principal,AuthnMethod.ASSERTION,
                            arg0.getSessionIndex(),arg0.getSource());

                    String tenantSessionCookieName = Shared.getTenantSessionCookieName(tenant);
                    log.trace("Setting cookie " + tenantSessionCookieName
                            + " value " + currentSession.getId());

                    Shared.addSessionCookie( tenantSessionCookieName,currentSession.getId(),response);
                }

                String rpID = chooseSignInRPSite( idmAccessor, request, response, locale);

                if (null == rpID) {
                    //skip for this request. There will be a new request with RP selected.
                    log.debug("No Relying party was selected yet. Skip sending the SAML response!");
                } else {
                    SAMLResponseSender responseSender = responseSenderFactory.buildResponseSender
                            (tenant, response, locale,
                            null,  //for IDP initiated, no relay state in post Response to SP
                            null,  //no request
                            currentSession.getAuthnMethod(),
                            currentSession.getId(),
                            currentSession.getPrincipalId(),
                            messageSource,sessionManager);

                    Document token = responseSender.generateTokenForResponse(rpID);

                    responseSender.sendResponseToRP(rpID, token);
                }

                log.info("IDP-Initiated: End processing authentication response. Session was created.");
            } else {

                log.debug("SP-Initiated: Begin processing authentication response.");
                Validate.notNull(requestState, "requestState");
                locale = requestState.getLocale();
                Validate.notNull(locale, "locale");

                tenant = requestState.getIdmAccessor().getTenant();

                PrincipalId principal = validateExternalUser(idmAccessor, arg0);
                if (principal == null) {
                    authenticationError(arg0, locale, tenant, request, response);
                    return;
                }

                //send response to initiating relying party
                requestState.addResponseHeaders(response); // set response headers
                requestState.setPrincipalId(principal);
                requestState.setAuthnMethod(AuthnMethod.ASSERTION);
                requestState.createSession(arg0.getSessionIndex(), arg0.getSource());

                String tenantSessionCookieName = Shared.getTenantSessionCookieName(tenant);
                log.trace("Setting cookie " + tenantSessionCookieName
                        + " value " + requestState.getSessionId());

                Shared.addSessionCookie( tenantSessionCookieName,requestState.getSessionId(),response);

                SAMLResponseSender responseSender = responseSenderFactory.buildResponseSender
                        (tenant, response, locale,
                        null,  //for IDP initiated, no relay state in post Response to SP
                        requestState,
                        AuthnMethod.ASSERTION,
                        requestState.getSessionId(),
                        principal,
                        messageSource,sessionManager);

                String rpID = requestState.getAuthnRequest()
                        .getIssuer().getValue();

                Validate.notEmpty(rpID, "No Relying party ID in SAML authentication request!");
                Document token = responseSender.generateTokenForResponse(rpID);
                responseSender.sendResponseToRP(rpID, token);

                log.info("SP-Initiated: End processing  Authentication response. Session was created.");
            }
        }
        catch (InvalidTokenException f) {
            log.error("Caught InvalidTokenException in validating external token {}", f.toString());
            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_NOT_ACCEPTABLE,
                    OasisNames.RESPONDER,"Unable to validate token issued by external IDP");
            arg0.setValidationResult(extResponseVr);
            authenticationError(arg0, locale, tenant, request, response);

        } catch (SamlServiceException e) {
            log.error("Caught Saml Service Exception in creating WebSSO session or issuring token. ", e.toString());
            if (!arg0.isIdpInitiated() && requestState != null) {
                requestState.setValidationResult( new ValidationResult(OasisNames.RESPONDER, e.getMessage()));
            }
            authenticationError(arg0,locale, tenant, request,response);                                                                 // server
        }
        catch (IOException g) {
            log.error("Caught IOException in sending out response or redirecting to relying party {}", g.toString());
            internalError(g, locale,tenant, request, response);
        }
    }

    /**
     * Choose a relying party site to send the assertion.
     *  1. Check RelayState
     *  2. Check if the selection is made by user.
     *  3. If not promp user for selection and return null from this function. The user selection of RP will come in as
     *      a new request.
     *
     * @param tenant
     * @param request
     * @param response
     * @param locale
     * @return EntityID string of the selected RP. Null if Choose RP dialog is called.
     * @throws SamlServiceException not able to dispatch request to RP selection jsp.
     */
    private String chooseSignInRPSite(IdmAccessor idmAccessor, HttpServletRequest request, HttpServletResponse response, Locale locale)
            throws SamlServiceException {

        Validate.notNull(idmAccessor, "idmAccessor");
        Validate.notNull(request, "request");
        Validate.notNull(response, "response");

        String tenant = idmAccessor.getTenant();
        Validate.notNull(tenant, "tenant");

        //Use RelayState if available.
        String rp_selected = request.getParameter(Shared.RELAY_STATE_PARAMETER);
        if (rp_selected != null && !rp_selected.isEmpty()) {
            String rpID;
            try {
                rpID = URLDecoder.decode(rp_selected, "UTF-8");
            } catch (UnsupportedEncodingException e) {
                throw new SamlServiceException("Failed in URL-decoding RelayState: "+rp_selected, e);
            }
            log.debug("IDP_Initiated response has relay state which will be used for" +
                    " identifying Relying Party. RelyingPartyID: "+rpID);
            return rpID;
        }

        //Check "CastleRPSelection" header
        rp_selected =request.getHeader(Shared.RP_SELECTION_ENTITYID);

        if (rp_selected != null && !rp_selected.isEmpty()) {
            log.debug("IDP_Initiated response has \"CastleRPSelection\" header which will be used for identifying Relying Party.");
            return rp_selected;
        }

        Collection<RelyingParty> rpCollection = idmAccessor.getRelyingParties(tenant);

        //Prepare lists of RP URL and name for the selection dialog
        Iterator<RelyingParty> it = rpCollection.iterator();

        //Skip the Relying party selection if only one RP is registered
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

        //dispatch selection form
        request.setAttribute(RPSelectionParam_RPNameList, rpNameList);
        request.setAttribute(RPSelectionParam_RPEntityIDList, rpEntityIdList);

        String samlResponseStr = request.getParameter(Shared.SAML_RESPONSE_PARAMETER);
        request.setAttribute(RPSelectionParam_SAMLResponse,StringEscapeUtils.escapeJavaScript( samlResponseStr));
        request.setAttribute(RPSelectionParam_DialogTittle, messageSource.getMessage("ChooseService.Title", null, locale));
        RequestDispatcher dispatcher = request.getRequestDispatcher(RPSelectionJSP);
        try {
            dispatcher.forward(request, response);
        } catch (Exception e) {
            throw new SamlServiceException("Failed in dispatch request to RPSelection.jsp", e);
        }

        return null;
    }

    /*
     * Error condition that we are not able notify SP. This error tipically encountered during sending response to SP.
     * So all we can do is send browser error message.
     *
     * (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogonProcessor#internalError(java.lang.Exception, javax.servlet.http.HttpServletRequest)
     */
    @Override
    public void internalError(Exception error, Locale locale, String tenant,
            HttpServletRequest request, HttpServletResponse response){
        log.error("Caught internalError in authenticating with external IDP! {}" , error.toString());
        Validate.notNull(response, "response");
        try {

            int httpErrorCode = HttpServletResponse.SC_INTERNAL_SERVER_ERROR;
            String messageStr = error.getMessage();
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, Shared.encodeString(messageStr));
            response.sendError(httpErrorCode, messageStr);
            log.info("Send client browser error: " + httpErrorCode
                    + ", message: " + messageStr);

        }
        catch (IOException e) {
            log.error("Caught IOException in sending out response to relying party! {}", e.toString());
        }
    }

    @Override
    public void internalError(Exception internalError, HttpServletRequest request, HttpServletResponse response) {
        internalError(internalError, null, null, request, response);
    }

    /**
     * Validate principal being asserted by IDP. Provision and update JIT user as needed.
     *  1. Validate external token from the response.
     *  2. Provision JIT user if the feature is on and the subject not found.
     *  3. Update JIT user attributes if it is existing JIT user.
     *
     * @param tenant
     * @param arg0
     * @return  PrincipalId or null if not able to link account to the incoming token
     * @throws InvalidSignatureException
     * @throws InvalidTokenException
     * @throws SystemException
     */
    private PrincipalId validateExternalUser(IdmAccessor idmAccessor, Message arg0)
            throws InvalidSignatureException, InvalidTokenException, SystemException {

        Validate.notNull(idmAccessor, "idmAccessor");
        Validate.notNull(arg0, "Message");
        String tenant = idmAccessor.getTenant();
        Validate.notNull(tenant, "tenant");

        AuthnData authnData = (AuthnData) arg0.getMessageData();
        Validate.notNull(authnData, "authnData");

        Element tokenEle = (Element) authnData.getToken().getElementsByTagNameNS(SAMLNames.NS_VAL_SAML_SAML,
                SAMLNames.ASSERTION).item(0);
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
            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(
                    HttpServletResponse.SC_NOT_ACCEPTABLE,
                    OasisNames.REQUEST_DENIED,
                    "Subject validation of external token."
                    + " Unable to validate the subject.");
            arg0.setValidationResult(extResponseVr);
            return null;
        }

        if (isJitEnabled && !isSubjectValidated) {
            try {
                subjectUpn = idmAccessor.createUserAccountJustInTime(servToken.getSubject(), tenant, extIdp);
            } catch (Exception e) {
                log.error("Failure to create a temporary user.", e);
                com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(
                        HttpServletResponse.SC_UNAUTHORIZED,
                        OasisNames.RESPONDER,
                        null);
                arg0.setValidationResult(extResponseVr);
                return null;
            }
        }

        if (isJitEnabled) {
            // update or add group information based on attribute info in new token
            try {
                idmAccessor.updateJitUserGroups(subjectUpn, tenant, extIdp.getTokenClaimGroupMappings(), authnData.getAttributes());
            } catch (Exception e){
                log.error("Encountered an error while updating Jit user groups", e);
                com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(
                        HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                        OasisNames.RESPONDER,
                        null);
                arg0.setValidationResult(extResponseVr);
                return null;
            }
        }

        return subjectUpn;
    }


    /**
     *
     * @param arg0  Message object from successful validation of SAML response message
     * @return IDPConfig for the issuer of the message, null if not found.
     */
    private IDPConfig getIssuerIDPConfig( Message arg0, IdmAccessor accessor) {

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
     * Propagate authentication status to websso client requestState.
     * check IDP side returned status first. If succeeded, check our validation status of the
     * external IDP response.
     * @param arg0 Message object passed in to the error callback.
     */

    private ValidationResult retrieveValidationResult(Message arg0) {
        Validate.notNull(arg0, "Message object");

        com.vmware.identity.websso.client.ValidationResult extResponseVr = arg0.getValidationResult();
        Validate.notNull(extResponseVr);
        ValidationResult vr = null;


        if (arg0.getStatus() != null && !arg0.getStatus().equals(OasisNames.SUCCESS)) {
            //If it is external IDP authentication error, popogated the error back to RP.
            vr = new ValidationResult(extResponseVr.getResponseCode()
                    ,arg0.getStatus(),arg0.getSubstatus());

        } else {
            //Else send the validation error to RP
            vr = new ValidationResult(extResponseVr.getResponseCode()
                        ,extResponseVr.getStatus(),extResponseVr.getSubstatus());
        }
        return vr;
    }

    /**
     * @return the requestState
     */
    public AuthnRequestState getRequestState() {
        return requestState;
    }

    /**
     * @param requestState the requestState to set
     */
    public void setRequestState(AuthnRequestState requestState) {
        this.requestState = requestState;
    }
}
