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
import java.util.Locale;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.Response;
import org.springframework.context.MessageSource;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.client.SAMLNames;
import com.vmware.identity.saml.DefaultSamlAuthorityFactory;
import com.vmware.identity.saml.InvalidTokenException;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.ServerValidatableSamlToken;
import com.vmware.identity.saml.ServerValidatableSamlToken.SubjectValidation;
import com.vmware.identity.saml.ServerValidatableSamlTokenFactory;
import com.vmware.identity.saml.TokenValidator;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.OasisNames;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.websso.client.AuthnData;
import com.vmware.identity.websso.client.LogonProcessor;
import com.vmware.identity.websso.client.Message;

/**
 * Proxying service  Logon Processor Implementation
 *
 */
public class LogonProcessorImpl implements LogonProcessor {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(LogonProcessorImpl.class);

    private final ServerValidatableSamlTokenFactory tokenFactory =
            new ServerValidatableSamlTokenFactory();
    private AuthenticationFilter<AuthnRequestState> authenticator;
    private AuthnRequestState requestState;

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogonProcessor#authenticationError(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     */
    @Override
    public void authenticationError(Message arg0, HttpServletRequest request,
            HttpServletResponse response) {
        Validate.notNull(requestState, "requestState");
        try {
            if (requestState.getValidationResult().isValid())
                setSPValidationResultFromIDPValidationResult(arg0);

            Locale locale = requestState.getLocale();
            Validate.notNull(locale);

            //HttpServletResponse.SC_UNAUTHORIZED

            ValidationResult vr = requestState.getValidationResult();

            int responseCode = vr.getResponseCode();

            //default code is  HttpServletResponse.SC_UNAUTHORIZED
            if (responseCode == HttpServletResponse.SC_OK) {
            	responseCode = HttpServletResponse.SC_UNAUTHORIZED;
            }

            String message = vr.getMessage(requestState.getMessageSource(), locale);
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, message);
            response.sendError(responseCode, message);
            log.info("Responded with ERROR {} message {}",vr.getResponseCode(), message);

        } catch (Exception e) {
            log.error("Caught unexpect exception in processing authentication error {}", e.toString());

            throw new IllegalStateException(e);
        }
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogonProcessor#authenticationSuccess(com.vmware.identity.websso.client.Message, javax.servlet.http.HttpServletResponse)
     *
     * We got a validated external token from client lib. Now validate the subject in lotus,
     * issue our own token. Send response to ACS of SP.
     *
     *
     */
    @Override
    public void authenticationSuccess(Message arg0, HttpServletRequest request,
            HttpServletResponse response) {
        Validate.notNull(requestState, "requestState");

        try {

            MessageSource messageSource = requestState.getMessageSource();
            Locale locale = requestState.getLocale();
            //HttpServletResponse responseToSP = requestState.getResponse();
            String tenant = requestState.getIdmAccessor().getTenant();

            Validate.notNull(locale, "locale");
            Validate.notNull(messageSource, "messageSource");
            Validate.notNull(response, "response obj for service provider");
            Validate.notEmpty(tenant, "tenant");

            PrincipalId subjectUpn = validateExternalUser(tenant, arg0);
            if (subjectUpn == null) {
				authenticationError(arg0, request, response);
				return;
            }

            requestState.setPrincipalId(subjectUpn);
            requestState.setAuthnMethod(AuthnMethod.ASSERTION);
            requestState.createSession(arg0.getSessionIndex());

            //Generate token
            Document token = requestState.createToken();
            requestState.addResponseHeaders(response); // set response headers

            Response samlResponse = requestState.generateResponseForTenant(
            tenant, token);
            if (samlResponse == null) {
                // use validation result code to return redirect or error to
                // client
                ValidationResult vr = requestState.getValidationResult();
                if (vr.isRedirect()) {
                    response.sendRedirect(vr.getStatus());
                    log.info("Responded with REDIRECT {} target {}",vr.getResponseCode(), vr.getStatus());
                } else {
                    String message = vr.getMessage(messageSource, locale);
                    response.addHeader(Shared.RESPONSE_ERROR_HEADER, message);
                    response.sendError(vr.getResponseCode(), message);
                    log.info("Responded with ERROR {} message {}",vr.getResponseCode(), message);
                }
            } else {
                String samlResponseForm = requestState
                .generateResponseFormForTenant(samlResponse, tenant);
                log.info("SAML Response Form is {}", samlResponseForm);
                // write response
                Shared.sendResponse(response, Shared.HTML_CONTENT_TYPE, samlResponseForm);
            }
        }
        catch (InvalidTokenException f) {
            log.error("Caught InvalidTokenException in validating external token {}", f.toString());
            // Failed in creating our own token.  Report authentication error
            com.vmware.identity.websso.client.ValidationResult extResponseVr = new com.vmware.identity.websso.client.ValidationResult(HttpServletResponse.SC_NOT_ACCEPTABLE,
                    OasisNames.REQUEST_DENIED,"Unable to validate token issued by external IDP");
            arg0.setValidationResult(extResponseVr);
            authenticationError(arg0,request,response);

        } catch (SamlServiceException e) {
            log.error("Caught Saml Service Exception in creating native token after validagted via external IDP {}", e.toString());
            requestState.setValidationResult( new ValidationResult(OasisNames.RESPONDER)); // indicate
                                                                                // unexpected
                                                                                // error

            // Failed in creating our own token.  Report authentication error
            authenticationError(arg0,request,response);                                                                 // server
        }
        catch (IOException g) {
            log.error("Caught IOException in sending out response to relying party {}", g.toString());
            internalError(g, request, response);
        }
    }

