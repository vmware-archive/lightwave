/*
 *  Copyright (c) 2012-2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.websso.client.sample;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.KeyManagementException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;

import java.security.cert.X509Certificate;
import java.util.List;

import org.apache.http.HttpException;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.conn.ssl.NoopHostnameVerifier;
import org.apache.http.ssl.SSLContextBuilder;
import org.apache.http.ssl.TrustStrategy;

import com.vmware.identity.rest.afd.client.AfdClient;
import com.vmware.identity.rest.core.client.exceptions.ClientException;
import com.vmware.identity.rest.core.client.exceptions.WebApplicationException;
import com.vmware.identity.rest.core.data.CertificateDTO;

public class ServiceLocator {

    private X509Certificate[] sslCertificates;
    private String host;
    private String tenant;

    static {
	    javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(
	    new javax.net.ssl.HostnameVerifier(){

	        public boolean verify(String hostname,
	                javax.net.ssl.SSLSession sslSession) {
	            if (hostname.equals("localhost")) {
	                return true;
	            }
	            return false;
	        }
	    });
	}

    public ServiceLocator(String host, String tenant) throws Exception {
        this.sslCertificates = getSSLCertificates(host);
        this.host = host;
        this.tenant = tenant;
    }

    public X509Certificate[] getAdminSSLCertificates() {
        return sslCertificates;
    }

    public X509Certificate[] getSTSSSLCertificates() {
        return sslCertificates;
    }

    public URL getSTSURL() throws MalformedURLException {
        return new URL("https", host, "/sts/STSService/" + tenant);
    }

    public URL getWebssoUrl() throws MalformedURLException {
        return new URL("https", host, "/websso/SAML2/Metadata/" + tenant);
    }

    public X509Certificate getWebssoSSLCertificates() {
        if (sslCertificates != null && sslCertificates.length > 0)
            return sslCertificates[0];


        return null;
    }

    public String getTenant() {
        return tenant;
    }

    public String getHost() {
        return host;
    }

    private X509Certificate[] getSSLCertificates(String host) throws KeyManagementException, NoSuchAlgorithmException,
    KeyStoreException, ClientProtocolException, WebApplicationException, ClientException, HttpException,
    IOException {
        X509Certificate[] x509Certs = null;
        AfdClient client = new AfdClient(host, NoopHostnameVerifier.INSTANCE, new SSLContextBuilder()
                .loadTrustMaterial(null, new TrustStrategy() {

                    @Override
                    public boolean isTrusted(X509Certificate[] chain, String authType) throws CertificateException {
                        return true;
                    }
                }).build());

        List<CertificateDTO> sslCerts = client.vecs().getSSLCertificates();
        if (sslCerts != null) {
            x509Certs = new X509Certificate[sslCerts.size()];
            int i = 0;
            for (CertificateDTO certificateDTO : sslCerts) {
                x509Certs[i] = certificateDTO.getX509Certificate();
                i++;
            }
        }

        return x509Certs;
    }
}
