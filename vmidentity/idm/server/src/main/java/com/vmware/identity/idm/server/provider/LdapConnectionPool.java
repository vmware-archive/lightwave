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

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.apache.commons.pool2.impl.GenericKeyedObjectPool;
import org.apache.commons.pool2.impl.GenericKeyedObjectPoolConfig;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.server.config.IdmServerConfig;
import com.vmware.identity.interop.ldap.ILdapConnectionEx;

public final class LdapConnectionPool {

    private final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(LdapConnectionPool.class);
    private final static LdapConnectionPool INSTANCE = new LdapConnectionPool();
    private final ConcurrentMap<String, GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx>> poolMap;
    private final LdapConnectionPoolConfigManager configManager = new LdapConnectionPoolConfigManager();

    private LdapConnectionPool() {
        String systemTenant = IdmServerConfig.getInstance().getDirectoryConfigStoreDomain().toLowerCase();
        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> systemTenantPool =
                new GenericKeyedObjectPool<>(new PooledLdapConnectionFactory(), getGenericKeyedObjectPoolConfig(systemTenant));
        this.poolMap = new ConcurrentHashMap<>();
        this.poolMap.put(systemTenant, systemTenantPool);
    }

    public static LdapConnectionPool getInstance() {
        return INSTANCE;
    }

    public ILdapConnectionEx borrowConnection(PooledLdapConnectionIdentity identity) throws Exception {
        String tenantName = identity.getTenantName().toLowerCase();

        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> currentPool = poolMap.get(tenantName);

        if (currentPool == null) {
            throw new IllegalStateException("Connection pool does not exist for tenant " + tenantName);
        }

        return currentPool.borrowObject(identity);
    }

    public void returnConnection(PooledLdapConnection pooledConnection) {
        if (pooledConnection == null || pooledConnection.getIdentity() == null || pooledConnection.getConnection() == null) {
            return;
        }

        String tenantName = pooledConnection.getIdentity().getTenantName().toLowerCase();

        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> pool = poolMap.get(tenantName);
        if (pool != null) {
            pool.returnObject(pooledConnection.getIdentity(), pooledConnection.getConnection());
        } else {
            pooledConnection.getConnection().close();
        }
    }

    public void createPool(String tenantName) {

        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> newPool = null;
        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> currentPool = poolMap
                .get(tenantName.toLowerCase());

        if (currentPool == null) {
            newPool = new GenericKeyedObjectPool<>(new PooledLdapConnectionFactory(),
                    getGenericKeyedObjectPoolConfig(tenantName));
            currentPool = poolMap.putIfAbsent(tenantName.toLowerCase(), newPool);
            if (currentPool != null) {
                newPool.close();
            }
            logger.info(currentPool != null ? "Pool created for tenant " + tenantName
                    : "Pool already existed for tenant " + tenantName);
        }
    }

    public void cleanPool(String tenantName) {
        GenericKeyedObjectPool<PooledLdapConnectionIdentity, ILdapConnectionEx> pool = poolMap
                .remove(tenantName.toLowerCase());
        if (pool != null) {
            pool.close();
            logger.info("Pool closed for tenant " + tenantName);
        }
    }

    private GenericKeyedObjectPoolConfig getGenericKeyedObjectPoolConfig(String tenantName) {
        LdapConnectionPoolConfig config = configManager.getConfiguration(tenantName);
        GenericKeyedObjectPoolConfig poolConfig = new GenericKeyedObjectPoolConfig();

        poolConfig.setTestOnBorrow(config.getTestOnBorrow());
        poolConfig.setTimeBetweenEvictionRunsMillis(config.getEvictionInterval());
        poolConfig.setMinEvictableIdleTimeMillis(config.getIdleTime());
        poolConfig.setMaxTotal(config.getMaxConnections());
        poolConfig.setMaxTotalPerKey(config.getMaxConnectionsPerKey());
        poolConfig.setMaxWaitMillis(config.getMaxWaitMilis());

        return poolConfig;
    }

}
