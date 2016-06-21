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
package com.vmware.identity.samlservice.impl;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;
import org.opensaml.saml2.core.AuthnRequest;
import org.opensaml.xml.util.Base64;
import org.springframework.stereotype.Component;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.PrincipalId;
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
 * Username/password authenticator
 *
 * @author root
 *
 */
@Component
public class AuthnRequestStatePasswordAuthenticationFilter implements
        AuthenticationFilter<AuthnRequestState> {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(AuthnRequestStatePasswordAuthenticationFilter.class);

    @Override
    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStatePasswordAuthenticationFilter.preAuthenticate is called");

        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        String authData = getAuthData(request);
        if (authData == null) {
            // authentication not possible
            log.debug(Shared.REQUEST_AUTH_PARAM + " is missing, requesting "
                    + Shared.PASSWORD_AUTH_PREFIX);
            t.setWwwAuthenticate(Shared.PASSWORD_AUTH_PREFIX);
            throw new SamlServiceException();
        }
    }

    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStatePasswordAuthenticationFilter.authenticate is called");

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
        byte[] decodedAuthData = Base64.decode(authData);

        PrincipalId result = null;
        try {
            // extract username and password from the auth data
            String unp = new String(decodedAuthData, "UTF-8");
            int idx = unp.indexOf(':');
            String username = idx > 0 ? unp.substring(0, idx) : null;
            String password = idx > 0 && unp.length() > idx+1 ? unp.substring(idx+1) : null;

            // call authenticate
            result = accessor.authenticate(username, password);
            Validate.notNull(result);
        } catch (Exception e) {
            // failed to authenticate with username/password.
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED,
                    WebSSOError.UNAUTHORIZED, null);
            t.setValidationResult(vr);
            throw new SamlServiceException();
        }

        if (result != null) {
            t.setPrincipalId(result);
            t.setAuthnMethod(AuthnMethod.PASSWORD);
        }
    }

    // return authData string from the request or null if it's not present
    private String getAuthData(HttpServletRequest request) {
        String authData = null;

        // extract auth data
        String authHeader = request.getParameter(Shared.REQUEST_AUTH_PARAM);

        if (authHeader != null
                && authHeader.startsWith(Shared.PASSWORD_AUTH_PREFIX)) {
            authData = authHeader.replace(Shared.PASSWORD_AUTH_PREFIX, "")
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