    /* (non-Javadoc)
     * @see com.vmware.identity.websso.client.LogonProcessor#internalError(java.lang.Exception, javax.servlet.http.HttpServletRequest)
     */
    @Override
    public void internalError(Exception internalError,
            HttpServletRequest request, HttpServletResponse response){
        log.error("Caught internalError in authenticating with external IDP! {}" , internalError.toString());
        Validate.notNull(requestState, "requestState");
        try {
            ValidationResult vr = requestState.getValidationResult();
            MessageSource messageSource = requestState.getMessageSource();
            Locale locale = requestState.getLocale();
            Validate.notNull(locale);
            Validate.notNull(messageSource);

            String message = vr.getMessage(requestState.getMessageSource(), locale);
            response.addHeader(Shared.RESPONSE_ERROR_HEADER, message);
            response.sendError(vr.getResponseCode(), message);
            log.info("Responded with ERROR " + vr.getResponseCode()
                    + ", message " + message);

        }
        catch (IOException e) {
            log.error("Caught IOException in sending out response to relying party! {}", e.toString());
        }
    }

    private PrincipalId validateExternalUser(String tenant, Message arg0) {
        AuthnData authnData = (AuthnData) arg0.getMessageData();
        Validate.notNull(authnData, "authnData");

        Element tokenEle = (Element) authnData.getToken().getElementsByTagNameNS(SAMLNames.NS_VAL_SAML_SAML,
                SAMLNames.ASSERTION).item(0);
        ServerValidatableSamlToken servToken = tokenFactory.parseToken(tokenEle);

        DefaultSamlAuthorityFactory samlAuthFactory = requestState.getSamlAuthFactory();
        Validate.notNull(samlAuthFactory, "samlAuthFactory");

        final TokenValidator authnOnlyTokenValidator = samlAuthFactory.createAuthnOnlyTokenValidator(tenant);
        final ServerValidatableSamlToken validatedToken = authnOnlyTokenValidator.validate(servToken);

        Validate.isTrue(validatedToken.isExternal());

        IDPConfig extIdp = requestState.getExtIDPToUse();
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
                subjectUpn = requestState.getIdmAccessor().createUserAccountJustInTime(servToken.getSubject(), tenant, extIdp);
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
                requestState.getIdmAccessor().updateJitUserGroups(subjectUpn, tenant, extIdp.getTokenClaimGroupMappings(), authnData.getAttributes());
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
     * Propagate authentication status to websso client requestState.
     * check IDP side returned status first. If succeeded, check our validation status of the
     * external IDP response.
     * @param arg0 Message object passed in to the error callback.
     */

    private void setSPValidationResultFromIDPValidationResult(Message arg0) {
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
        requestState.setValidationResult(vr);
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

    /**
     * @return the authenticator
     */
    public AuthenticationFilter<AuthnRequestState> getAuthenticator() {
        return authenticator;
    }

    /**
     * @param authenticator the authenticator to set
     */
    public void setAuthenticator(AuthenticationFilter<AuthnRequestState> authenticator) {
        this.authenticator = authenticator;
    }
}
