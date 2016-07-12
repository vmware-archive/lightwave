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

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.TestConstants;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
import com.vmware.identity.samlservice.impl.ConfigExtractorFactoryImpl;
import com.vmware.identity.samlservice.impl.PrincipalAttributesExtractorFactoryImpl;

/**
 * Test implementation of authentication filter which always returns same
 * principal id
 *
 */
public class TestAuthnRequestStateAuthenticationFilter implements
        AuthenticationFilter<AuthnRequestState> {
    private static final String REQUEST_AUTH_HEADER = Shared.REQUEST_AUTH_PARAM;
    private static final String AUTH_PREFIX = Shared.KERB_AUTH_PREFIX;

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(TestAuthnRequestStateAuthenticationFilter.class);

    public void preAuthenticate(AuthnRequestState t)
            throws SamlServiceException {
        log.debug("AuthnRequestStateAuthenticationFilter.preAuthenticate is called");

        Validate.notNull(t);
        HttpServletRequest request = t.getRequest();
        Validate.notNull(request);
        if (request.getParameter(REQUEST_AUTH_HEADER) == null) {
            // authentication not possible
            log.debug(REQUEST_AUTH_HEADER + " is missing, requesting "
                    + AUTH_PREFIX);
            t.setWwwAuthenticate(AUTH_PREFIX);
            t.setValidationResult(new ValidationResult(
                    HttpServletResponse.SC_UNAUTHORIZED, "Unauthorized", null));
            throw new SamlServiceException(REQUEST_AUTH_HEADER
                    + " is missing, requesting " + AUTH_PREFIX);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see
     * com.vmware.identity.samlservice.AuthenticationFilter#authenticate(java
     * .lang.Object)
     */
    public void authenticate(AuthnRequestState t) throws SamlServiceException {
        t.setPrincipalId(new PrincipalId(TestConstants.USER,
                TestConstants.DOMAIN));
        t.setAuthnMethod(AuthnMethod.KERBEROS);
    }

    public PrincipalAttributesExtractorFactory getPrincipalAttributeExtractorFactory(
            String idmHostName) {
        return new PrincipalAttributesExtractorFactoryImpl();
    }

    public ConfigExtractorFactory getConfigExtractorFactory(String idmHostName) {
        return new ConfigExtractorFactoryImpl();
    }

    public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }
}
