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

import org.apache.commons.lang3.Validate;

/**
 * Client configuration
 *
 * @author Jun Sun
 */
public class ClientConfig {
    private final ConnectionConfig connectionConfig;
    private final ClientID clientId;
    private final HolderOfKeyConfig holderOfKeyConfig;
    private final HighAvailabilityConfig highAvailabilityConfig;

    /**
     * Constructor
     *
     * @param connectionConfig                  Server connection configuration.
     * @param clientId                          OIDC registered client Id.
     * @param holderOfKeyConfig                 Client key configuration.
     */
    public ClientConfig(
            ConnectionConfig connectionConfig,
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig) {
        this(connectionConfig, clientId, holderOfKeyConfig, (HighAvailabilityConfig) null);
    }

    /**
     * Constructor
     *
     * @param connectionConfig                  Server connection configuration.
     * @param clientId                          OIDC registered client Id.
     * @param holderOfKeyConfig                 Client key configuration.
     * @param highAvailabilityConfig            High Availability / Site Affinity config.
     */
    public ClientConfig(
            ConnectionConfig connectionConfig,
            ClientID clientId,
            HolderOfKeyConfig holderOfKeyConfig,
            HighAvailabilityConfig highAvailabilityConfig) {
        Validate.notNull(connectionConfig, "connectionConfig");

        this.connectionConfig = connectionConfig;
        this.clientId = clientId;
        this.holderOfKeyConfig = holderOfKeyConfig;
        this.highAvailabilityConfig = highAvailabilityConfig;
    }

    /**
     * Get connection config
     *
     * @return                          Connection config object
     */
    public ConnectionConfig getConnectionConfig() {
        return this.connectionConfig;
    }

    /**
     * Get client id
     *
     * @return                          Client Id
     */
    public ClientID getClientId() {
        return this.clientId;
    }

    /**
     * Get holder of key config
     *
     * @return                          Holder of key config object
     */
    public HolderOfKeyConfig getHolderOfKeyConfig() {
        return this.holderOfKeyConfig;
    }

    /**
     * Get high availability config
     *
     * @return                          High availability config object
     */
    public HighAvailabilityConfig getHighAvailabilityConfig() {
        return this.highAvailabilityConfig;
    }
}
