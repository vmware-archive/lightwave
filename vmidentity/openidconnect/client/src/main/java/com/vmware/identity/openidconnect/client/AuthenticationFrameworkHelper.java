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

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.net.URL;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import net.minidev.json.JSONArray;
import net.minidev.json.JSONObject;
import net.minidev.json.JSONValue;

import org.apache.commons.codec.binary.Base64;

import sun.security.provider.X509Factory;

import com.nimbusds.oauth2.sdk.ParseException;
import com.nimbusds.oauth2.sdk.http.HTTPRequest;
import com.nimbusds.oauth2.sdk.http.HTTPResponse;

/**
 * Authentication framework helper class
 *
 * @author Jun Sun
 */
public class AuthenticationFrameworkHelper {
    private final URL authenticationFrameworkSSLCertificateURL;

    public AuthenticationFrameworkHelper(String domainControllerFQDN) {
        this(
                domainControllerFQDN,
                OIDCClientUtils.DEFAULT_OP_PORT);
    }

    public AuthenticationFrameworkHelper(
            String domainControllerFQDN,
            int domainControllerPort) {
        this.authenticationFrameworkSSLCertificateURL = OIDCClientUtils.buildEndpointUrl(
                OIDCClientUtils.buildBaseUrl(
                        domainControllerFQDN,
                        domainControllerPort),
                "/afd/vecs/ssl");
    }

    public void populateSSLCertificates(KeyStore keyStore) throws OIDCClientException, SSLConnectionException, AdminServerException {

        HTTPRequest httpRequest = new HTTPRequest(HTTPRequest.Method.GET, this.authenticationFrameworkSSLCertificateURL);
        HTTPResponse httpResponse = null;

        SSLContext sslContext;
        try {
            sslContext = SSLContext.getInstance("TLS");
            TrustManager tm = new X509TrustManager() {
                @Override
                public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                }
                @Override
                public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                }
                @Override
                public X509Certificate[] getAcceptedIssuers() {
                    return null;
                }
            };
            sslContext.init(null, new TrustManager[] { tm }, null);
        } catch (NoSuchAlgorithmException | KeyManagementException e) {
            throw new OIDCClientException("Failed to build SSL context: " + e.getMessage(), e);
        }

        httpResponse = OIDCClientUtils.sendSecureRequest(httpRequest, sslContext);
        try {
            if (httpResponse.getStatusCode() != 200 && httpResponse.getStatusCode() != 204) {
                throw AdminServerHelper.convertToAdminServerException(httpResponse.getStatusCode(), httpResponse.getContentAsJSONObject());
            }
        } catch (ParseException e) {
            throw new OIDCClientException("Exception caught during exception conversion: " + e.getMessage(), e);
        }

        JSONArray jsonArray = (JSONArray) JSONValue.parse(httpResponse.getContent());
        int index = 1;
        for (Object object : jsonArray) {
            JSONObject jsonObject = (JSONObject) object;
            String cert = (String) jsonObject.get("encoded");
            cert = cert.replaceAll(X509Factory.BEGIN_CERT, "").replaceAll(X509Factory.END_CERT, "");
            try {
                keyStore.setCertificateEntry(String.format("VecsSSLCert%d", index), convertToX509Certificate(cert));
            } catch (KeyStoreException e) {
                throw new OIDCClientException("Failed to set X509 certificate in key store: " + e.getMessage(), e);
            }
            index++;
        }
    }

    private X509Certificate convertToX509Certificate(String base64PEMCertificate) throws OIDCClientException {
        X509Certificate x509Certificate = null;
        InputStream is = new ByteArrayInputStream(Base64.decodeBase64(base64PEMCertificate.getBytes()));
        CertificateFactory cf;
        try {
            cf = CertificateFactory.getInstance("X509");
            x509Certificate = (X509Certificate) cf.generateCertificate(is);
        } catch (CertificateException e) {
            throw new OIDCClientException("Failed to convert to X509 certificate: " + e.getMessage(), e);
        }
        return x509Certificate;
    }
}
