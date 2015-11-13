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

import java.util.Dictionary;
import java.util.Hashtable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.vmware.identity.interop.domainmanager.DomainControllerInfo;

public class DcInfoCache
{
    private ReentrantReadWriteLock                _dcLookupLock;
    private Dictionary<String, DomainControllerInfo> _dcLookup; // key on domain's NetBios name

    public DcInfoCache()
    {
        _dcLookupLock = new ReentrantReadWriteLock();
        _dcLookup = new Hashtable<String, DomainControllerInfo>();
    }

    public DomainControllerInfo findDcInfo(String name)
    {
        final Lock readLock = _dcLookupLock.readLock();

        readLock.lock();

        try
        {
            return _dcLookup.get(name.toLowerCase());
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addDcInfo(DomainControllerInfo dcInfo)
    {
        final Lock writeLock = _dcLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != dcInfo) {
                _dcLookup.put(dcInfo.domainNetBiosName.toLowerCase(), dcInfo);
                _dcLookup.put(dcInfo.domainName.toLowerCase(), dcInfo);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public void deleteDcInfo(String name)
    {
        final Lock writeLock = _dcLookupLock.writeLock();

        writeLock.lock();

        try
        {
            _dcLookup.remove(name.toLowerCase());
        }
        finally
        {
            writeLock.unlock();
        }
    }
}
