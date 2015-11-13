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

package com.vmware.identity.idm.server.provider.ldap;

import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Hashtable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class ConnectionInfoCache
{
    private ReentrantReadWriteLock     _connLookupLock;
    private Dictionary<String, ArrayList<String>> _connLookup; // key on domain name

    public ConnectionInfoCache()
    {
        _connLookupLock = new ReentrantReadWriteLock();
        _connLookup = new Hashtable<String, ArrayList<String>>();
    }

    public ArrayList<String> findConnInfo(String name)
    {
        final Lock readLock = _connLookupLock.readLock();

        readLock.lock();

        try
        {
            return _connLookup.get(name.toLowerCase());
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addConnInfo(String domainName, ArrayList<String> connInfo)
    {
        final Lock writeLock = _connLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != connInfo) {
                _connLookup.put(domainName.toLowerCase(), connInfo);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public void deleteConnInfo(String name)
    {
        final Lock writeLock = _connLookupLock.writeLock();

        writeLock.lock();

        try
        {
            _connLookup.remove(name.toLowerCase());
        }
        finally
        {
            writeLock.unlock();
        }
    }
}
