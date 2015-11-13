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

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.identity.interop.NativeAdapter;
import com.vmware.identity.interop.PlatformUtils;

/**
 * Created by IntelliJ IDEA.
 * Date: 1/6/12
 * Time: 12:05 PM
 * To change this template use File | Settings | File Templates.
 */
public class LinuxDomainAdapter extends NativeAdapter implements IDomainAdapter
{
    static int LWNET_GUID_SIZE = 16;

    private interface LsaClientLibrary extends Library
    {
        LsaClientLibrary INSTANCE =
             (LsaClientLibrary) Native.loadLibrary(
                   "lsaclient",
                   LsaClientLibrary.class);

        static String LSA_PROVIDER_TAG_AD = "lsa-activedirectory-provider";

        //LW_DWORD
        //LsaOpenServer(
        //        LW_PHANDLE phConnection
        //);
        int LsaOpenServer(PointerByReference phConnection);

        //DWORD
        //LsaAdGetJoinedDomains(
        //    IN HANDLE hLsaConnection,
        //    OUT PDWORD pdwNumDomainsFound,
        //    OUT PSTR** pppszJoinedDomains
        //    );
        int LsaAdGetJoinedDomains(
            Pointer hLsaConnection,
            PointerByReference pdwNumDomainsFound,
            PointerByReference pppszJoinedDomains
            );

        //LW_DWORD
        //LsaGetStatus2(
        //    LW_HANDLE hLsaConnection,
        //    LW_IN LW_PCSTR pszTargetProvider,
        //    PLSASTATUS* ppLsaStatus
        //    );
        int
        LsaGetStatus2(
            Pointer hLsaConnection,
            String pszTargetProvider,
            PointerByReference ppLsaStatus
            );

        //LW_VOID
        //LsaFreeStatus(
        //    PLSASTATUS pLsaStatus
        //    );
        void
        LsaFreeStatus(Pointer pLsaStatus);

        //LW_DWORD
        //LsaCloseServer(
        //        LW_HANDLE hConnection
        //);
        int LsaCloseServer(
            Pointer hConnection
            );
    }

    private interface LwNetClientLibrary extends Library
    {
        LwNetClientLibrary INSTANCE =
                (LwNetClientLibrary) Native.loadLibrary(
                      "lwnetclientapi",
                      LwNetClientLibrary.class);

        //LWNET_API
        //DWORD
        //LWNetGetDCName(
        //    PCSTR pszServerFQDN,
        //    PCSTR pszDomainFQDN,
        //    PCSTR pszSiteName,
        //    DWORD dwFlags,
        //    PLWNET_DC_INFO* ppDCInfo
        //    );
        int
        LWNetGetDCName(
            String pszServerFQDN,
            String pszDomainFQDN,
            String pszSiteName,
            int dwGetDcFlags,
            PointerByReference ppDCInfo
            );

        //LW_VOID
        //LWNetFreeDCInfo(
        //    LW_IN LW_OUT PLWNET_DC_INFO pDCInfo
        //    );
        void
        LWNetFreeDCInfo(Pointer pDCInfo);
    }

    private interface LwAdvapiLibrary extends Library
    {
        LwAdvapiLibrary INSTANCE =
             (LwAdvapiLibrary) Native.loadLibrary(
                   "lwadvapi_nothr",
                   LwAdvapiLibrary.class);

        String LwWin32ExtErrorToName(int dwError);

        //void
        //LwFreeStringArray(
        //    PSTR * ppStringArray,
        //    DWORD dwCount
        //    );
        void LwFreeStringArray(Pointer ppStringArray, int dwCount);
    }

    private static LinuxDomainAdapter ourInstance = new LinuxDomainAdapter();

    public static LinuxDomainAdapter getInstance() {
        return ourInstance;
    }

    public LinuxDomainAdapter()
    {
        if(Platform.isLinux() == false)
        {
            throw new RuntimeException(
                            "This class is only supported on Linux platform.");
        }
    }

