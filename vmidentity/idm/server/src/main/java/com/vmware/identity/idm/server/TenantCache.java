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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Dictionary;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.vmware.identity.idm.RSAAgentConfig;

public class TenantCache
{
    private ReentrantReadWriteLock                _tenantLookupLock;
    private Dictionary<String, TenantInformation> _tenantLookup;

    //this setting reflect what is persisted on local server. It could be lagging behind a refreshed tenantInformation
    private ReentrantReadWriteLock             _extRsaAgentConfigLookupLock;
    private Dictionary<String, RSAAgentConfig> _extRsaAgentConfigLookup;

    private ReentrantReadWriteLock   _defaultTenantLock;
    private String _defaultTenant;

    private ReentrantReadWriteLock   _systemTenantLock;
    private String _systemTenant;

    public TenantCache()
    {
        _tenantLookupLock = new ReentrantReadWriteLock();
        _tenantLookup = new Hashtable<String, TenantInformation>();

        _defaultTenantLock = new ReentrantReadWriteLock();
        _defaultTenant = null;

        _systemTenantLock = new ReentrantReadWriteLock();
        _systemTenant = null;

        _extRsaAgentConfigLookupLock = new ReentrantReadWriteLock();
        _extRsaAgentConfigLookup = new Hashtable<String, RSAAgentConfig>();
    }

    public TenantInformation findTenant(String name)
    {
        final Lock readLock = _tenantLookupLock.readLock();

        readLock.lock();

        try
        {
            return _tenantLookup.get(name.toLowerCase());
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addTenant(TenantInformation tenantInfo)
    {
        final Lock writeLock = _tenantLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != tenantInfo) {
                _tenantLookup.put(tenantInfo.getTenant().getName().toLowerCase(), tenantInfo);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public void deleteTenant(String name)
    {
        final Lock writeLock = _tenantLookupLock.writeLock();

        writeLock.lock();

        try
        {
            _tenantLookup.remove(name.toLowerCase());

            // reset the _defaultTenant
            // no need to do this for _systemTenant, since it should never be removed
            if (name != null && name.equalsIgnoreCase(_defaultTenant))
            {
                _defaultTenant = null;
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public String setDefaultTenant(String defaultTenant)
    {
        final Lock writeLock = _defaultTenantLock.writeLock();

        writeLock.lock();

        try
        {
            _defaultTenant = defaultTenant;

            return _defaultTenant;
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public String findDefaultTenant()
    {
        final Lock readLock = _defaultTenantLock.readLock();

        readLock.lock();

        try
        {
            return _defaultTenant;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public String setSystemTenant(String systemTenant)
    {
        final Lock writeLock = _systemTenantLock.writeLock();

        writeLock.lock();

        try
        {
            _systemTenant = systemTenant;

            return _systemTenant;
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public String findSystemTenant()
    {
        final Lock readLock = _systemTenantLock.readLock();

        readLock.lock();

        try
        {
            return _systemTenant;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public Collection<String> findAllTenants()
    {
        final Lock readLock = _tenantLookupLock.readLock();
        Collection<String> result = new ArrayList<String>();

        readLock.lock();

        try
        {
            Enumeration<String> keys = _tenantLookup.keys();

            while (keys.hasMoreElements())
            {
                result.add(keys.nextElement());
            }

            return result;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public RSAAgentConfig findExtRsaConfig(String name)
    {
        final Lock readLock = _extRsaAgentConfigLookupLock.readLock();

        readLock.lock();

        try
        {
            return _extRsaAgentConfigLookup.get(name.toLowerCase());
        }
        finally
        {
            readLock.unlock();
        }
    }

    public void addExtRsaConfig(String tenantName, RSAAgentConfig config)
    {
        final Lock writeLock = _extRsaAgentConfigLookupLock.writeLock();

        writeLock.lock();

        try
        {
            if (null != config && null != tenantName && !tenantName.isEmpty()) {
                _extRsaAgentConfigLookup.put(tenantName.toLowerCase(), config);
            }
        }
        finally
        {
            writeLock.unlock();
        }
    }

    public void deleteExtRsaConfig(String name)
    {
        final Lock writeLock = _extRsaAgentConfigLookupLock.writeLock();

        writeLock.lock();

        try
        {
            _extRsaAgentConfigLookup.remove(name.toLowerCase());
        }
        finally
        {
            writeLock.unlock();
        }
    }

}
