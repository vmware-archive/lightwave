/*
 *  Copyright (c) 2018 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.service;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManagerFactory;

import java.net.HttpURLConnection;
import java.net.URL;
import java.security.KeyStore;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.heartbeat.HeartbeatInfo;
import com.vmware.identity.heartbeat.HeartbeatStatus;
import com.vmware.identity.heartbeat.VmAfdHeartbeat;
import com.vmware.provider.VecsLoadStoreParameter;
import com.vmware.af.VmAfClient;

/**
 * A custom health checker to validate the health of all STS endpoints and components
 */
public class STSHealthChecker implements Runnable {
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(STSHealthChecker.class);

    private static final String REST_AFD = "/afd/vecs/ssl";
    private static final String REST_IDM = "/idm/";
    private static final String OPENIDCONNECT = "/openidconnect/jwks";
    private static final String STS = "/sts/STSService";
    private static final String BASE_URL = "https://%s:%d";
    private static final String VKS_KEYSTORE_INSTANCE = "VKS";
    private static final String VKS_KEYSTORE_NAME = "TRUSTED_ROOTS";
    private static final int STS_PORT = 443;
    private static final int SLEEP_SECONDS = 3;

    private List<String> stsUrls;
    private String stsBaseUrl;
    private String stsHostname;
    private VmAfdHeartbeat heartbeatHandle;
    private AtomicBoolean shouldRun;
    private static AtomicBoolean isHealthy;

    private static STSHealthChecker instance = null;

    private STSHealthChecker() {
        try {
            // get handle for local instance
            heartbeatHandle = new VmAfdHeartbeat(null, null, null);
            // get fqdn of current instance
            VmAfClient afdClient = new VmAfClient("localhost");
            stsHostname = afdClient.getDomainController();
            log.info("Initializing with hostname: {}", stsHostname);
        } catch (Exception e) {
            log.error("Failed to get hostname from vmafd: {}", e.getMessage());
            throw e;
        }

        stsBaseUrl = String.format(BASE_URL, stsHostname, STS_PORT);
        stsUrls = new ArrayList<String>() {
            {
                add(getStsUrl(REST_AFD));
                add(getStsUrl(REST_IDM));
                add(getStsUrl(OPENIDCONNECT));
                add(getStsUrl(STS));
            }
        };
        shouldRun = new AtomicBoolean();
        isHealthy = new AtomicBoolean();

        shouldRun.set(true);
        isHealthy.set(false);
    }

    /**
     * Gets instance of STSHealthChecker for the local STS
     *
     * @return STSHealthChecker
     */
    public static STSHealthChecker getInstance() {
        if (instance == null) {
            instance = new STSHealthChecker();
        }
        return instance;
    }

    private String getStsUrl(String contextPath) {
        return stsBaseUrl + contextPath;
    }

    /**
     * Starts polling thread to check health of endpoints and services
     */
    @Override
    public void run() {
        SSLContext sslContext = null;

        log.info("STS Health Checker polling for health");
        while (shouldRun.get()) {
            if (sslContext == null) {
                sslContext = initVecsSSLContext();
            }

            boolean overallStatus = sslContext != null &&
                    checkServices() &&
                    checkSTSService(sslContext);

            isHealthy.set(overallStatus);

            try {
                Thread.sleep(SLEEP_SECONDS * 1000);
            } catch (InterruptedException e) {
                break;
            }
        }
        log.info("STS Health Checker thread exiting.");
    }

    /**
     * Signals thread to stop polling
     */
    public void stop() {
        shouldRun.set(false);
    }

    /**
     * Gets Overall status of STS
     * vmca, vmafd, vmdir, vecs, and sts/oidc/sso endpoints are checked
     *
     * @return boolean
     */
    public boolean getOverallStatus() {
        return isHealthy != null && isHealthy.get();
    }

    private SSLContext initVecsSSLContext() {
        SSLContext sslContext = null;

        try {
            // Load the VKS keystore
            KeyStore vksKeyStore = KeyStore.getInstance(VKS_KEYSTORE_INSTANCE);
            vksKeyStore.load(new VecsLoadStoreParameter(VKS_KEYSTORE_NAME));
            // Initialize TrustManager and SSLFactory
            TrustManagerFactory trustMgrFactory = TrustManagerFactory.getInstance("PKIX");
            trustMgrFactory.init(vksKeyStore);
            sslContext = SSLContext.getInstance("SSL");
            sslContext.init(null, trustMgrFactory.getTrustManagers(), null);
        } catch (Exception e) {
            log.error("Failed to init SSL context with certificate from VECS: {}", e.getMessage());
            return null;
        }

        return sslContext;
    }

    private boolean checkServices() {
        HeartbeatStatus hbStatus = null;
        try {
            hbStatus = heartbeatHandle.getHeartbeatStatus();
        } catch (Exception e) {
            log.error("Failed to get Heartbeat Status from vmafd: {}", e.getMessage());
            return false;
        }

        if (hbStatus.isAlive) {
            return true;
        }

        for (HeartbeatInfo info : hbStatus.heartbeatInfo) {
            if (!info.isAlive) {
                log.error("Service {} is not live", info.serviceName);
            }
        }

        return false;
    }

    private boolean checkSTSService(SSLContext sslContext) {
        if (sslContext == null) {
            return false;
        }

        SSLSocketFactory sslFactory = sslContext.getSocketFactory();
        int aliveComponents = 0;

        // validate if all the web applications are deployed successfully.
        for (String endpoint : stsUrls) {
            try {
                int statusCode = sendHTTPSRequest(endpoint, sslFactory);

                if (statusCode == HttpURLConnection.HTTP_NOT_FOUND) {
                    log.error("Endpoint {} is not responding", endpoint);
                } else {
                    aliveComponents++;
                }
            } catch (Exception e) {
                log.error("Failed to get status of STS endpoint {}, error: {}", endpoint, e.getMessage());
            }
        }

        return aliveComponents == stsUrls.size();
    }

    private int sendHTTPSRequest(String endpoint, SSLSocketFactory sslFactory) throws Exception {
        HttpsURLConnection connection = null;
        try {
            URL stsEndpoint = new URL(endpoint);
            connection = (HttpsURLConnection) stsEndpoint.openConnection();
            connection.setSSLSocketFactory(sslFactory);
            connection.setRequestMethod("GET");
            connection.connect();

            return connection.getResponseCode();
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
        }
    }
}
