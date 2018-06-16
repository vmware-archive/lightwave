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

import static com.vmware.identity.SharedUtils.bootstrap;
import static com.vmware.identity.SharedUtils.getSTSCertificates;
import static com.vmware.identity.SharedUtils.getSTSPrivateKey;
import static com.vmware.identity.SharedUtils.getTenantEndpoint;
import static com.vmware.identity.SharedUtils.getTenantName;

import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.commons.lang.Validate;

import com.vmware.identity.TestConstants;
import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDPConfig;
import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.idm.ServerConfig;
import com.vmware.identity.saml.NoSuchIdPException;
import com.vmware.identity.saml.PrincipalAttributesExtractorFactory;
import com.vmware.identity.saml.SamlTokenSpec.AuthenticationData.AuthnMethod;
import com.vmware.identity.saml.SignatureAlgorithm;
import com.vmware.identity.saml.config.Config;
import com.vmware.identity.saml.config.Config.SamlAuthorityConfiguration;
import com.vmware.identity.saml.config.ConfigExtractor;
import com.vmware.identity.saml.config.ConfigExtractorFactory;
import com.vmware.identity.saml.config.SystemConfigurationException;
import com.vmware.identity.saml.config.TokenRestrictions;
import com.vmware.identity.samlservice.SamlValidator.ValidationResult;
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

    private final int tenantId;

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(TestAuthnRequestStateAuthenticationFilter.class);

    public TestAuthnRequestStateAuthenticationFilter(int tenantId) throws Exception {
        bootstrap();
        this.tenantId = tenantId;
    }

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
        return new MockConfigExtractorFactory();
    }

    public void preProcess(AuthnRequestState t) throws SamlServiceException {
        preAuthenticate(t);
    }

    public void process(AuthnRequestState t) throws SamlServiceException {
        authenticate(t);
    }

    class MockConfigExtractorFactory implements ConfigExtractorFactory {

        @Override
        public ConfigExtractor getConfigExtractor(String tenantName)
                throws NoSuchIdPException, SystemConfigurationException {
            return new MockConfigExtrator();
        }
    }

    class MockConfigExtrator implements ConfigExtractor {

        @Override
        public Config getConfig() throws SystemConfigurationException {
            String issuer = getTenantEndpoint(tenantId);
            String tenant = getTenantName(tenantId);
            long clockTolerance = ServerConfig.getTenantClockTolerance(tenant);
            long maxBearerTokenLiftetime = ServerConfig.getTenantMaximumBearerTokenLifetime(tenant);
            long maxHOKTokenLiftetime = ServerConfig.getTenantMaximumHokTokenLifetime(tenant);
            int delegationCount = ServerConfig.getTenantDelegationCount(tenant);
            int renewCount = ServerConfig.getTenantRenewCount(tenant);
            SamlAuthorityConfiguration samlAuthorityConfig;
            try {
                samlAuthorityConfig = new SamlAuthorityConfiguration(issuer, getSTSCertificates(), getSTSPrivateKey(),
                        SignatureAlgorithm.RSA_SHA256.toString());
                TokenRestrictions tokenRestrictions = new TokenRestrictions(maxBearerTokenLiftetime,
                        maxHOKTokenLiftetime, delegationCount, renewCount);
                List<List<Certificate>> validCertChains = new ArrayList<>();
                validCertChains.add(getSTSCertificates());
                Config config = new Config(samlAuthorityConfig, tokenRestrictions, validCertChains, clockTolerance,
                        Collections.<IDPConfig>emptyList(), new HashSet<String>());
                return config;
            } catch (Exception e) {
                throw new SystemConfigurationException(e);
            }
        }
    }
}
