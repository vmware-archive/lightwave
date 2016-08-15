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

package com.vmware.identity.openidconnect.protocol;

import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.net.URI;
import java.net.URLDecoder;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.SecureRandom;
import java.security.Security;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.util.Arrays;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;

import com.vmware.identity.openidconnect.common.AuthorizationCode;
import com.vmware.identity.openidconnect.common.ClientID;
import com.vmware.identity.openidconnect.common.ErrorCode;
import com.vmware.identity.openidconnect.common.Issuer;
import com.vmware.identity.openidconnect.common.JWTID;
import com.vmware.identity.openidconnect.common.Nonce;
import com.vmware.identity.openidconnect.common.PersonUserAssertionSigner;
import com.vmware.identity.openidconnect.common.Scope;
import com.vmware.identity.openidconnect.common.SessionID;
import com.vmware.identity.openidconnect.common.State;
import com.vmware.identity.openidconnect.common.StatusCode;
import com.vmware.identity.openidconnect.common.Subject;
import com.vmware.identity.openidconnect.common.TokenType;

/**
 * @author Yehia Zayour
 */
public class TestContext {
    public static final String USERNAME = "un";
    public static final String PASSWORD = "pw";
    public static final String SECURID_PASSCODE = "securid_pc";
    public static final String SECURID_SESSION_ID = "securid_session_id";
    public static final String GSS_CONTEXT_ID = "_gss_context_id_";
    public static final byte[] GSS_TICKET = new byte[1];

    public static final URI REQUEST_URI = URI.create("https://ssoserver.com/openidconnect");
    public static final URI REDIRECT_URI = URI.create("https://client.com/openidconnect/redirect");
    public static final URI POST_LOGOUT_REDIRECT_URI = URI.create("https://client.com/openidconnect/post_logout_redirect");

    public static final Scope SCOPE = Scope.OPENID;
    public static final ClientID CLIENT_ID = new ClientID("__client_id__");
    public static final State STATE = new State();
    public static final Nonce NONCE = new Nonce();
    public static final AuthorizationCode AUTHZ_CODE = new AuthorizationCode();

    public static final StatusCode STATUS_CODE = StatusCode.BAD_REQUEST;
    public static final ErrorCode ERROR_CODE = ErrorCode.INVALID_REQUEST;
    public static final String ERROR_DESCRIPTION = "error description xyz";

    public static IDToken ID_TOKEN;
    public static AccessToken ACCESS_TOKEN;
    public static RefreshToken REFRESH_TOKEN;
    public static ClientAssertion CLIENT_ASSERTION;
    public static SolutionUserAssertion SOLUTION_USER_ASSERTION;
    public static PersonUserAssertion PERSON_USER_ASSERTION;

    public static final String CERT_SUBJECT_DN = "OU=abc,C=US,DC=local,DC=vsphere,CN=_solution_username_xyz_";
    public static X509Certificate CERT;
    public static String CERT_ENCODED;

    public static void initialize() throws Exception  {
        Security.addProvider(new BouncyCastleProvider());
        KeyPairGenerator keyGenerator = KeyPairGenerator.getInstance("RSA", "BC");
        keyGenerator.initialize(1024, new SecureRandom());
        KeyPair keyPair = keyGenerator.genKeyPair();
        RSAPrivateKey privateKey = (RSAPrivateKey) keyPair.getPrivate();
        RSAPublicKey publicKey = (RSAPublicKey) keyPair.getPublic();

        CERT = generateCertificate(keyPair, CERT_SUBJECT_DN);
        CERT_ENCODED = Base64Utils.encodeToString(CERT.getEncoded());

        long lifetimeSeconds = 300;
        Date issueTime = new Date();
        Date expirationTime = new Date(issueTime.getTime() + (lifetimeSeconds * 1000L));

        ID_TOKEN = new IDToken(
                privateKey,
                TokenType.BEARER,
                new JWTID(),
                new Issuer("iss"),
                new Subject("sub"),
                Arrays.asList("aud"),
                issueTime,
                expirationTime,
                Scope.OPENID,
                "tenant",
                (ClientID) null,
                (SessionID) null,
                publicKey,
                (Subject) null,
                (Nonce) null,
                Collections.<String>emptySet(),
                "given_name",
                "family_name");

        ACCESS_TOKEN = new AccessToken(
                privateKey,
                TokenType.BEARER,
                new JWTID(),
                new Issuer("iss"),
                new Subject("sub"),
                Arrays.asList("aud"),
                issueTime,
                expirationTime,
                Scope.OPENID,
                "tenant",
                (ClientID) null,
                (SessionID) null,
                publicKey,
                (Subject) null,
                (Nonce) null,
                Collections.<String>emptySet(),
                "Administrator");

        REFRESH_TOKEN = new RefreshToken(
                privateKey,
                TokenType.BEARER,
                new JWTID(),
                new Issuer("iss"),
                new Subject("sub"),
                Arrays.asList("aud"),
                issueTime,
                expirationTime,
                Scope.OPENID,
                "tenant",
                (ClientID) null,
                (SessionID) null,
                publicKey,
                (Subject) null,
                (Nonce) null);

        CLIENT_ASSERTION = new ClientAssertion(
                privateKey,
                new JWTID(),
                CLIENT_ID,
                URI.create("https://a.com"),
                issueTime);

        SOLUTION_USER_ASSERTION = new SolutionUserAssertion(
                privateKey,
                new JWTID(),
                CERT_SUBJECT_DN,
                URI.create("https://a.com"),
                issueTime);

        PersonUserAssertionSigner testSigner = new PersonUserAssertionSigner() {
            @Override
            public byte[] signUsingRS256(byte[] data) {
                return null;
            }
        };

        PERSON_USER_ASSERTION = new PersonUserAssertion(
                testSigner,
                new JWTID(),
                CERT_SUBJECT_DN,
                URI.create("https://a.com"),
                issueTime);
    }

    public static Map<String, String> parseParameters(String query) {
        Map<String, String> params = new HashMap<>();

        StringTokenizer st = new StringTokenizer(query, "&");
        while (st.hasMoreTokens()) {
            String token = st.nextToken();
            String[] keyValuePair = token.split("=");
            String key;
            String value;
            try {
                key = URLDecoder.decode(keyValuePair[0], "UTF-8");
                value = URLDecoder.decode(keyValuePair[1], "UTF-8");
            } catch (UnsupportedEncodingException e) {
                throw new IllegalStateException(e);
            }
            params.put(key, value);
        }

        return params;
    }

    public static X509Certificate generateCertificate(
            KeyPair keyPair,
            String dn) throws OperatorCreationException, CertificateException {
        ContentSigner sigGen = new JcaContentSignerBuilder("SHA1withRSA").setProvider("BC").build(keyPair.getPrivate());

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
        X509Certificate x509Certificate = new JcaX509CertificateConverter().setProvider("BC").getCertificate(certHolder);
        return x509Certificate;
    }
}