    // IDomainAdapter

    @Override
    public
    DomainTrustInfo[]
    getDomainTrusts(String domainName)
    {
        PlatformUtils.validateNotNull(domainName, "domainName");

        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        PointerByReference ppLsaStatus = new PointerByReference(Pointer.NULL);
        Pointer pLsaStatus = Pointer.NULL;
        DomainTrustInfo[] platform_trust = new DomainTrustInfo[0];
        int dwError = 0;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new DomainManagerNativeException(dwError);
            }

            dwError = LsaClientLibrary.INSTANCE.LsaGetStatus2(pLsaConnection.getValue(),
                                                              LsaClientLibrary.LSA_PROVIDER_TAG_AD,
                                                              ppLsaStatus);
            if (dwError != 0)
            {
                throw new DomainManagerNativeException(dwError);
            }

            pLsaStatus = ppLsaStatus.getValue();
            if (pLsaStatus == Pointer.NULL)
            {
                throw new DomainManagerException(String.format("GetDomainTrusts for domain %s failed", domainName));
            }
            LsaStatusNative lsaStatus = new LsaStatusNative(pLsaStatus);

            if (lsaStatus.pAuthProviderStatusList == Pointer.NULL)
            {
                throw new DomainManagerException(String.format("GetDomainTrusts for domain %s failed", domainName));
            }
            LsaAuthProviderStatusNative lsaAuthProviderStatus = new LsaAuthProviderStatusNative(lsaStatus.pAuthProviderStatusList);
            int trustNum = lsaAuthProviderStatus.dwNumTrustedDomains;
            Pointer trustDomainStartPtr = lsaAuthProviderStatus.pTrustedDomainInfoArray;
            LsaTrustedDomainInfoNative trust = new LsaTrustedDomainInfoNative();
            platform_trust = new DomainTrustInfo[trustNum];

