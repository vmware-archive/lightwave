/*
 * Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.identity.interop.domainmanager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.platform.win32.Netapi32Util;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.win32.W32APIOptions;
import com.vmware.identity.interop.PlatformUtils;

enum WinNetSetupJoinStatus {
    NET_SETUP_UNKNOWN_STATUS(0),
    NET_SETUP_UNJOINED(1),
    NET_SETUP_WORKGROUP_NAME(2),
    NET_SETUP_DOMAIN_NAME(3);

    private int _status;

    private WinNetSetupJoinStatus(int status)
    {
        _status = status;
    }

    public int getStatus()
    {
        return _status;
    }
}

/**
 * Created by IntelliJ IDEA.
 * User: wfu
 * Date: 3/13/13
 * Time: 1:47 PM
 * To change this template use File | Settings | File Templates.
 */
public class WinDomainAdapter implements IDomainAdapter {

    static int WIN_GUID_DATA4_SIZE = 8;

    static final int ERROR_ACCESS_DENIED        = 0x00000005;
    static final int ERROR_INVALID_DOMAINNAME   = 0x000004BC;
    static final int ERROR_INVALID_FLAGS        = 0x000003EC;
    static final int ERROR_NOT_ENOUGH_MEMORY    = 0x00000008;
    static final int ERROR_NO_SUCH_DOMAIN       = 0x0000054B;
    static final int ERROR_NO_LOGON_SERVERS     = 0x0000051F;
    static final int ERROR_NO_TRUST_LFA_SECRET  = 0x000006FA;
    static final int ERROR_NO_TRUST_SAM_ACCOUNT = 0x000006FB;
    static final int ERROR_NOT_SUPPORTED        = 0x00000032;

    private static WinDomainAdapter ourInstance = new WinDomainAdapter();

    private interface WinNetApi32 extends Library
    {
        WinNetApi32 INSTANCE =
             (WinNetApi32) Native.loadLibrary(
                   "Netapi32.dll",
                   WinNetApi32.class, W32APIOptions.UNICODE_OPTIONS);

        //DWORD
        // DsEnumerateDomainTrusts(
        //    IN_OPT LPTSTR ServerName,
        //    IN ULONG Flags,
        //    OUT PDS_DOMAIN_TRUSTS *Domains,
        //    OUT PULONG DomainCount
        //    );
        int
        DsEnumerateDomainTrusts(
            String ServerName,
            int Flags,
            PointerByReference ppDomains,
            IntByReference pDomainCount
        );

        //DWORD
        // DsGetDcName(
        //     IN LPCTSTR ComputerName,
        //     IN LPCTSTR DomainName,
        //     GUID* DomainCuid,
        //     LPCTSTR SiteName,
        //     ULONG Flags,
        //     PDOMAIN_CONTROLLER_INFO *DomainControllerInfo
        //     );

        int DsGetDcName(
            String computerName,
            String domainName,
            Pointer domainGuid,
            String siteName,
            int flags,
            PointerByReference ppDCInfo
            );

        // NET_API_STATUS
        // NetApiBufferFree(
        //     LPVOID buffer
        //     )
        int NetApiBufferFree(Pointer pBuf);
    }

    private static final Log logger = LogFactory.getLog(WinDomainAdapter.class);

    public static WinDomainAdapter getInstance() {
        return ourInstance;
    }

    public WinDomainAdapter()
    {
        if(Platform.isWindows() == false)
        {
            throw new RuntimeException(
                          "This class is only supported on Windows platform.");
        }
    }

    // IDomainAdapter

