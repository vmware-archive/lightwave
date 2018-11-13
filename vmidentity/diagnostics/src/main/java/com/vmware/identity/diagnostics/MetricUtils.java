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
package com.vmware.identity.diagnostics;

import org.apache.commons.lang.StringUtils;

import io.prometheus.client.Counter;
import io.prometheus.client.Histogram;
import io.prometheus.client.Gauge;

public class MetricUtils {
    private static final Counter totalRequests = Counter.build()
            .name("oidc_requests_total").help("Total requests.")
            .labelNames("status", "resource", "operation")
            .register();
    private static final Histogram requestLatency = Histogram.build()
            .name("oidc_requests_latency_seconds").help("Request latency in seconds.")
            .labelNames("resource", "operation")
            .buckets(0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 3.0, 4.0)
            .register();
    private static final Gauge federatedUILogins = Gauge.build()
            .name("federated_user_login_ui")
            .labelNames("tenant")
            .help("Total logins in Lightwave using federated SP or IDP init flow")
            .register();
    private static final Gauge federatedTokenLogins = Gauge.build()
            .name("federated_user_login_token")
            .labelNames("tenant", "client_id")
            .help("Total logins in Lightwave using federated token grant")
            .register();

    public static void increaseRequestCount(String status, String resource, String operation) {
        status = StringUtils.isEmpty(status) ? "unknown" : status;
        resource = StringUtils.isEmpty(resource) ? "unknown" : resource;
        operation = StringUtils.isEmpty(operation) ? "unknown" : operation;
        totalRequests.labels(status, resource, operation).inc();
    }

    public static Histogram.Timer startRequestTimer(String resource, String operation) {
        resource = StringUtils.isEmpty(resource) ? "unknown" : resource;
        operation = StringUtils.isEmpty(operation) ? "unknown" : operation;
        return requestLatency.labels(resource, operation).startTimer();
    }

    public static void recordFederatedUILogin(String tenant) {
        tenant = (tenant == null || StringUtils.isEmpty(tenant)) ? "unknown" : tenant;
        federatedUILogins.labels(tenant).inc();
    }

    public static void recordFederatedTokenLogin(String tenant, String clientId) {
        tenant = (tenant == null || StringUtils.isEmpty(tenant)) ? "unknown" : tenant;
        clientId = (clientId == null || StringUtils.isEmpty(clientId)) ? "none" : clientId;
        federatedTokenLogins.labels(tenant, clientId).inc();
    }
}