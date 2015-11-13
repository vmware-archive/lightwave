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
import com.vmware.identity.idm.GSSResult;
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

/**
 * @author root
 * 
 */
@Component
public class AuthnRequestStateKerbAuthenticationFilter implements
        AuthenticationFilter<AuthnRequestState> {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(AuthnRequestStateKerbAuthenticationFilter.class);

    @Override
    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStateKerbAuthenticationFilter.preAuthenticate is called");

        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);

        // then check if required auth header is present
        if (request.getParameter(Shared.REQUEST_AUTH_HEADER) == null) {
            // authentication not possible
            log.debug("REQUEST_AUTH_HEADER is missing, requesting KERB_AUTH_PREFIX");
            t.setWwwAuthenticate(Shared.KERB_AUTH_PREFIX);
            t.setValidationResult(new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, "Unauthorized", null));
            throw new SamlServiceException();
        }
    }

    @Override
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        log.debug("AuthnRequestStateKerbAuthenticationFilter.authenticate is called");

        Validate.notNull(t);
        IdmAccessor accessor = t.getIdmAccessor();
        Validate.notNull(accessor);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        AuthnRequest authnRequest = t.getAuthnRequest();
        Validate.notNull(authnRequest);

        GSSResult result = null;

        // call IDM to perform GSS auth
        String authHeader = request.getParameter(Shared.REQUEST_AUTH_HEADER);
        Validate.notNull(authHeader);
        String authData = authHeader.replace(Shared.KERB_AUTH_PREFIX, "")
                .trim();
        String[] parts = authData.split(" ");
        assert parts.length == 2;
        String contextId = parts[0];
        byte[] decodedAuthData = Base64.decode(parts[1]);

        try {
            result = accessor.authenticate(contextId, decodedAuthData);
        } catch (Exception ex) {
            // Could not authenticate with GSS, send browser login credential
            // error message. this allow user fall back to using password
            // authentication.
            ValidationResult vr = new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, "Unauthorized", null);
            t.setValidationResult(vr);
            throw new SamlServiceException();
        }

        if (result != null) {
            if (!result.complete()) {
                // need additional auth exchange
                log.debug("Requesting more auth data");
                String encodedAuthData = Shared.encodeBytes(result
                        .getServerLeg());
                t.setWwwAuthenticate(Shared.KERB_AUTH_PREFIX + " " + contextId
                        + " " + encodedAuthData);
                throw new SamlServiceException();
            }

            PrincipalId principalId = result.getPrincipalId();
            Validate.notNull(principalId);
            t.setPrincipalId(principalId);
            t.setAuthnMethod(AuthnMethod.KERBEROS);
        }
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
