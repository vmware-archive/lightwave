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

package com.vmware.identity.sts.auth.impl;

import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;

import junit.framework.Assert;

import org.apache.commons.codec.binary.Base64;
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
import org.easymock.EasyMock;
import org.junit.BeforeClass;
import org.junit.Test;
import org.oasis_open.docs.ws_sx.ws_trust._200512.RequestSecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.BinarySecurityTokenType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.ObjectFactory;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SecurityHeaderType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.SignatureAlgorithmType;
import org.oasis_open.docs.wss._2004._01.oasis_200401_wss_wssecurity_secext_1_0.UserCertificateTokenType;
import org.w3._2000._09.xmldsig_.SignatureValueType;

import com.vmware.identity.idm.PrincipalId;
import com.vmware.identity.sts.InvalidCredentialsException;
import com.vmware.identity.sts.Request;
import com.vmware.identity.sts.auth.Authenticator;
import com.vmware.identity.sts.auth.Result;

public class UserCertAuthenticatorTest {

    static final String name = "user";
    static final String upn = name + "@ACMEC";
    static final String signedInfo = "vmware";
    static RSAPrivateKey userPrivateKey;
    static X509Certificate x509Certificate;

    public static final String ENCODING_TYPE_BASE64 =
            "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-soap-message-security-1.0#Base64Binary";
    public static final String X509_CERTIFICATE_TYPE =
            "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-x509-token-profile-1.0#X509v3";

    @BeforeClass
    public static void setUp() throws Exception {
        // create key pair and client private key, certificate
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(1024, new SecureRandom());
        KeyPair userKeyPair = keyGen.generateKeyPair();
        userPrivateKey = (RSAPrivateKey) userKeyPair.getPrivate();
        x509Certificate = generateCertificate(userKeyPair, "User");
    }

    @Test
    public void testNullUserCertificateToken() throws Exception {
        com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock.createMock(com.vmware.identity.sts.idm.Authenticator.class);
        final Authenticator authenticator = new UserCertAuthenticator(idmAuth);

        final SecurityHeaderType header = new SecurityHeaderType();
        ObjectFactory objectFactory = new ObjectFactory();
        header.getAny().add(objectFactory.createUserCertificateToken(null));

        final Result authResult = authenticator.authenticate(newReq(header));
        Assert.assertNull(authResult);
    }

    @Test(expected=InvalidCredentialsException.class)
    public void testNullSignatureAlgorithm() throws Exception {
        com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock.createMock(com.vmware.identity.sts.idm.Authenticator.class);
        final Authenticator authenticator = new UserCertAuthenticator(idmAuth);

        final SecurityHeaderType header = new SecurityHeaderType();
        final UserCertificateTokenType userCertiticateToken = new UserCertificateTokenType();
        userCertiticateToken.setSignatureAlgorithm(null);
        ObjectFactory objectFactory = new ObjectFactory();
        header.getAny().add(objectFactory.createUserCertificateToken(userCertiticateToken));

        final Result authResult = authenticator.authenticate(newReq(header));
        Assert.assertNull(authResult);
    }

    @Test(expected=InvalidCredentialsException.class)
    public void testNullSignatureValueData() throws Exception {
        com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock.createMock(com.vmware.identity.sts.idm.Authenticator.class);
        final Authenticator authenticator = new UserCertAuthenticator(idmAuth);

        final SecurityHeaderType header = new SecurityHeaderType();
        final UserCertificateTokenType userCertiticateToken = new UserCertificateTokenType();
        BinarySecurityTokenType binarySecurityToken = new BinarySecurityTokenType();
        binarySecurityToken.setValueType(X509_CERTIFICATE_TYPE);
        binarySecurityToken.setEncodingType(ENCODING_TYPE_BASE64);
        // base64 encode the x509 certificate
        binarySecurityToken.setValue(new String(Base64.encodeBase64(x509Certificate.getEncoded())));
        userCertiticateToken.setUserCertificate(binarySecurityToken);

        userCertiticateToken.setSignatureInfo(signedInfo);
        userCertiticateToken.setSignatureAlgorithm(SignatureAlgorithmType.SHA_256_WITH_RSA);

        SignatureValueType signatureValueType = new SignatureValueType();
        signatureValueType.setValue(null);
        userCertiticateToken.setSignatureValue(signatureValueType);

        ObjectFactory objectFactory = new ObjectFactory();
        header.getAny().add(objectFactory.createUserCertificateToken(userCertiticateToken));

        authenticator.authenticate(newReq(header));
    }

