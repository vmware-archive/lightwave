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

package com.vmware.identity.idm.server.test.integration.util;

import com.vmware.identity.idm.server.performance.IPerformanceMonitor;
import com.vmware.identity.idm.server.performance.IdmAuthStatCache;

public class NoopPerfMonitor implements IPerformanceMonitor {

    private IdmAuthStatCache cache;

    public NoopPerfMonitor() {
        this.cache = new IdmAuthStatCache(0, false);
    }

    @Override
    public IdmAuthStatCache getCache(String tenantName) {
        return cache;
    }

    @Override
    public void deleteCache(String tenantName) {
    }

    @Override
    public int getDefaultCacheSize() {
        return 0;
    }

    @Override
    public boolean summarizeLdapQueries() {
        return false;
    }
}