    @Override
    public
    DomainTrustInfo[]
    getDomainTrusts(String domainName)
    {
        Map<String, WinDomainTrustInfoNative> complete_trusts = new HashMap<String, WinDomainTrustInfoNative>();
        ArrayList<WinDomainTrustInfoNative> trusts = new ArrayList<WinDomainTrustInfoNative>();
        ArrayList<WinDomainTrustInfoNative> extra_trusts_to_enum = new ArrayList<WinDomainTrustInfoNative>();

        trusts = getDomainTrustsInternal(domainName);

        if (trusts.size() > 0)
        {
            for (WinDomainTrustInfoNative trust : trusts)
            {
                if (isEmptyOrNullTrust(trust)) continue;

                complete_trusts.put(trust.dnsDomainName.toLowerCase(), trust);

                boolean isInforest = trust.isInForest();
                boolean isExternal = trust.isExternal();
                boolean isInBound = trust.isInBound();
                boolean isOutBound = trust.isOutBound();
                // only add 2-way trust as extra trust candidates to discover more
                if (!isInforest && !isExternal && isInBound && isOutBound)
                {
                    extra_trusts_to_enum.add(trust);
                }
            }

            if (extra_trusts_to_enum.size() > 0)
            {
                for (WinDomainTrustInfoNative extra_trust_to_enum : extra_trusts_to_enum)
                {
                    if (isEmptyOrNullTrust(extra_trust_to_enum)) continue;

                    ArrayList<WinDomainTrustInfoNative> extra_trusts = new ArrayList<WinDomainTrustInfoNative>();
                    try
                    {
                        extra_trusts = dsEnumerateDomainTrusts(extra_trust_to_enum.dnsDomainName,
                                                               WinDomainTrustInfoNative.AllFlags());
                    }
                    catch(DomainManagerException e)
                    {
                        logger.info(String.format("Failed to enumerate trust for %s ",
                                extra_trust_to_enum.dnsDomainName) , e);
                    }

                    if (extra_trusts.size() > 0)
                    {
                        for (WinDomainTrustInfoNative extraTrust : extra_trusts)
                        {
                            if (isEmptyOrNullTrust(extraTrust)) continue;

                            if (extraTrust.isInForest() &&
                                (complete_trusts.isEmpty() || !complete_trusts.containsKey(extraTrust.dnsDomainName.toLowerCase()))
                               )
                            {
                                complete_trusts.put(extraTrust.dnsDomainName.toLowerCase(), extraTrust);
                            }
                        }
                    }
                }
            }
        }

        return processTrust(new ArrayList<WinDomainTrustInfoNative>(complete_trusts.values()));
    }

    @Override
    public DomainControllerInfo getDomainJoinInfo()
    {
        try
        {
            int joinStatus = Netapi32Util.getJoinStatus();
            if (WinNetSetupJoinStatus.NET_SETUP_DOMAIN_NAME.getStatus() == joinStatus)
            {
                return getDcInfo(null);
            }
            else
            {
                throw new HostNotJoinedException("Local host is not joined.");
            }
        }
        catch(HostNotJoinedException e)
        {
            throw e;
        }
        catch(Win32Exception e)
        {
            throw new DomainManagerException("Could not get domain join info" + e.getMessage());
        }
    }

    @Override
    public DomainControllerInfo getDcInfo(String domainName)
    {
        return getDcInfo(domainName, false);
    }

    @Override
    public DomainControllerInfo getDcInfoWithRediscover(String domainName)
    {
        return getDcInfo(domainName, true);
    }

    private DomainControllerInfo getDcInfo(String domainName, boolean bForceRediscovery)
    {
        DomainControllerInfo domainInfo = null;

        int dwError = 0;
        WinDcInfoNative dcInfo = null;
        PointerByReference ppDcInfo = new PointerByReference(Pointer.NULL);
        Pointer pDcInfo = Pointer.NULL;

        WinDcInfoNative dcInfo_flat = null;
        PointerByReference ppDcInfo_flat = new PointerByReference(Pointer.NULL);
        Pointer pDcInfo_flat = Pointer.NULL;

        try
        {
            dwError = WinNetApi32.INSTANCE.DsGetDcName(null,
                                                       domainName,
                                                       null,
                                                       null,
                                                       bForceRediscovery
                                                       ? PlatformUtils.getFlagsForGetDcInfoWithRediscovery()
                                                       : PlatformUtils.getFlagsForGetDcInfo(),
                                                       ppDcInfo);
            logAndThrow(String.format("Failed to get domain controller information for %s ", domainName), dwError);

            pDcInfo = ppDcInfo.getValue();
            if (pDcInfo == Pointer.NULL)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            try
            {
                dcInfo = new WinDcInfoNative(pDcInfo);
            }
            catch(Exception e)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            dwError = WinNetApi32.INSTANCE.DsGetDcName(null,
                                                       domainName,
                                                       null,
                                                       null,
                                                       PlatformUtils.getFlagsForGetFlatDcInfo(),
                                                       ppDcInfo_flat);
            logAndThrow(String.format("Failed to get domain controller information for %s ", domainName), dwError);

            pDcInfo_flat = ppDcInfo_flat.getValue();
            if (pDcInfo_flat == Pointer.NULL)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            try
            {
                dcInfo_flat = new WinDcInfoNative(pDcInfo_flat);
            }
            catch(Exception e)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            if (dcInfo != null &&  dcInfo_flat != null)
            {
                domainInfo = new DomainControllerInfo(dcInfo.domainName,
                                                      dcInfo_flat.domainName,
                                                      dcInfo.domainControllerAddress,
                                                      dcInfo.domainControllerName,
                                                      dcInfo.dnsForestName
                                                      );
            }

            return domainInfo;

        }
        catch(Win32Exception e)
        {
            String errMsg = String.format("Failed to get domain controller information for %s",
                                           (domainName==null || domainName.length()==0) ? "local host domain" : domainName);
            logger.error(errMsg);
            throw new DomainManagerException(errMsg + e.getMessage());
        }
        finally
        {
            if (pDcInfo != Pointer.NULL)
            {
                WinNetApi32.INSTANCE.NetApiBufferFree(pDcInfo);
            }
            if (pDcInfo_flat != Pointer.NULL)
            {
                WinNetApi32.INSTANCE.NetApiBufferFree(pDcInfo_flat);
            }
        }
    }

