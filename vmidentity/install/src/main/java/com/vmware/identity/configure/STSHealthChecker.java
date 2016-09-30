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
package com.vmware.identity.configure;

import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.configure.STSWebappNotDeployedException;

/**
 * A custom health checker to validates the health of all STS endpoints.
 *
 * @author bboggaramrama
 *
 */
public class STSHealthChecker {

    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory.getLogger(STSHealthChecker.class);

    private static final String REST_AFD = "/afd";
    private static final String REST_IDM = "/idm";
    //private static final String LEGACY_LS = "/lookupservice";
    private static final String OPENIDCONNECT = "/openidconnect";
    private static final String STS = "/sts";
    private static final String WEBSSO = "/websso";
   //private static final String LEGACY_ADMIN = "/sso-adminserver";
    private static final String STS_HTTP_PORT = "443";
    private static final String STS_BASE_URL = "https://localhost:" + STS_HTTP_PORT;

    private static final long MAX_TIME_TO_WAIT_MILLIS = 120000 ; // 2 minutes
    private static final long WAIT_TIME_PER_ITERATION = 5000; // 5 seconds

    private static final List<String> stsUrls = new ArrayList<String> () {
        {
            add(getStsUrl(REST_AFD));
            add(getStsUrl(REST_IDM));
           //add(getStsUrl(LEGACY_LS));
            add(getStsUrl(OPENIDCONNECT));
            add(getStsUrl(STS));
            add(getStsUrl(WEBSSO));
           // add(getStsUrl(LEGACY_ADMIN));
        }

        private String getStsUrl(String contextPath) {
            return STS_BASE_URL + contextPath;
        }
    };

    public void checkHealth() throws Exception {

        long startTimeMillis = System.currentTimeMillis();
        // Now - Monitor until all the web applications are deployed successfully.
        for (String endpoint : stsUrls) {
            while(true) {
                try{
                    URL stsEndpoint = new URL(endpoint);
                    HttpURLConnection connection = (HttpURLConnection) stsEndpoint.openConnection();
                    connection.setRequestMethod("GET");
                    connection.connect();
                    int httpResponseCode = connection.getResponseCode();
                    if (httpResponseCode == 404) {
                        String message = String.format("The webapp '%s' is either being still deployed or still being deployed. Waiting to complete deployment", endpoint);
                        throw new STSWebappNotDeployedException(message);
                    }
                }catch(STSWebappNotDeployedException e) {
                    log.error(e.getMessage());
                    long totalTimeElapsedMillis = System.currentTimeMillis() - startTimeMillis;
                    if(totalTimeElapsedMillis > MAX_TIME_TO_WAIT_MILLIS) throw e;
                    Thread.sleep(WAIT_TIME_PER_ITERATION);
                } 
            }
        }
    }
}
