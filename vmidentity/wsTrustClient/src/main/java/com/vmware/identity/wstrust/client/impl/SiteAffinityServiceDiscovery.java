/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *   use this file except in compliance with the License.  You may obtain a copy
 *   of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS, without
 *   warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *   License for the specific language governing permissions and limitations
 *   under the License.
 */
package com.vmware.identity.wstrust.client.impl;

import java.net.URL;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.cdc.CdcDCEntry;
import com.vmware.identity.cdc.CdcFactory;
import com.vmware.identity.cdc.CdcSession;
import com.vmware.identity.cdc.CdcState;
import com.vmware.identity.wstrust.client.SecurityTokenServiceConfig.ConnectionConfig;
import com.vmware.identity.wstrust.client.ServerCommunicationException;

class SiteAffinityServiceDiscovery implements ServiceDiscovery {

    private final ConnectionConfig connConfig;
    private boolean useSiteAffinity;
    private static final int FLAGS = 0;
    private static final String HTTP = "http";
    private static final String HTTPS = "https";
    private static final String DEFAULT_FILE = "/sts/STSService/";
    private static final int DEFAULT_PORT = -1;
    private static final Logger log = LoggerFactory.getLogger(SiteAffinityServiceDiscovery.class);

    public SiteAffinityServiceDiscovery(ConnectionConfig connConfig) {
        this.connConfig = connConfig;
        this.useSiteAffinity = connConfig.getUrl() == null;
    }

    @Override
    public URL getServiceLocation() {
        if (!useSiteAffinity)
            return connConfig.getUrl();

        URL connConfirUrl = connConfig.getUrl();
        try (CdcSession cdcSession = CdcFactory.createCdcSessionViaIPC()) {

            boolean isSiteAffinityDisabled = cdcSession.getCdcState() == CdcState.CDC_STATE_DISABLED;
            if (isSiteAffinityDisabled) {
                log.info("Site affinity is disabled");
                return connConfirUrl;
            }

            return getDomainController(cdcSession);
        } catch (ServerCommunicationException e) {
            log.error(String.format("Failed to create affinitized URL %s", e.toString()));

            throw e;
        } catch (Exception e) {
            log.warn(String.format("Failed to create affinitized URL %s", e.toString()));
            // when failure is at creating the session or getting site-affinity
            // status don't throw exception, because it can't be determined if
            // site-affinity is enabled
            return connConfirUrl;
        }
    }

    @Override
    public boolean retryConnection() {
        return useSiteAffinity && connConfig.getMaxRetryAttempts() > 0;
    }

    private URL getDomainController(CdcSession cdcSession) {
        try {
            // when first parameter is null the default domain from vmafd is
            // used
            CdcDCEntry domainController = cdcSession.getAffinitizedDC(null, FLAGS);

            log.info(String.format("Site affinity DC name: %s", domainController.dcName));

            String protocol = connConfig.getSSLTrustedManagerConfig() != null ? HTTPS : HTTP;
            URL affinitizedUrl = new URL(protocol, domainController.dcName, DEFAULT_PORT, DEFAULT_FILE
                    + connConfig.getTenant());

            return affinitizedUrl;
        } catch (Exception e) {
            throw new ServerCommunicationException("Failed to create affinitized URL", e);
        }
    }
}
