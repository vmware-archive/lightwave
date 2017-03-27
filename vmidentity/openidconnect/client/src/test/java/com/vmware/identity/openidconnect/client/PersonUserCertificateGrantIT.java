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

package com.vmware.identity.openidconnect.client;

import java.security.InvalidKeyException;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.SignatureException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.util.Arrays;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import com.vmware.identity.openidconnect.common.PersonUserAssertionSigner;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.data.AuthenticationPolicyDTO;
import com.vmware.identity.rest.idm.data.ClientCertificatePolicyDTO;
import com.vmware.identity.rest.idm.data.TenantConfigurationDTO;

/**
 * OIDC Client Integration Test for PersonUserCertificateGrant
 *
 * @author Yehia Zayour
 */
public class PersonUserCertificateGrantIT extends OIDCClientITBase {
    private static TenantConfigurationDTO tenantConfigurationDTO;
    private static RSAPrivateKey personUserPrivateKey;
    private static X509Certificate personUserCertificate;

    @BeforeClass
    public static void setUp() throws Exception {
        OIDCClientITBase.setUp("config.properties");

        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair keyPair = keyGen.generateKeyPair();
        personUserPrivateKey = (RSAPrivateKey) keyPair.getPrivate();
        personUserCertificate = TestUtils.generateCertificate(keyPair, "PersonUser", username);

        CertificateDTO personUserCertificateDTO = new CertificateDTO.Builder().
                withEncoded(TestUtils.convertToBase64PEMString(personUserCertificate)).build();

        tenantConfigurationDTO = idmClient.tenant().getConfig(tenant);
        AuthenticationPolicyDTO authenticationPolicyDTO = tenantConfigurationDTO.getAuthenticationPolicy();

        ClientCertificatePolicyDTO clientCertificatePolicyDTO = new ClientCertificatePolicyDTO.Builder().
                withCertPolicyOIDs(null).
                withTrustedCACertificates(Arrays.asList(personUserCertificateDTO)).
                withRevocationCheckEnabled(true).
                withOcspEnabled(false).
                withFailOverToCrlEnabled(false).
                withOcspUrlOverride(null).
                withCrlDistributionPointUsageEnabled(true).
                withCrlDistributionPointOverride(null).build();
        AuthenticationPolicyDTO authenticationPolicyDTOUpdate = new AuthenticationPolicyDTO.Builder().
                withPasswordBasedAuthenticationEnabled(authenticationPolicyDTO.isPasswordBasedAuthenticationEnabled()).
                withWindowsBasedAuthenticationEnabled(authenticationPolicyDTO.isWindowsBasedAuthenticationEnabled()).
                withCertificateBasedAuthenticationEnabled(true).
                withClientCertificatePolicy(clientCertificatePolicyDTO).build();
        TenantConfigurationDTO tenantConfigurationDTOUpdate = new TenantConfigurationDTO.Builder().
                withPasswordPolicy(tenantConfigurationDTO.getPasswordPolicy()).
                withLockoutPolicy(tenantConfigurationDTO.getLockoutPolicy()).
                withTokenPolicy(tenantConfigurationDTO.getTokenPolicy()).
                withProviderPolicy(tenantConfigurationDTO.getProviderPolicy()).
                withBrandPolicy(tenantConfigurationDTO.getBrandPolicy()).
                withAuthenticationPolicy(authenticationPolicyDTOUpdate).build();
        idmClient.tenant().updateConfig(tenant, tenantConfigurationDTOUpdate);
    }

    @AfterClass
    public static void tearDown() throws Exception {
        OIDCClientITBase.tearDown();
        idmClient.tenant().updateConfig(tenant, tenantConfigurationDTO);
    }

    @Test
    public void testSuccess() throws Exception {
        OIDCTokens oidcTokens = nonRegNoHOKConfigClient.acquireTokensByPersonUserCertificate(
                personUserCertificate,
                new TestPersonUserAssertionSigner(personUserPrivateKey),
                withRefreshSpec);
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getRefreshToken());

        Assert.assertTrue("id_token subject", username.equalsIgnoreCase(oidcTokens.getIDToken().getSubject().getValue()));
    }

    @Test
    public void testFailWithInvalidSignature() throws Exception {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair keyPair = keyGen.generateKeyPair();
        RSAPrivateKey wrongPrivateKey = (RSAPrivateKey) keyPair.getPrivate();

        try {
            nonRegNoHOKConfigClient.acquireTokensByPersonUserCertificate(
                    personUserCertificate,
                    new TestPersonUserAssertionSigner(wrongPrivateKey),
                    withoutRefreshSpec);
            Assert.fail("expecting OIDCServerException");
        } catch (OIDCServerException e) {
            Assert.assertEquals("error_description", "person_user_assertion has an invalid signature", e.getErrorObject().getDescription());
        }
    }

    private class TestPersonUserAssertionSigner implements PersonUserAssertionSigner {
        private final RSAPrivateKey privateKey;

        public TestPersonUserAssertionSigner(RSAPrivateKey privateKey) {
            this.privateKey = privateKey;
        }

        @Override
        public byte[] signUsingRS256(byte[] data) {
            try {
                Signature signer = Signature.getInstance("SHA256withRSA");
                signer.initSign(this.privateKey);
                signer.update(data);
                return signer.sign();
            } catch (NoSuchAlgorithmException | InvalidKeyException | SignatureException e) {
                throw new IllegalStateException("failed to sign", e);
            }
        }
    }
}
