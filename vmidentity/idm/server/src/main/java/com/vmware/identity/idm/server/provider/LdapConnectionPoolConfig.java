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

class LdapConnectionPoolConfig {

    private boolean testOnBorrow;
    private long evictionInterval;
    private long idleTime;
    private int maxConnections;
    private long maxWaitMilis;
    private int maxConnectionsPerKey;

    public boolean getTestOnBorrow() {
	return testOnBorrow;
    }

    public long getEvictionInterval() {
	return evictionInterval;
    }

    public long getIdleTime() {
	return idleTime;
    }

    public int getMaxConnections() {
	return maxConnections;
    }

    public long getMaxWaitMilis() {
	return maxWaitMilis;
    }

    public int getMaxConnectionsPerKey() {
	return maxConnectionsPerKey;
    }

    public void setTestOnBorrow(boolean testOnBorrow) {
	this.testOnBorrow = testOnBorrow;
    }

    public void setTimeBetweenEvictionRunsMillis(long evictionInterval) {
	this.evictionInterval = evictionInterval;

    }

    public void setMinEvictableIdleTimeMillis(long idleTime) {
	this.idleTime = idleTime;
    }

    public void setMaxConnections(int maxConnections) {
	this.maxConnections = maxConnections;
    }

    public void setMaxWaitMillis(long maxWait) {
	this.maxWaitMilis = maxWait;
    }

    public void setMaxConnectionsPerKey(int maxConnectionsPerKey) {
	this.maxConnectionsPerKey = maxConnectionsPerKey;
    }
}