            for (int i = 0; i < trustNum; i++)
            {
                Pointer pTrust = new Pointer(Pointer.nativeValue(trustDomainStartPtr) + trust.size()*i);
                LsaTrustedDomainInfoNative trustInfo = new LsaTrustedDomainInfoNative(pTrust);
                platform_trust[i] = new DomainTrustInfo(new LinuxDomainTrust(trustInfo));
            }
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                  LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                  pLsaConnection.setValue( Pointer.NULL );
            }
            if (pLsaStatus != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaFreeStatus(pLsaStatus);
            }
        }

        return platform_trust;
    }

    @Override
    public DomainControllerInfo getDomainJoinInfo()
    {
        PointerByReference pLsaConnection = new PointerByReference(Pointer.NULL);
        PointerByReference pdwNumDomainsFound = new PointerByReference(Pointer.NULL);
        PointerByReference pppszJoinedDomains = new PointerByReference(Pointer.NULL);
        Pointer ppJoinedDomains = Pointer.NULL;
        DomainControllerInfo joined_domainInfo = null;
        int dwNumDomainsFound = 0;
        int dwError = 0;

        try
        {
            dwError = LsaClientLibrary.INSTANCE.LsaOpenServer(pLsaConnection);
            if (dwError != 0)
            {
                throw new DomainManagerNativeException(dwError);
            }

            /* lwis supports multi-tenants, however, we do not use that in SSO
             * there will be only 1 domain joined to
             */
            dwError = LsaClientLibrary.INSTANCE.LsaAdGetJoinedDomains(pLsaConnection.getValue(),
                                                                      pdwNumDomainsFound,
                                                                      pppszJoinedDomains);
            if (dwError != 0)
            {
                throw new DomainManagerNativeException(dwError);
            }

            ppJoinedDomains = pppszJoinedDomains.getValue();
            if (pdwNumDomainsFound.getPointer() == Pointer.NULL || ppJoinedDomains == Pointer.NULL)
            {
                throw new DomainManagerException("GetDomainJoinInfo failed");
            }

            dwNumDomainsFound = pdwNumDomainsFound.getPointer().getInt(0);
            if (dwNumDomainsFound == 0)
            {
                throw new HostNotJoinedException("Local host is not joined.");
            }

            String ppszJoinedDomains[] = ppJoinedDomains.getStringArray(0, dwNumDomainsFound);
            if (ppszJoinedDomains != null && ppszJoinedDomains.length > 0)
            {
                joined_domainInfo = getDcInfo(ppszJoinedDomains[0]);
            }
        }
        finally
        {
            if (pLsaConnection.getValue() != Pointer.NULL)
            {
                LsaClientLibrary.INSTANCE.LsaCloseServer( pLsaConnection.getValue() );
                pLsaConnection.setValue( Pointer.NULL );
            }
            if (ppJoinedDomains != Pointer.NULL)
            {
                LwAdvapiLibrary.INSTANCE.LwFreeStringArray(ppJoinedDomains, dwNumDomainsFound);
            }
        }

        return joined_domainInfo;
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
        PlatformUtils.validateNotNull(domainName, "domainName");
        int dwError = 0;
        LwNetDcInfoNative dcInfo = null;
        DomainControllerInfo domainInfo = null;

        PointerByReference ppDcInfo = new PointerByReference(Pointer.NULL);
        Pointer pDcInfo = Pointer.NULL;
        try
        {
            dwError = LwNetClientLibrary.INSTANCE.LWNetGetDCName(null,
                                                                 domainName,
                                                                 null,
                                                                 bForceRediscovery
                                                                 ? PlatformUtils.getFlagsForGetDcInfoWithRediscovery()
                                                                 : PlatformUtils.getFlagsForGetDcInfo(),
                                                                 ppDcInfo);
            if (dwError != 0)
            {
                throw new DomainManagerNativeException(dwError);
            }

            pDcInfo = ppDcInfo.getValue();
            if (pDcInfo == Pointer.NULL)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            try
            {
                dcInfo = new LwNetDcInfoNative(pDcInfo);
            }
            catch(Exception e)
            {
                throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
            }

            if (dcInfo != null)
            {
                domainInfo = new DomainControllerInfo(dcInfo.pszFullyQualifiedDomainName,
                                                      dcInfo.pszNetBIOSDomainName,
                                                      dcInfo.pszDomainControllerAddress,
                                                      dcInfo.pszDomainControllerName,
                                                      dcInfo.pszDnsForestName);
            }
        }
        catch(DomainManagerException ex)
        {
            // In case of any failure that we could not get dc info, try to make use of trust information
            DomainControllerInfo joinedDcInfo = getDomainJoinInfo();
            DomainTrustInfo[] trusts = getDomainTrusts(joinedDcInfo.domainName);
            boolean bFound = false;

               if (trusts != null && trusts.length > 0)
               {
                   for (DomainTrustInfo trust : trusts)
                   {
                       if (trust.dcInfo.domainName.equalsIgnoreCase(domainName) ||
                           trust.dcInfo.domainNetBiosName.equalsIgnoreCase(domainName))
                       {
                           domainInfo = new DomainControllerInfo(trust.dcInfo.domainName,
                                                                 trust.dcInfo.domainNetBiosName,
                                                                 trust.dcInfo.domainIpAddress,
                                                                 trust.dcInfo.domainFQDN,
                                                                 trust.dcInfo.domainDnsForestName);
                           bFound = true;
                           break;
                       }
                   }
               }
               if (!bFound)
               {
                   throw new DomainManagerException(String.format("Failed to get domain controller information for %s", domainName));
               }
        }
        finally
        {
            if (pDcInfo != Pointer.NULL)
            {
                LwNetClientLibrary.INSTANCE.LWNetFreeDCInfo(pDcInfo);
            }
        }

        return domainInfo;
    }
}

