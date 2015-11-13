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

package com.vmware.identity.idm.server.provider.activedirectory;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class GroupSidToNameInfoCache
{
    private ReentrantReadWriteLock _groupNameLookupLock;

    private final Map<String, GroupSidToNameInfo> cacheMap; // LRU cache

    public GroupSidToNameInfoCache(final int cacheSize)
    {
        _groupNameLookupLock = new ReentrantReadWriteLock();

        this.cacheMap = new LinkedHashMap<String, GroupSidToNameInfo>(cacheSize, 0.75f, true) {
            /**
             *
             */
            private static final long serialVersionUID = -1632117332383495887L;

            @Override
            protected boolean removeEldestEntry(Map.Entry<String, GroupSidToNameInfo> eldest)
            {
                return size() > cacheSize; // size exceeds the max allowed
            }
        };
    }

    public GroupSidToNameInfo findGroupName(String groupSid)
    {
        final Lock readLock = _groupNameLookupLock.readLock();

        readLock.lock();

        try
        {
            return this.cacheMap.get(groupSid);
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addGroupName(String groupSid, GroupSidToNameInfo groupSidToNameInfo)
    {
        final Lock writeLock = _groupNameLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != groupSidToNameInfo) {
                this.cacheMap.put(groupSid, groupSidToNameInfo);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }
}