    @Test(expected=InvalidCredentialsException.class)
    public void testSignatureVerfificationFailure() throws Exception {
        com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock.createMock(com.vmware.identity.sts.idm.Authenticator.class);
        final Authenticator authenticator = new UserCertAuthenticator(idmAuth);

        final SecurityHeaderType header = new SecurityHeaderType();
        final UserCertificateTokenType userCertiticateToken = new UserCertificateTokenType();
        BinarySecurityTokenType binarySecurityToken = new BinarySecurityTokenType();
        binarySecurityToken.setValueType(X509_CERTIFICATE_TYPE);
        binarySecurityToken.setEncodingType(ENCODING_TYPE_BASE64);
        // base64 encode the x509 certificate
        binarySecurityToken.setValue(new String(Base64.encodeBase64(x509Certificate.getEncoded())));
        userCertiticateToken.setUserCertificate(binarySecurityToken);

        userCertiticateToken.setSignatureInfo(signedInfo);
        userCertiticateToken.setSignatureAlgorithm(SignatureAlgorithmType.SHA_256_WITH_RSA);

        SignatureValueType signatureValueType = new SignatureValueType();
        signatureValueType.setValue(new byte[]{ 0 });
        userCertiticateToken.setSignatureValue(signatureValueType);

        ObjectFactory objectFactory = new ObjectFactory();
        header.getAny().add(objectFactory.createUserCertificateToken(userCertiticateToken));

        authenticator.authenticate(newReq(header));
    }

    @Test
    public void testOK() throws Exception {
        com.vmware.identity.sts.idm.Authenticator idmAuth = EasyMock.createMock(com.vmware.identity.sts.idm.Authenticator.class);

        final PrincipalId principalIdc13d = new PrincipalId(name, "acme.com");

        EasyMock.expect(idmAuth.authenticate(EasyMock.isA(X509Certificate[].class))).andReturn(principalIdc13d);
        EasyMock.replay(idmAuth);

        final Authenticator authenticator = new UserCertAuthenticator(idmAuth);

        final SecurityHeaderType header = new SecurityHeaderType();
        final UserCertificateTokenType userCertiticateToken = new UserCertificateTokenType();
        BinarySecurityTokenType binarySecurityToken = new BinarySecurityTokenType();
        binarySecurityToken.setValueType(X509_CERTIFICATE_TYPE);
        binarySecurityToken.setEncodingType(ENCODING_TYPE_BASE64);
        // base64 encode the x509 certificate
        binarySecurityToken.setValue(new String(Base64.encodeBase64(x509Certificate.getEncoded())));
        userCertiticateToken.setUserCertificate(binarySecurityToken);

        userCertiticateToken.setSignatureInfo(signedInfo);
        userCertiticateToken.setSignatureAlgorithm(SignatureAlgorithmType.SHA_256_WITH_RSA);

        SignatureValueType signatureValueType = new SignatureValueType();
        Signature dsa = Signature.getInstance("SHA256withRSA");
        dsa.initSign(userPrivateKey);
        dsa.update(signedInfo.getBytes());
        signatureValueType.setValue(dsa.sign());
        userCertiticateToken.setSignatureValue(signatureValueType);

        ObjectFactory objectFactory = new ObjectFactory();
        header.getAny().add(objectFactory.createUserCertificateToken(userCertiticateToken));

        final Result authResult = authenticator.authenticate(newReq(header));
        Assert.assertNotNull(authResult);
        Assert.assertTrue(authResult.completed());
        Assert.assertEquals(principalIdc13d, authResult.getPrincipalId());
        Assert.assertEquals(Result.AuthnMethod.SMARTCARD, authResult.getAuthnMethod());

        EasyMock.verify(idmAuth);
    }

    private Request newReq(SecurityHeaderType header) {
        return new Request(header, new RequestSecurityTokenType(), null, null, null);
    }

    private static X509Certificate generateCertificate(KeyPair keyPair, String dn) throws Exception {
        ContentSigner sigGen = new JcaContentSignerBuilder("SHA1withRSA").build(keyPair.getPrivate());

        Date startDate = new Date(System.currentTimeMillis() - 24 * 60 * 60 * 1000);
        Date endDate = new Date(System.currentTimeMillis() + 365 * 24 * 60 * 60 * 1000);

        X509v3CertificateBuilder v3CertGen =
                new JcaX509v3CertificateBuilder(new X500Name("CN=" + dn), new BigInteger(64, new SecureRandom()), startDate, endDate,
                        new X500Name("CN=" + dn), keyPair.getPublic());
        v3CertGen.addExtension(Extension.subjectAlternativeName, true, new GeneralNames(new GeneralName(GeneralName.otherName,
                new DERSequence(new ASN1Encodable[] { new DERObjectIdentifier("1.3.6.1.4.1.311.20.2.3"),
                        new DERTaggedObject(true, 0, new DERUTF8String(upn)) }))));

        X509CertificateHolder certHolder = v3CertGen.build(sigGen);
        X509Certificate x509Certificate = new JcaX509CertificateConverter().getCertificate(certHolder);
        return x509Certificate;
    }
}
