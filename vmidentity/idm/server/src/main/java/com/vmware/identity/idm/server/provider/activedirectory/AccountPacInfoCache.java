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

import java.util.Dictionary;
import java.util.Hashtable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class AccountPacInfoCache
{
    private ReentrantReadWriteLock                _accountLookupLock;
    private Dictionary<String, UserInfoEx> _accountLookup;

    public AccountPacInfoCache()
    {
        _accountLookupLock = new ReentrantReadWriteLock();
        _accountLookup = new Hashtable<String, UserInfoEx>();
    }

    public UserInfoEx findAccount(String name)
    {
        final Lock readLock = _accountLookupLock.readLock();

        readLock.lock();

        try
        {
            return _accountLookup.get(name.toLowerCase());
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addAccount(UserInfoEx accountInfo, String upn)
    {
        final Lock writeLock = _accountLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != accountInfo) {
                _accountLookup.put(upn.toLowerCase(), accountInfo);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public void deleteAccount(String name)
    {
        final Lock writeLock = _accountLookupLock.writeLock();

        writeLock.lock();

        try
        {
            _accountLookup.remove(name.toLowerCase());
        }
        finally
        {
            writeLock.unlock();
        }
    }
}
