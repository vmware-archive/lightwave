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

package com.vmware.identity.openidconnect.server;

import java.util.Date;

import org.junit.Assert;
import org.junit.Test;

/**
 * @author Yehia Zayour
 */
public class SlidingWindowMapTest {
    private static class TimeProvider implements SlidingWindowMap.TimeProvider {
        private Date currentTime;

        public TimeProvider(Date currentTime) {
            this.currentTime = currentTime;
        }

        public void setCurrentTime(Date currentTime) {
            this.currentTime = currentTime;
        }

        @Override
        public Date getCurrentTime() {
            return this.currentTime;
        }
    }

    @Test
    public void testAdd() {
        SlidingWindowMap<String, String> map = new SlidingWindowMap<String, String>(1 /* windowLength */, new TimeProvider(new Date()));
        map.add("k0", "v0");
        map.add("k1", "v1");
        Assert.assertEquals("v0", map.get("k0"));
        Assert.assertEquals("v1", map.get("k1"));
    }

    @Test
    public void testRemove() {
        SlidingWindowMap<String, String> map = new SlidingWindowMap<String, String>(1 /* windowLength */, new TimeProvider(new Date()));
        map.add("k0", "v0");
        map.add("k1", "v1");
        Assert.assertEquals("v0", map.remove("k0"));
        Assert.assertEquals("v1", map.remove("k1"));
        Assert.assertNull(map.get("k0"));
        Assert.assertNull(map.get("k1"));
    }

    @Test
    public void testSlidingWindow() {
        TimeProvider timeProvider = new TimeProvider(new Date());
        SlidingWindowMap<String, String> map = new SlidingWindowMap<String, String>(2 /* windowLength */, timeProvider);

        map.add("k0", "v0");
        map.add("k1", "v1");
        timeProvider.setCurrentTime(new Date(timeProvider.getCurrentTime().getTime() + 1));
        map.add("k2", "v2");
        timeProvider.setCurrentTime(new Date(timeProvider.getCurrentTime().getTime() + 1));
        map.add("k3", "v3");
        timeProvider.setCurrentTime(new Date(timeProvider.getCurrentTime().getTime() + 1));

        Assert.assertNull(map.get("k0")); // because it fell outside the sliding window
        Assert.assertNull(map.get("k1")); // because it fell outside the sliding window
        Assert.assertEquals("v2", map.get("k2"));
        Assert.assertEquals("v3", map.get("k3"));
    }
}