    private DomainTrustInfo[] processTrust(ArrayList<WinDomainTrustInfoNative> trusts)
    {
        ArrayList<DomainTrustInfo> platform_trust = new ArrayList<DomainTrustInfo>();
        if (trusts != null && trusts.size() > 0)
        {
            for (int i = 0; i < trusts.size(); i++)
            {
                WinDomainTrustInfoNative trustNative = trusts.get(i);
                if (isEmptyOrNullTrust(trustNative)) continue;

                DomainControllerInfo dcInfo = null;
                try
                {
                    dcInfo = this.getDcInfo(trustNative.dnsDomainName);
                }
                catch(Exception e)
                {
                    logger.warn(String.format("Failed to process trust with domain name %s - ", trustNative.dnsDomainName)
                            + e.getMessage());
                }
                if (dcInfo != null)
                {
                    platform_trust.add(new DomainTrustInfo(trustNative, dcInfo));
                }
            }
        }

        return ((DomainTrustInfo[])platform_trust.toArray(new DomainTrustInfo[platform_trust.size()]));
    }

    private
    ArrayList<WinDomainTrustInfoNative>
    dsEnumerateDomainTrusts(String domainName, int trustFlags)
    {
        PointerByReference ppTrustsInfo = new PointerByReference(Pointer.NULL);
        Pointer pTrustsInfo = Pointer.NULL;
        WinDomainTrustInfoNative[] trustsArray = null;
        ArrayList<WinDomainTrustInfoNative> trusts = new ArrayList<WinDomainTrustInfoNative>();
        int numTrusts = 0;
        IntByReference pNumTrusts = new IntByReference();

        DomainControllerInfo dcInfo = null;
        try
        {
            dcInfo = this.getDcInfo(domainName);
        }
        catch(DomainManagerException ex)
        {
           logger.error(String.format("Before enumerating trust for domain [%s], getDcInfo failed. ", domainName));
        }

        try
        {
            String serverName = dcInfo == null ? domainName : dcInfo.domainFQDN;
            int dwError = WinNetApi32.INSTANCE.DsEnumerateDomainTrusts(serverName,
                                                                       trustFlags,
                                                                       ppTrustsInfo,
                                                                       pNumTrusts);
            logAndThrow(String.format("Failed to enumerate domain trusts for %s ", serverName), dwError);

            numTrusts = pNumTrusts.getValue();
            pTrustsInfo = ppTrustsInfo.getValue();

            if ( ( numTrusts > 0 ) && ( pTrustsInfo != null ) && ( pTrustsInfo != Pointer.NULL ) )
            {
                WinDomainTrustInfoNative trustsInfo = new WinDomainTrustInfoNative(pTrustsInfo);
                trustsArray = (WinDomainTrustInfoNative[])trustsInfo.toArray( numTrusts );
            }

            for (int i = 0; i < numTrusts; i++)
            {
                if (isEmptyOrNullTrust(trustsArray[i])) continue;
                trusts.add(trustsArray[i]);
            }
        }
        finally
        {
            if (pTrustsInfo != Pointer.NULL)
            {
                WinNetApi32.INSTANCE.NetApiBufferFree(pTrustsInfo);
            }
        }

        return trusts;
    }

