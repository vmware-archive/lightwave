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

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.ActiveDirectoryJoinInfo;
import com.vmware.identity.idm.server.DcInfoCache;
import com.vmware.identity.idm.server.vmaf.VmafClientUtil;
import com.vmware.identity.interop.domainmanager.DomainAdapterFactory;
import com.vmware.identity.interop.domainmanager.DomainControllerInfo;
import com.vmware.identity.interop.domainmanager.DomainManagerException;
import com.vmware.identity.interop.domainmanager.DomainManagerNativeException;
import com.vmware.identity.interop.domainmanager.DomainTrustInfo;
import com.vmware.identity.interop.domainmanager.HostNotJoinedException;
import com.vmware.identity.interop.domainmanager.IDomainAdapter;

public class IdmDomainState
{
    private static IdmDomainState instance = null;

    class IdmDomainStateRetriever extends Thread
    {
        @Override
        public void run()
        {
            while(true)
            {
                // Go online to refresh host domain related information
                boolean bSucceed = retrieveDomainInfo();

                try
                {
                    // refresh domain trust information every 6 hours on succeed
                    if (bSucceed)
                    {
                        Thread.sleep(time_interval_on_succeed);
                    }
                    // On failure, be more eager to refresh in 10 minutes
                    else
                    {
                        Thread.sleep(time_interval_on_fail);
                    }
                }
                catch (InterruptedException e)
                {
                    logger.error("IdmDomainStateRetriever Thread is interrupted!", e);
                }
                catch (Throwable t)
                {
                   logger.error("IdmDomainStateRetriever threw an error", t);
                   throw t;
                }
            }
        }
    }

    private static final long time_interval_on_succeed = 60000*60*6; // 6 hours
    private static final long time_interval_on_fail = 60000*10; // 10 minutes
    private static DomainTrustInfo[] _trustsInfo;
    private static DcInfoCache _dcInfoCache;
    private static ActiveDirectoryJoinInfo _adJoinInfo;
    private static DomainControllerInfo _forestDcInfo;
    private static ReentrantReadWriteLock _domainInfoLock;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(IdmDomainState.class);

    protected IdmDomainState()
    {
        _trustsInfo = new DomainTrustInfo[0];
        _dcInfoCache = new DcInfoCache();
        _adJoinInfo = null;
        _domainInfoLock = new ReentrantReadWriteLock();

        Thread domainInfoRetriever = new IdmDomainStateRetriever();
        domainInfoRetriever.start();
    }

    public static IdmDomainState getInstance()
    {
        if (instance == null)
        {
            instance = new IdmDomainState();
        }

        return instance;
    }

    public DomainTrustInfo[] getDomainTrustInfo()
    {
        final Lock readLock = _domainInfoLock.readLock();

        readLock.lock();

        try
        {
            return _trustsInfo;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public DcInfoCache getDcInfoCache()
    {
        final Lock readLock = _domainInfoLock.readLock();

        readLock.lock();

        try
        {
            return _dcInfoCache;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public ActiveDirectoryJoinInfo getDomainJoinInfo()
    {
        final Lock readLock = _domainInfoLock.readLock();

        readLock.lock();

        try
        {
            return _adJoinInfo;
        }
        finally
        {
            readLock.unlock();
        }
    }

    public DomainControllerInfo getForestDcInfo()
    {
        final Lock readLock = _domainInfoLock.readLock();

        readLock.lock();

        try
        {
            return _forestDcInfo;
        }
        finally
        {
            readLock.unlock();
        }
    }

    private boolean retrieveDomainInfo()
    {
        boolean bSucceed = true;
        boolean bJoined = false;
        final Lock writeLock = _domainInfoLock.writeLock();
        ActiveDirectoryJoinInfo joinInfo = null;
        DomainTrustInfo[] trusts = new DomainTrustInfo[0];
        DcInfoCache dcInfoCache = new DcInfoCache();
        DomainControllerInfo forestDcInfo = null;

        try
        {
            try
            {
                IDomainAdapter adapter =
                        DomainAdapterFactory.getInstance().getDomainAdapter();
                DomainControllerInfo dcInfo = adapter.getDomainJoinInfo();

                joinInfo = VmafClientUtil.queryActiveDirectory();
                bJoined = true;

                forestDcInfo = adapter.getDcInfo(dcInfo.domainDnsForestName);
            }
            catch(HostNotJoinedException e)
            {
                logger.debug("IdmDomainStateRetriever - Current host is not joined to active directory");
                joinInfo = null;
            }
            catch(Exception e)
            {
                logger.error("IdmDomainStateRetriever - failed to retrieve current domain join state");
                bSucceed = false;
            }

            if (bJoined)
            {
                trusts = IdmDomainState.this.getTrustedInfo(joinInfo.getName());
                if (trusts == null || trusts.length == 0)
                {
                    bSucceed = false;
                }

                if (trusts != null && trusts.length > 0)
                {
                    for (DomainTrustInfo trust : trusts)
                    {
                        if (trust != null && trust.dcInfo != null)
                        {
                            dcInfoCache.addDcInfo(trust.dcInfo);
                        }
                    }
                }
            }

            writeLock.lock();
            try
            {
                _adJoinInfo = joinInfo;
                _trustsInfo = trusts;
                _dcInfoCache = dcInfoCache;
                _forestDcInfo = forestDcInfo;
            }
            finally
            {
                writeLock.unlock();
            }
        }
        catch (Exception e1)
        {
            logger.error(String.format("IdmDomainStateRetriever refresh domain information failed : %s",
                    e1.getMessage()), e1);
            if (logger.isDebugEnabled())
            {
                e1.printStackTrace();
            }
            bSucceed  = false;
        }

        return bSucceed;
    }

    private DomainTrustInfo[] getTrustedInfo(String domainName)
    {
        // retrieve trust information
        DomainTrustInfo[] trustsInfo = new DomainTrustInfo[0];
        IDomainAdapter domainAdapter =
                   DomainAdapterFactory.getInstance().getDomainAdapter();
        try
        {
            trustsInfo = domainAdapter.getDomainTrusts(domainName);
        }
        // If we cannot retrieve trust information, log and proceed
        catch(DomainManagerNativeException|DomainManagerException e)
        {
            logger.info(String.format("Enumerate trusts failed %s", e.getMessage()));
        }

        return trustsInfo;
    }
}
