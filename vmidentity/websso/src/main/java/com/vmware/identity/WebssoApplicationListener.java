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
package com.vmware.identity;

import org.springframework.context.ApplicationListener;
import org.springframework.context.event.ApplicationContextEvent;
import org.springframework.context.event.ContextClosedEvent;
import org.springframework.context.event.ContextRefreshedEvent;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.heartbeat.VmAfdHeartbeat;

public class WebssoApplicationListener implements ApplicationListener<ApplicationContextEvent> {

    private static final int port = 443;
    private static final String serviceName = "Websso";
    private static final IDiagnosticsLogger log = DiagnosticsLoggerFactory
            .getLogger(WebssoApplicationListener.class);

    private VmAfdHeartbeat heartbeat = new VmAfdHeartbeat(serviceName, port);

    @Override
    public void onApplicationEvent(ApplicationContextEvent contextEvent) {

        if (contextEvent instanceof ContextRefreshedEvent) {
            heartbeat.startBeating();
            log.info("Heartbeat started");
        }
        else if (contextEvent instanceof ContextClosedEvent)
        {
            heartbeat.stopBeating();
            log.info("Heartbeat stopped");
        }
    }
}
