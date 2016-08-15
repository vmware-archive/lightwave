/*
 *
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
 *
 */

package com.vmware.identity.samlservice.impl;

import java.io.UnsupportedEncodingException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.xml.util.Base64;
import org.springframework.stereotype.Component;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDMSecureIDNewPinException;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.RSAAMResult;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.idm.IdmPrincipalAttributesExtractorFactory;
import com.vmware.identity.samlservice.AuthenticationFilter;
import com.vmware.identity.samlservice.AuthnRequestState;
import com.vmware.identity.samlservice.IdmAccessor;
import com.vmware.identity.samlservice.SamlServiceException;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.Shared;
import com.vmware.identity.samlservice.WebSSOError;

/**
 * RSA SecureID authenticator
 *
 * @author root
 *
 */
@Component
public class AuthnRequestStateRsaAmAuthenticationFilter implements
        AuthenticationFilter<AuthnRequestState> {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(AuthnRequestStateRsaAmAuthenticationFilter.class);

    @Override
    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStateRsaAmAuthenticationFilter.preAuthenticate is called");

        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        String authData = getAuthData(request);
        if (authData == null) {
            // authentication not possible
            log.debug(Shared.REQUEST_AUTH_PARAM + " is missing, requesting "
                    + Shared.RSAAM_AUTH_PREFIX);
            t.setWwwAuthenticate(Shared.RSAAM_AUTH_PREFIX);
            throw new SamlServiceException(Shared.REQUEST_AUTH_PARAM
                    + " parameter is not found in the request.");
        }
    }

    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateRsaAmAuthenticationFilter.authenticate is called");

        Validate.notNull(t);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        AuthnRequest authnRequest = t.getAuthnRequest();
        Validate.notNull(authnRequest);

        // extract auth data
        String authData = getAuthData(request);
        Validate.notNull(authData);
        // byte[] decodedAuthData = Base64.decode(authData);

        RSAAMResult result = null;

        // call IDM to perform SecureID auth
        String authParam = request.getParameter(Shared.REQUEST_AUTH_PARAM);
        Validate.notNull(authParam);
        String[] parts = authData.split(" ");
        assert parts.length == 1 || parts.length == 2;

        byte[] decodedAuthData;
        byte[] decodedSessionID = null;
        String rsaSessionID = null;

        if (parts.length == 1) {
            decodedAuthData = Base64.decode(authData);
        }
        else if (parts.length == 2) {
            decodedSessionID = Base64.decode(parts[0]);
            decodedAuthData = Base64.decode(parts[1]);
        } else {
            throw new SamlServiceException(
                    "Wrong castle parameter. Extra parameter found in "+Shared.REQUEST_AUTH_PARAM);
        }

        try {
            // extract username and passcode from the auth data
            String unp = new String(decodedAuthData, "UTF-8");
            if (decodedSessionID != null) {
                rsaSessionID = new String(decodedSessionID, "UTF-8");
            }
            int idx = unp.indexOf(':');
            String username = idx > 0 ? unp.substring(0, idx) : null;
            String passcode = idx > 0 && unp.length() > idx+1 ? unp.substring(idx+1) : null;

            // call authenticate
            result = accessor.authenticatebyPasscode(rsaSessionID, username,
                    passcode);
            Validate.notNull(result);
        } catch (IDMSecureIDNewPinException e) {
            // failed to authenticate with passcode.
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, WebSSOError.UNAUTHORIZED, WebSSOError.SECUREID_NEWPIN_REQUIRED);
            t.setValidationResult(vr);
            throw new SamlServiceException(
                    "Failed to authenticate by passcode. ", e);

        } catch (Exception e) {
            // failed to authenticate with passcode.
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, WebSSOError.UNAUTHORIZED, null);
            t.setValidationResult(vr);
            throw new SamlServiceException(
                    "Failed to authenticate by passcode. ", e);
        }

        //Next code mode.  We include header "CastleAuthorizatin=RSAAM sessionID" in the error response.
        if (result != null) {
            if (!result.complete()) {
                // need additional auth exchange
                log.debug("Requesting next passcode.");
                String encodedRsaSessionID = Shared.encodeBytes(result.getRsaSessionID().getBytes());
                t.setWwwAuthenticate(Shared.RSAAM_AUTH_PREFIX + " " + encodedRsaSessionID);

                ValidationResult vr = new ValidationResult(
                        HttpServletResponse.SC_UNAUTHORIZED, WebSSOError.UNAUTHORIZED,
                        WebSSOError.SECUREID_NEXTCODE_MODE);
                t.setValidationResult(vr);

                throw new SamlServiceException("Require next passcode to finish the authentication.");
            }

            PrincipalId principalId = result.getPrincipalId();
            Validate.notNull(principalId);
            t.setPrincipalId(principalId);
            t.setAuthnMethod(AuthnMethod.TIMESYNCTOKEN);
        }

    }

    // return authData string from the request or null if it's not present
    private String getAuthData(HttpServletRequest request) {
        String authData = null;

        // extract auth data
        String authHeader = request.getParameter(Shared.REQUEST_AUTH_PARAM);

        if (authHeader != null
                && authHeader.startsWith(Shared.RSAAM_AUTH_PREFIX)) {
            authData = authHeader.replace(Shared.RSAAM_AUTH_PREFIX, "")
                    .trim();
        }

        return authData;
    }

    @Override
    public PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName) {
        return new IdmPrincipalAttributesExtractorFactory(idmHostName);
    }

    @Override
    public ConfigExtractorFactory getConfigExtractorFactory(String idmHostName) {
        return new ConfigExtractorFactoryImpl();
    }

    @Override
    public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    @Override
    public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }
}
