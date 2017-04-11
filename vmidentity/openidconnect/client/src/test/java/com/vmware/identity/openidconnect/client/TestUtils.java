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

import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyStore;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;
import java.util.List;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;

import org.apache.http.conn.ssl.DefaultHostnameVerifier;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.TrustStrategy;
import org.bouncycastle.asn1.ASN1Encodable;
import org.bouncycastle.asn1.DERObjectIdentifier;
import org.bouncycastle.asn1.DERSequence;
import org.bouncycastle.asn1.DERTaggedObject;
import org.bouncycastle.asn1.DERUTF8String;
import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.asn1.x509.Extension;
import org.bouncycastle.asn1.x509.GeneralName;
import org.bouncycastle.asn1.x509.GeneralNames;
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
import com.vmware.directory.rest.client.VmdirClient;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.TokenType;
import com.vmware.identity.openidconnect.protocol.AuthorizationGrant;
import com.vmware.identity.openidconnect.protocol.Base64Utils;
import com.vmware.identity.openidconnect.protocol.RefreshTokenGrant;
import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.core.data.CertificateDTO;
import com.vmware.identity.rest.idm.client.IdmClient;

/**
 * Test Utils
 *
 * @author Jun Sun
 */
class TestUtils {

    static X509Certificate generateCertificate(KeyPair keyPair, String dn, String subjectAltName) throws Exception {
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
        if (subjectAltName != null) {
            v3CertGen.addExtension(Extension.subjectAlternativeName, true, new GeneralNames(
                    new GeneralName(GeneralName.otherName, new DERSequence(new ASN1Encodable[] {
                            new DERObjectIdentifier("1.3.6.1.4.1.311.20.2.3"),
                            new DERTaggedObject(true, 0, new DERUTF8String(subjectAltName))}))));
        }

        X509CertificateHolder certHolder = v3CertGen.build(sigGen);
        X509Certificate x509Certificate = new JcaX509CertificateConverter().getCertificate(certHolder);
        return x509Certificate;
    }

    static String buildBaseToken(Issuer issuer, String audience, String tokenClass, RSAPrivateKey rsaPrivateKey, long tokenLifeTime) throws Exception {

        Date now = new Date();
        Date issueTime = now;
        Date expirationTime = new Date(now.getTime() + tokenLifeTime);
        return buildBaseToken(issuer, audience, tokenClass, rsaPrivateKey, issueTime, expirationTime);
    }

    static String buildBaseToken(
            Issuer issuer,
            String audience,
            String tokenClass,
            RSAPrivateKey rsaPrivateKey,
            Date issueTime,
            Date expirationTime) throws Exception {

        // build an id token
        JWTClaimsSet.Builder claimsBuilder = new JWTClaimsSet.Builder();
        claimsBuilder = claimsBuilder.subject("PersonUser");
        claimsBuilder = claimsBuilder.issuer(issuer.getValue());
        claimsBuilder = claimsBuilder.claim("token_type", TokenType.HOK.getValue());
        claimsBuilder = claimsBuilder.audience(audience);
        claimsBuilder = claimsBuilder.expirationTime(expirationTime);
        claimsBuilder = claimsBuilder.claim("token_class", tokenClass);
        claimsBuilder = claimsBuilder.jwtID(new JWTID().getValue());
        claimsBuilder = claimsBuilder.issueTime(issueTime);
        claimsBuilder = claimsBuilder.claim("scope", "openid");
        claimsBuilder = claimsBuilder.claim("tenant", "test_tenant");

        SignedJWT signedJWT = new SignedJWT(new JWSHeader(JWSAlgorithm.RS256), claimsBuilder.build());
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
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNotNull(oidcTokens.getRefreshToken());

        RefreshTokenGrant refreshTokenGrant = new RefreshTokenGrant(oidcTokens.getRefreshToken().getRefreshToken());
        oidcTokens = oidclient.acquireTokens(refreshTokenGrant, TokenSpec.EMPTY);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNull(oidcTokens.getRefreshToken());
    }

