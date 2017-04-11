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

package com.vmware.identity.openidconnect.server;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.KeyPair;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.security.interfaces.RSAPrivateKey;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;

import com.nimbusds.jose.JOSEException;
import com.nimbusds.jwt.JWTClaimsSet;
import com.nimbusds.jwt.SignedJWT;
import com.vmware.identity.openidconnect.protocol.JWTUtils;

/**
 * @author Yehia Zayour
 */
public class TestUtil {
    public static MockHttpServletRequest createGetRequest(Map<String, String> params) {
        MockHttpServletRequest request = new MockHttpServletRequest("GET", "");
        request.setServerName(TestContext.SERVER_NAME);
        request.setParameters(multiValuedMap(params));
        return request;
    }

    public static MockHttpServletRequest createPostRequest(Map<String, String> params) throws IOException {
        MockHttpServletRequest request = new MockHttpServletRequest("POST", "");
        request.setServerName(TestContext.SERVER_NAME);
        request.setContentType("application/x-www-form-urlencoded");
        request.setParameters(multiValuedMap(params));
        return request;
    }

    public static MockHttpServletRequest createPostRequestWithQueryString(
            Map<String, String> formParams,
            Map<String, String> queryParams) throws IOException {
        Map<String, String> allParams = new HashMap<String, String>();
        allParams.putAll(formParams);
        allParams.putAll(queryParams);
        MockHttpServletRequest request = createPostRequest(allParams);
        return request;
    }

    public static Map<String, String> parseParameters(String query) throws UnsupportedEncodingException {
        Map<String, String> params = new HashMap<>();

        StringTokenizer st = new StringTokenizer(query, "&");
        while (st.hasMoreTokens()) {
            String token = st.nextToken();
            String[] keyValuePair = token.split("=");
            String key = URLDecoder.decode(keyValuePair[0], "UTF-8");
            String value = URLDecoder.decode(keyValuePair[1], "UTF-8");
            params.put(key, value);
        }

        return params;
    }

    // JAXP fails to parse the authn response html so we need to extract fields manually
    public static String extractString(MockHttpServletResponse response, String prefix, String suffix) {
        String result;

        String html;
        try {
            html = response.getContentAsString();
        } catch (UnsupportedEncodingException e) {
            throw new IllegalArgumentException(e);
        }

        int index = html.indexOf(prefix);
        if (index >= 0) {
            int startIndex = index + prefix.length();
            int endIndex = html.indexOf(suffix, startIndex);
            result = html.substring(startIndex, endIndex);
        } else {
            result = null;
        }

        return result;
    }

    public static String urlEncode(String value) {
        try {
            return URLEncoder.encode(value, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new IllegalStateException(e);
        }
    }

    public static SignedJWT sign(
            JWTClaimsSet claims,
            RSAPrivateKey privateKey) {
        try {
            return JWTUtils.signClaimsSet(claims, privateKey);
        } catch (JOSEException e) {
            throw new IllegalStateException("failed to sign jwt", e);
        }
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

    private static Map<String, String[]> multiValuedMap(Map<String, String> singleValuedMap) {
        Map<String, String[]> multiValuedMap = new HashMap<String, String[]>();
        for (Map.Entry<String, String> entry : singleValuedMap.entrySet()) {
            multiValuedMap.put(entry.getKey(), new String[] { entry.getValue() });
        }
        return multiValuedMap;
    }
}