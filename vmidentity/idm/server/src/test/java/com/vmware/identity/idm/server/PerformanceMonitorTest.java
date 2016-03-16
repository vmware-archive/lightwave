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

package com.vmware.identity.idm.server;

import org.junit.Assert;
import org.junit.Test;

import com.vmware.identity.idm.server.performance.IdmAuthStatCache;
import com.vmware.identity.performanceSupport.IIdmAuthStat.ActivityKind;
import com.vmware.identity.performanceSupport.IIdmAuthStat.EventLevel;
import com.vmware.identity.performanceSupport.IdmAuthStat;

public class PerformanceMonitorTest {
    @Test
    public void testIdmAuthStatCacheSingleThread() throws Exception {
        int size = 10;
        IdmAuthStatCache cache = new IdmAuthStatCache(size, true);

        // element is added till cache size is reached.
        int current = 0;
        while (current < size) {
            cache.add(new IdmAuthStat("", "", current,
                    ActivityKind.AUTHENTICATE, EventLevel.INFO, null, 0, 0, null, null, null));
            Assert.assertEquals(cache.getIdmAuthStats().size(), current + 1);
            Assert.assertEquals(cache.getIdmAuthStats().get(current)
                    .getProviderFlag(), current);
            ++current;
        }

        // when cache size is reached, the first is removed, the new one added
        // to the end.
        cache.add(new IdmAuthStat("", "", current, ActivityKind.AUTHENTICATE, EventLevel.INFO,
                null, 0, 0, null, null, null)); // current == 10
        Assert.assertEquals(cache.getIdmAuthStats().size(), size);
        Assert.assertEquals(cache.getIdmAuthStats().get(size - 1)
                .getProviderFlag(), current);
        Assert.assertEquals(cache.getIdmAuthStats().get(0).getProviderFlag(), 1);
        ++current;

        // check again
        cache.add(new IdmAuthStat("", "", current, ActivityKind.AUTHENTICATE, EventLevel.INFO,
                null, 0, 0, null, null, null)); // current == 11
        Assert.assertEquals(cache.getIdmAuthStats().size(), size);
        Assert.assertEquals(cache.getIdmAuthStats().get(size - 1)
                .getProviderFlag(), current);
        Assert.assertEquals(cache.getIdmAuthStats().get(0).getProviderFlag(), 2);
    }

    class Adder extends Thread {
        private IdmAuthStatCache _cache;
        private int _count;

        public Adder(IdmAuthStatCache cache, int count) {
            this._cache = cache;
            this._count = count;
        }

        public void run() {
            int current = 0;
            while (current < this._count) {
                this._cache.add(new IdmAuthStat("", "", 0,
                        ActivityKind.AUTHENTICATE, EventLevel.INFO, null, 0, 0, null, null, null));
                ++current;
            }
        }
    }

    @Test
    public void testIdmAuthStatCacheMultiThread() throws Exception {
        int cacheSize = 10;
        IdmAuthStatCache cache = new IdmAuthStatCache(cacheSize, true);

        int threadCount = 5;
        Adder[] ts = new Adder[threadCount];
        for (int i = 0; i < threadCount; i++) {
            ts[i] = new Adder(cache, 20);
            ts[i].start();
        }

        for (int i = 0; i < threadCount; i++) {
            ts[i].join();
        }

        // Thread-safe check: size should not exceed cacheSize
        Assert.assertEquals(cache.getIdmAuthStats().size(), cacheSize);
    }
}
