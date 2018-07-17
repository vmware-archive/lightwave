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
package com.vmware.identity.openidconnect.server;

import org.apache.commons.lang.StringUtils;

import io.prometheus.client.Counter;
import io.prometheus.client.Histogram;

public class MetricUtils {
    private static final Counter totalRequests = Counter.build()
            .name("oidc_requests_total").help("Total requests.")
            .labelNames("tenant", "status", "resource", "operation")
            .register();

    private static final Histogram requestLatency = Histogram.build()
            .name("oidc_requests_latency_seconds").help("Request latency in seconds.")
            .labelNames("tenant", "resource", "operation")
            .buckets(0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 3.0, 4.0)
            .register();

    private MetricUtils() {

    }

    public static void increaseRequestCount(String tenant, String status, String resource, String operation) {
        tenant = StringUtils.isEmpty(tenant) ? "unknown" : tenant;
        status = StringUtils.isEmpty(status) ? "unknown" : status;
        resource = StringUtils.isEmpty(resource) ? "unknown" : resource;
        operation = StringUtils.isEmpty(operation) ? "unknown" : operation;
        totalRequests.labels(tenant, status, resource, operation).inc();
    }

    public static Histogram.Timer startRequestTimer(String tenant, String resource, String operation) {
        tenant = StringUtils.isEmpty(tenant) ? "unknown" : tenant;
        resource = StringUtils.isEmpty(resource) ? "unknown" : resource;
        operation = StringUtils.isEmpty(operation) ? "unknown" : operation;
        return requestLatency.labels(tenant, resource, operation).startTimer();
    }
}
