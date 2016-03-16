/*
 *
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
 *
 */

package com.vmware.identity.idm.server.performance;

import java.util.concurrent.ConcurrentHashMap;

import com.vmware.identity.idm.server.config.IdmServerConfig;

public class PerformanceMonitor implements IPerformanceMonitor {
    private ConcurrentHashMap<String, IdmAuthStatCache> _cacheMap;
    private static PerformanceMonitor _instance = new PerformanceMonitor();

    private PerformanceMonitor() {
        this._cacheMap = new ConcurrentHashMap<String, IdmAuthStatCache>();
    }

    public static PerformanceMonitor getInstance() {
        return _instance;
    }

    @Override
    public IdmAuthStatCache getCache(String tenantName) {
        assert (tenantName != null && !tenantName.isEmpty());

        IdmAuthStatCache cache = this._cacheMap.putIfAbsent(tenantName, new IdmAuthStatCache(getDefaultCacheSize(), false));
        return cache != null? cache : this._cacheMap.get(tenantName);
    }

    @Override
    public void deleteCache(String tenantName) {
        assert (tenantName != null && !tenantName.isEmpty());

        this._cacheMap.remove(tenantName);
    }

    @Override
    public int getDefaultCacheSize() {
        return IdmServerConfig.getInstance().getIdmAuthStatsCacheDepth();
    }

    @Override
    public boolean summarizeLdapQueries() {
        return IdmServerConfig.getInstance()
                .getIdmAuthStatsSummarizeLdapQueries();
    }
}