/*
 *  Copyright (c) 2012-2016 VMware, Inc.  All Rights Reserved.
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
package com.vmware.identity.idm.server.provider;

import com.vmware.identity.idm.server.config.IdmServerConfig;

class LdapConnectionPoolConfigManager {

    private static final boolean TEST_ON_BORROW = true;
    // no limit per key
    private static final int MAX_CONNECTIONS_PER_KEY = -1;

    public LdapConnectionPoolConfig getConfiguration(String tenantName) {
	LdapConnectionPoolConfig poolConfig = new LdapConnectionPoolConfig();
	IdmServerConfig serverConfig = IdmServerConfig.getInstance();
	poolConfig.setTestOnBorrow(TEST_ON_BORROW);
	poolConfig.setTimeBetweenEvictionRunsMillis(serverConfig.getLdapConnPoolEvictionInterval());
	poolConfig.setMinEvictableIdleTimeMillis(serverConfig.getLdapConnPoolIdleTime());
	poolConfig.setMaxConnections(serverConfig.getLdapConnPoolMaxConnections());
	poolConfig.setMaxConnectionsPerKey(MAX_CONNECTIONS_PER_KEY);
	poolConfig.setMaxWaitMillis(serverConfig.getLdapConnPoolMaxWait());

	return poolConfig;
    }
}
