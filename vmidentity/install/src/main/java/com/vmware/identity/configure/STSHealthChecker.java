package com.vmware.identity.configure;

/*
 *  Copyright (c) 2016 VMware, Inc.  All Rights Reserved.
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

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

import java.io.IOException;
import java.net.URL;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.configure.STSWebappNotDeployedException;
import com.vmware.provider.VecsLoadStoreParameter;

/**
 * A custom health checker to validates the health of all STS endpoints.
 *
 * @author bboggaramrama
 *
 */
public class STSHealthChecker {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(STSHealthChecker.class);

    // Endpoint candidates for health check
    private static final String REST_AFD = "/afd/vecs/ssl";
    private static final String REST_IDM = "/idm/";
    private static final String OPENIDCONNECT = "/openidconnect/jwks";
    private static final String STS = "/sts/STSService";


    private String sts_https_url;
    private static final String STS_BASE_URL = "https://%s:%s";

    private static final String VKS_KEYSTORE_INSTANCE = "VKS";
    private static final String VKS_KEYSTORE_NAME = "TRUSTED_ROOTS";

    private static final long MAX_TIME_TO_WAIT_MILLIS = 120000 ; // 2 minutes
    private static final long WAIT_TIME_PER_ITERATION = 5000; // 5 seconds

    private  List<String> stsUrls;

    public STSHealthChecker(String hostname, String portnum) {

        sts_https_url = String.format(STS_BASE_URL, hostname, portnum) ;
        stsUrls  = new ArrayList<String> () {
            {
                add(getStsUrl(REST_AFD));
                add(getStsUrl(REST_IDM));
                add(getStsUrl(OPENIDCONNECT));
                add(getStsUrl(STS));
            }};
    }

    private String getStsUrl(String contextPath) {
        return sts_https_url + contextPath;
    }

    public void checkHealth() throws Exception {

        // Load the VKS keystore
        KeyStore vksKeyStore = getVksKeyStore();

        // Initialize TrustManager and SSLFactory
        TrustManagerFactory trustMgrFactory = TrustManagerFactory.getInstance("PKIX");
        trustMgrFactory.init(vksKeyStore);
        SSLContext sslCnxt = SSLContext.getInstance("SSL");
        sslCnxt.init(null, trustMgrFactory.getTrustManagers(), null);
        SSLSocketFactory sslFactory = sslCnxt.getSocketFactory();
        HttpsURLConnection connection = null;
        long startTimeMillis = System.currentTimeMillis();
        // validate if all the web applications are deployed successfully.
        for (String endpoint : stsUrls) {
            System.out.println(String.format("Checking health of endpoint: '%s'", endpoint));
            while(true) {
                try {
                    URL stsEndpoint = new URL(endpoint);
                    connection =  (HttpsURLConnection) stsEndpoint.openConnection();
                    connection.setSSLSocketFactory(sslFactory);
                    connection.setRequestMethod("GET");
                    connection.connect();
                    int httpResponseCode = connection.getResponseCode();
                    if (httpResponseCode == 404) {
                        String message = String.format("The webapp '%s' is either being still deployed or still being deployed. Waiting to complete deployment", endpoint);
                        throw new STSWebappNotDeployedException(message);
                    } else {
                        System.out.println(String.format("The endpoint : '%s' is deployed successfully", endpoint));
                        break;
                    }
                }catch(Exception e) {
                    log.error(e.getMessage());
                    long totalTimeElapsedMillis = System.currentTimeMillis() - startTimeMillis;

                    if(totalTimeElapsedMillis > MAX_TIME_TO_WAIT_MILLIS) {
                        throw e;
                    }
                    Thread.sleep(WAIT_TIME_PER_ITERATION);
                } finally {
                    connection.disconnect();
                }

            }
        }
    }

    /**
     * Get VKS keystore by calling VECS
     */
    public KeyStore getVksKeyStore() throws KeyStoreException, NoSuchAlgorithmException, CertificateException, IOException {
        KeyStore ks = KeyStore.getInstance(VKS_KEYSTORE_INSTANCE);
        ks.load(new VecsLoadStoreParameter(VKS_KEYSTORE_NAME));
        return ks;
    }
}