    static void verifyTokensWithRefresh(
            OIDCClient oidclient,
            GSSNegotiationHandler gssNegotiationHandler,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        OIDCTokens oidcTokens = oidclient.acquireTokensByGSS(gssNegotiationHandler, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNotNull(oidcTokens.getRefreshToken());

        RefreshTokenGrant refreshTokenGrant = new RefreshTokenGrant(oidcTokens.getRefreshToken().getRefreshToken());
        oidcTokens = oidclient.acquireTokens(refreshTokenGrant, TokenSpec.EMPTY);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNull(oidcTokens.getRefreshToken());
    }

    static void verifyTokens(
            OIDCClient oidclient,
            AuthorizationGrant authorizationGrant,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        OIDCTokens oidcTokens = oidclient.acquireTokens(authorizationGrant, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIDToken());
        Assert.assertNull(oidcTokens.getRefreshToken());
    }

    static void verifyTokens(
            OIDCClient oidclient,
            GSSNegotiationHandler gssNegotiationHandler,
            TokenSpec tokenSpec) throws OIDCClientException, OIDCServerException, TokenValidationException, SSLConnectionException {
        OIDCTokens oidcTokens = oidclient.acquireTokensByGSS(gssNegotiationHandler, tokenSpec);
        Assert.assertNotNull(oidcTokens.getAccessToken());
        Assert.assertNotNull(oidcTokens.getIDToken());
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

    static void verifyOIDCClientException(
            OIDCClient oidclient,
            GSSNegotiationHandler gssNegotiationHandler,
            TokenSpec tokenSpec,
            String exceptionMessage) throws OIDCServerException, TokenValidationException, SSLConnectionException {
        boolean catched = false;
        try {
            oidclient.acquireTokensByGSS(gssNegotiationHandler, tokenSpec);
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

    static void populateSSLCertificates(
            String domainControllerFQDN,
            int domainControllerPort,
            KeyStore keyStore) throws Exception {
        AfdClient afdClient = new AfdClient(
                domainControllerFQDN,
                domainControllerPort,
                NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(
                        null,
                        new TrustStrategy() {
                            @Override
                            public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                                return true;
                            }
                        }).build());

        List<CertificateDTO> certs = afdClient.vecs().getSSLCertificates();
        int index = 1;
        for (CertificateDTO cert : certs) {
            keyStore.setCertificateEntry(String.format("VecsSSLCert%d", index), cert.getX509Certificate());
            index++;
        }
    }

    static IdmClient createIdmClient(
            AccessToken accessToken,
            String domainControllerFQDN,
            int domainControllerPort,
            KeyStore keyStore,
            RSAPrivateKey privateKey) throws Exception {
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        SSLContext sslContext = SSLContext.getInstance("SSL");
        sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
        IdmClient idmClient = new IdmClient(
                domainControllerFQDN,
                domainControllerPort,
                new DefaultHostnameVerifier(),
                sslContext);
        com.vmware.identity.rest.core.client.AccessToken restAccessToken =
                new com.vmware.identity.rest.core.client.AccessToken(
                        accessToken.getValue(),
                        privateKey == null ?
                                com.vmware.identity.rest.core.client.AccessToken.Type.JWT :
                                com.vmware.identity.rest.core.client.AccessToken.Type.JWT_HOK,
                        privateKey);
        idmClient.setToken(restAccessToken);
        return idmClient;
    }

    static VmdirClient createVMdirClient(
            AccessToken accessToken,
            String domainControllerFQDN,
            int domainControllerPort,
            KeyStore keyStore) throws Exception {
        TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
        trustManagerFactory.init(keyStore);
        SSLContext sslContext = SSLContext.getInstance("SSL");
        sslContext.init(null, trustManagerFactory.getTrustManagers(), null);
        VmdirClient vmdirClient = new VmdirClient(
                domainControllerFQDN,
                domainControllerPort,
                new DefaultHostnameVerifier(),
                sslContext);
        com.vmware.identity.rest.core.client.AccessToken restAccessToken =
                new com.vmware.identity.rest.core.client.AccessToken(accessToken.getValue(),
                        com.vmware.identity.rest.core.client.AccessToken.Type.JWT);
        vmdirClient.setToken(restAccessToken);
        return vmdirClient;
    }

    static String convertToBase64PEMString(X509Certificate x509Certificate) throws Exception {
        ByteArrayOutputStream byteArrayOutputStream = new ByteArrayOutputStream();
        byteArrayOutputStream.write("-----BEGIN CERTIFICATE-----".getBytes());
        byteArrayOutputStream.write("\n".getBytes());
        byteArrayOutputStream.write(Base64Utils.encodeToBytes(x509Certificate.getEncoded()));
        byteArrayOutputStream.write("-----END CERTIFICATE-----".getBytes());
        byteArrayOutputStream.write("\n".getBytes());
        return byteArrayOutputStream.toString();
    }
}