    // Retrieve trusts discovered by the domain that is currently joined
    private
    ArrayList<WinDomainTrustInfoNative>
    getDomainTrustsInternal(String domainName)
    {
        Map<String, WinDomainTrustInfoNative> trusts_to_ret = new HashMap<String, WinDomainTrustInfoNative>();
        ArrayList<WinDomainTrustInfoNative> trusts = new ArrayList<WinDomainTrustInfoNative>();
        ArrayList<WinDomainTrustInfoNative> trustRoots = new ArrayList<WinDomainTrustInfoNative>();

        // Initial trust enumerate against the joined domain
        trusts = dsEnumerateDomainTrusts(domainName, WinDomainTrustInfoNative.AllFlags());

        if (trusts.size() > 0)
        {
            for (WinDomainTrustInfoNative trust : trusts)
            {
                if (isEmptyOrNullTrust(trust)) continue;

                // Add the currently discovered trust
                trusts_to_ret.put(trust.dnsDomainName.toLowerCase(), trust);
                if (trust.isRoot())
                {
                    trustRoots.add(trust);
                }
            }
        }

        // Process tree root domains
        if (trustRoots.size() > 0)
        {
            for (WinDomainTrustInfoNative trustRoot : trustRoots)
            {
                if (isEmptyOrNullTrust(trustRoot)) continue;

                ArrayList<WinDomainTrustInfoNative> trustsOutSide = new ArrayList<WinDomainTrustInfoNative>();
                try
                {
                    trustsOutSide = dsEnumerateDomainTrusts(trustRoot.dnsDomainName,
                                                            WinDomainTrustInfoNative.AllFlags());
                }
                catch(DomainManagerException e)
                {
                    logger.info(String.format("Failed to enumerate trust for %s ",
                                                     trustRoot.dnsDomainName) + e.getMessage());
                }

                if (trustsOutSide.size() > 0)
                {
                    for (WinDomainTrustInfoNative trustOutSide : trustsOutSide)
                    {
                        if (isEmptyOrNullTrust(trustOutSide)) continue;

                        boolean isInBound = trustOutSide.isInBound();
                        boolean isOutBound = trustOutSide.isOutBound();
                        boolean isExternal = trustOutSide.isExternal();

                        // discover trusts outside of the current forest (do not discover if trust is external
                        // external trust is non-transitive
                        if (!isExternal && (isInBound || isOutBound) &&
                            (trusts_to_ret.isEmpty() || !trusts_to_ret.containsKey(trustOutSide.dnsDomainName.toLowerCase()))
                           )
                        {
                            trusts_to_ret.put(trustOutSide.dnsDomainName.toLowerCase(), trustOutSide);
                        }
                    }
                }
            }
        }

        return new ArrayList<WinDomainTrustInfoNative>(trusts_to_ret.values());
    }

    private boolean isEmptyOrNullTrust(WinDomainTrustInfoNative trust)
    {
        return (trust == null || trust.dnsDomainName == null || trust.dnsDomainName.isEmpty());
    }

    private void logAndThrow(String opInfo, int dwError)
    {
        String errMsg = null;
        if (dwError != 0)
        {
            switch (dwError)
            {
                case ERROR_INVALID_DOMAINNAME:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_INVALID_DOMAINNAME)", dwError);
                    break;
                case ERROR_INVALID_FLAGS:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_INVALID_FLAGS)", dwError);
                    break;
                case ERROR_NOT_ENOUGH_MEMORY:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NOT_ENOUGH_MEMORY)", dwError);
                    break;
                case ERROR_NO_SUCH_DOMAIN:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NO_SUCH_DOMAIN)", dwError);
                    break;
                case ERROR_NO_LOGON_SERVERS:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NO_LOGON_SERVERS)", dwError);
                    break;
                case ERROR_NO_TRUST_LFA_SECRET:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NO_TRUST_LFA_SECRET)", dwError);
                    break;
                case ERROR_NO_TRUST_SAM_ACCOUNT:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NO_TRUST_SAM_ACCOUNT)", dwError);
                    break;
                case ERROR_NOT_SUPPORTED:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_NOT_SUPPORTED)", dwError);
                    break;
                case ERROR_ACCESS_DENIED:
                    errMsg = opInfo + String.format("(dwError - %d - ERROR_ACCESS_DENIED)", dwError);

                default:
                    errMsg = opInfo + String.format("(dwError - %d)", dwError);
            }
            logger.debug(errMsg);
            throw new DomainManagerException(errMsg);
        }
    }
}
