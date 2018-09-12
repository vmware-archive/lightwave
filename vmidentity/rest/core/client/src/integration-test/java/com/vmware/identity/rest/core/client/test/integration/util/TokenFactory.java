/* **********************************************************************
 * Copyright 2015 VMware, Inc. All rights reserved.
 * *********************************************************************/
package com.vmware.identity.rest.core.client.test.integration.util;

import java.io.IOException;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.xml.soap.SOAPException;

import org.apache.http.client.ClientProtocolException;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.conn.ssl.TrustStrategy;
import org.apache.http.ssl.SSLContextBuilder;

import com.vmware.identity.rest.core.client.AccessToken;
import com.vmware.identity.rest.core.client.exceptions.ClientException;

public class TokenFactory {

    private static String getOidcAccessToken(String host, String tenant, String username, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        OIDCClient client = new OIDCClient(host,
                NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType)
                            throws CertificateException {
                        return true;
                    }
                }).build());

        return client.getAccessToken(tenant, username, password);
    }

    private static String getSamlAccessToken(String host, String tenant, String username, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, SOAPException, IOException, ClientException {
        SAMLClient client = new SAMLClient(host, NoopHostnameVerifier.INSTANCE,
                new SSLContextBuilder().loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                        return true;
                    }
                }).build());
        return client.getAccessToken(tenant, username, password);
    }

    public static String getAccessToken(String host, String tenant, String username, String password, AccessToken.Type tokenType) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException, SOAPException {
        String token = null;
        if (tokenType == AccessToken.Type.JWT) {
            token = TokenFactory.getOidcAccessToken(host, tenant, username, password);
        } else if (tokenType == AccessToken.Type.SAML) {
            token = TokenFactory.getSamlAccessToken(host, tenant, username, password);
        } else {
            throw new IllegalStateException(String.format("Token type %s is not supported.", tokenType.name()));
        }
        return token;
    }

    public static String getAccessToken(String host, String tenant, String username, String password) throws KeyManagementException, NoSuchAlgorithmException, KeyStoreException, ClientProtocolException, ClientException, IOException {
        // return oidc token by default
        return getOidcAccessToken(host, tenant, username, password);
    }
}
