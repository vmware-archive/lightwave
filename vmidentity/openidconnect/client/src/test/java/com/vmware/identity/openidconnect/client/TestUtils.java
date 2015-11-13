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

import java.math.BigInteger;
import java.security.KeyPair;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;

import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.junit.Assert;

import com.nimbusds.jose.JWSAlgorithm;
import com.nimbusds.jose.JWSHeader;
import com.nimbusds.jose.JWSSigner;
import com.nimbusds.jose.crypto.RSASSASigner;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * Test Utils
 *
 * @author Jun Sun
 */
class TestUtils {

    static X509Certificate generateCertificate(KeyPair keyPair, String dn) throws Exception {
        ContentSigner sigGen = new JcaContentSignerBuilder("SHA1withRSA").build(keyPair.getPrivate());

        Date startDate = new Date(System.currentTimeMillis() - 24 * 60 * 60 * 1000);
        Date endDate = new Date(System.currentTimeMillis() + 365 * 24 * 60 * 60 * 1000);

        X509v3CertificateBuilder v3CertGen =
                new JcaX509v3CertificateBuilder(new X500Name("CN=" + dn),
                        new BigInteger(64, new SecureRandom()),
                        startDate,
                        endDate,
                        new X500Name("CN=" + dn),
                        keyPair.getPublic());

        X509CertificateHolder certHolder = v3CertGen.build(sigGen);
        X509Certificate x509Certificate = new JcaX509CertificateConverter().getCertificate(certHolder);
        return x509Certificate;
    }

    static String buildBaseToken(Issuer issuer, String audience, String tokenClass, RSAPrivateKey rsaPrivateKey, Long tokenLifeTime) throws Exception {

        // build an id token
        JWTClaimsSet claimsSet = new JWTClaimsSet();
        claimsSet.setIssuer(issuer.getValue());
        claimsSet.setClaim("token_type", TokenType.HOK.getName());
        claimsSet.setAudience(audience);
        claimsSet.setExpirationTime(new Date(new Date().getTime() + tokenLifeTime));
        claimsSet.setClaim("token_class", tokenClass);

        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsSet);
        JWSSigner signer = new RSASSASigner(rsaPrivateKey);
        signedJWT.sign(signer);

        return signedJWT.serialize();
    }

    static void verifyTokensWithRefresh(
            OIDCClient oidclient,
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        OIDCTokens oidcTokens = oidclient.acquireTokens(authorizationGrant, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIdToken());
        Assert.assertNotNull(oidcTokens.getRefreshToken());

        RefreshTokenGrant refreshTokenGrant = new RefreshTokenGrant(new RefreshToken(oidcTokens.getRefreshToken().getValue()));
        oidcTokens = oidclient.acquireTokens(refreshTokenGrant, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIdToken());
        Assert.assertNull(oidcTokens.getRefreshToken());
    }

    static void verifyTokens(
            OIDCClient oidclient,
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        OIDCTokens oidcTokens = oidclient.acquireTokens(authorizationGrant, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIdToken());
        Assert.assertNull(oidcTokens.getRefreshToken());
    }

    static void verifyOIDCClientException(
            OIDCClient oidclient,
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec,
            String exceptionMessage) throws OIDCServerException, TokenValidationException, SSLConnectionException {
        boolean catched = false;
        try {
            oidclient.acquireTokens(authorizationGrant, tokenSpec);
        } catch (OIDCClientException e) {
            if (e.getMessage().equals(exceptionMessage)) {
                catched = true;
            }
        }
        Assert.assertTrue(catched);
    }

    static void verifyOIDCServerException(
            OIDCClient oidclient,
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec,
            String exceptionMessage) throws TokenValidationException, SSLConnectionException, OIDCClientException {
        boolean catched = false;
        try {
            oidclient.acquireTokens(authorizationGrant, tokenSpec);
        } catch (OIDCServerException e) {
            if (e.getMessage().equals(exceptionMessage)) {
                catched = true;
            }
        }
        Assert.assertTrue(catched);
    }
}
