/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

package com.vmware.af.interop;

import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.WString;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;
import com.vmware.af.DomainInfo;
import com.vmware.af.PasswordCredential;
import com.vmware.af.VmAfClientNativeException;

public class VmAfClientAdapter extends NativeAdapter
{
    public interface VmAfClientLibrary extends Library
    {
        VmAfClientLibrary INSTANCE =
                (VmAfClientLibrary) Native.loadLibrary(
                                                "vmafdclient",
                                                VmAfClientLibrary.class);

        int
        VmAfdGetStatusA(
            String         pszServerName,  /* IN     OPTIONAL */
            IntByReference pStatus         /* IN OUT          */
            );

        int
        VmAfdGetStatusW(
            WString        pwszServerName, /* IN     */
            IntByReference pStatus         /* IN OUT */
            );

        int
        VmAfdGetDomainNameA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference ppszDomain        /*    OUT          */
            );

        int
        VmAfdGetDomainNameW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszDomain      /*    OUT          */
            );

        int
        VmAfdSetDomainNameA(
            String pszServerName, /* IN     OPTIONAL */
            String pszDomain      /* IN              */
            );

        int
        VmAfdSetDomainNameW(
            WString pwszServerName, /* IN     OPTIONAL */
            WString pwszDomain      /* IN              */
            );

        int
        VmAfdGetDomainStateA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference pDomainState      /*    OUT          */
            );

        int
        VmAfdGetDomainStateW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference pDomainState     /*    OUT          */
            );

        int
        VmAfdGetLDUA(
            String             pszServerName, /* IN     OPTIONAL */
            PointerByReference ppszLDU        /*    OUT          */
            );

        int
        VmAfdGetLDUW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszLDU         /*    OUT          */
            );

        int
        VmAfdSetLDUA(
            String pszServerName, /* IN     OPTIONAL */
            String pszLDU         /*    OUT          */
            );

        int
        VmAfdSetLDUW(
            WString pwszServerName, /* IN     OPTIONAL */
            WString pwszLDU         /* IN              */
            );

        int
        VmAfdGetRHTTPProxyPortA(
            String pszServerName, /* IN     OPTIONAL */
            IntByReference pdwPort
            );

        int
        VmAfdSetRHTTPProxyPortA(
            String pszServerName, /* IN     OPTIONAL */
            int port              /* IN              */
            );

        int
        VmAfdSetRHTTPProxyPortW(
            WString pwszServerName, /* IN     OPTIONAL */
            int port                /* IN              */
            );

        int
        VmAfdSetDCPortA(
            String pszServerName, /* IN     OPTIONAL */
            int port              /* IN              */
            );

        int
        VmAfdSetDCPortW(
            WString pwszServerName, /* IN     OPTIONAL */
            int port                /* IN              */
            );

        int
        VmAfdGetCMLocationA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference ppszCMLocation    /*    OUT          */
            );

        int
        VmAfdGetCMLocationW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszCMLocation  /*    OUT          */
            );

        int
        VmAfdGetLSLocationA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference ppszLSLocation    /*    OUT          */
            );

        int
        VmAfdGetLSLocationW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszLSLocation  /*    OUT          */
            );

        int
        VmAfdSetLSLocationW(
            WString pwszServerName, /* IN     OPTIONAL */
            WString pwszLSLocation  /* IN              */
            );

        int
        VmAfdGetDCNameA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference ppszDCName        /*    OUT          */
            );

        int
        VmAfdGetDCNameW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszDCName      /*    OUT          */
            );

        int
        VmAfdSetDCNameA(
            String pszServerName,    /* IN      OPTIONAL */
            String pszDCName         /* IN               */
            );

        int
        VmAfdSetDCNameW(
            WString pwszServerName,  /* IN      OPTIONAL */
            WString pwszDCName       /* IN               */
            );

        int
        VmAfdGetPNIDForUrlA(
            String             pszServerName,
            PointerByReference ppszPNID
            );

        int
        VmAfdGetPNIDA(
            String             pszServerName,
            PointerByReference ppszPNID
            );

        int
        VmAfdGetPNIDW(
            WString            pwszServerName,
            PointerByReference ppwszPNID
            );

        int
        VmAfdSetPNIDA(
            String pszServerName,
            String pszPNID
            );

        int
        VmAfdSetPNIDW(
            WString pwszServerName,
            WString pwszPNID
            );

        int
        VmAfdGetMachineAccountInfoA(
            String             pszServerName,    /* IN     OPTIONAL */
            PointerByReference ppszAccount,      /*    OUT          */
            PointerByReference ppszPassword      /*    OUT          */
            );

        int
        VmAfdGetMachineAccountInfoW(
            WString            pwszServerName,  /* IN     OPTIONAL */
            PointerByReference ppwszAccount,    /*    OUT          */
            PointerByReference ppwszPassword    /*    OUT          */
            );

        int
        VmAfdPromoteVmDirA(
            String pszServerName,    /* IN              */
            String pszDomainName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword,      /* IN              */
            String pszSiteName,      /* IN              */
            String pszPartnerName    /* IN              */
            );

        int
        VmAfdPromoteVmDirW(
            WString pwszServerName,  /* IN              */
            WString pwszDomainName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword,    /* IN              */
            WString pszSiteName,     /* IN              */
            WString pszPartnerName   /* IN              */
            );

        int
        VmAfdDemoteVmDirA(
            String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword       /* IN              */
            );

        int
        VmAfdDemoteVmDirW(
            WString pwszServerName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword     /* IN              */
            );

        int
        VmAfdJoinVmDirA(
            String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword,      /* IN              */
            String pszMachineName,   /* IN             */
            String pszDomainName,    /* IN             */
            String pszOrgUnit        /* IN             */
            );

        int
        VmAfdJoinVmDirW(
            WString pwszServerName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword,    /* IN              */
            WString pszMachineName,  /* IN             */
            WString pszDomainName,   /* IN             */
            WString pszOrgUnit       /* IN             */
            );

        int
        VmAfdLeaveVmDirA(
            String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword       /* IN              */
            );

        int
        VmAfdLeaveVmDirW(
            WString pwszServerName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword     /* IN              */
            );

        int
        VmAfdJoinADA(
            String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword,      /* IN              */
            String pszDomainName,    /* IN              */
            String pszOrgUnit        /* IN              */
            );

        int
        VmAfdJoinADW(
            WString pwszServerName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword,    /* IN              */
            WString pszDomainName,   /* IN              */
            WString pszOrgUnit       /* IN              */
            );

        int
        VmAfdLeaveADA(
            String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword       /* IN              */
            );

        int
        VmAfdLeaveADW(
            WString pwszServerName,  /* IN              */
            WString pwszUserName,    /* IN              */
            WString pwszPassword     /* IN              */
            );

        int
        VmAfdCreateComputerAccountA(
            String pszUserName,              /* IN              */
            String pszPassword,              /* IN              */
            String pszMachineName,           /* IN              */
            String pszOrgUnit,               /* IN              */
            PointerByReference ppszOutPassword /* OUT             */
            );

        int
        VmAfdCreateComputerAccountW(
            WString pwszUserName,             /* IN              */
            WString pwszPassword,             /* IN              */
            WString pwszMachineName,          /* IN              */
            WString pwszOrgUnit,              /* IN              */
            PointerByReference ppwszOutPassword /* OUT             */
            );

        int
        VmAfdQueryADA(
            String pszServerName,                     /* IN              */
            PointerByReference ppszComputer,          /*    OUT          */
            PointerByReference ppszDomain,            /*    OUT          */
            PointerByReference ppszDistinguishedName, /*    OUT          */
            PointerByReference ppszNetbiosName        /*    OUT          */
            );

        int
        VmAfdQueryADW(
            WString pwszServerName,                    /* IN              */
            PointerByReference ppwszComputer,          /*    OUT          */
            PointerByReference ppwszDomain,            /*    OUT          */
            PointerByReference ppwszDistinguishedName, /*    OUT          */
            PointerByReference ppwszNetbiosName        /*    OUT          */
            );

        int
        VmAfdGetSiteGUIDA(
            String pszServerName,                      /* IN              */
            PointerByReference ppszSiteGUID            /*    OUT          */
            );

        int
        VmAfdGetSiteGUIDW(
            WString pwszServerName,                   /* IN               */
            PointerByReference ppwszSiteGUID          /*    OUT           */
            );

        void VmAfdFreeMemory(Pointer pMemory);

        int
        VmAfdGetSSLCertificate(
                String pszServerName,
                PointerByReference ppszCertificate,
                PointerByReference ppszPrivateKey
                );

        int
        VmAfdSetSSLCertificate(
                String pszServerName,
                String ppszCertificate,
                String ppszPrivateKey
                );

        int
        VmAfdSetMachineIDA(
                String pszServerName,
                String pszMachineId
                );

        int
        VmAfdSetMachineIDW(
                WString pwszServerName,
                WString pwszMachineId
                );

        int
        VmAfdGetMachineIDA(
                String             pszServerName,
                PointerByReference ppszMachineId
                );

        int
        VmAfdGetMachineIDW(
                WString            pwszServerName,
                PointerByReference ppwszMachineId
                );

        int
        VmAfdStartHeartbeatA(
                String             pszServiceName,
                int                port,
                PointerByReference ppHeartbeatHandle
                );

        int
        VmAfdStartHeartbeatW(
                WString            pwszServiceName,
                int                port,
                PointerByReference ppHeartbeatHandle
                );

        int
        VmAfdStopHeartbeat(
                Pointer           pHeartbeatHandle
                );
    }

    private Pointer _vmAfHeartbeatHandle;

    public static int getStatus(String hostname)
    {
        IntByReference pStatus = new IntByReference(0);
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetStatusA(hostname, pStatus);
        if( errCode != 0 )
        {
            throw new VmAfClientNativeException(errCode);
        }
        return pStatus.getValue();
    }

    public static String getDomainName(String hostname)
    {
        PointerByReference ppszDomain = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetDomainNameA(
                                                        hostname,
                                                        ppszDomain);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszDomain.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszDomain.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void setDomainName(String hostname, String domain)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetDomainNameA(
                                                        hostname,
                                                        domain);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static String getDomainState(String hostname)
    {
        PointerByReference pDomainState = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetDomainStateA(
                                                        hostname,
                                                        pDomainState);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return pDomainState.getValue().getString(0);
        }
        finally
        {
            Pointer val = pDomainState.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static String getLDU(String hostname)
    {
        PointerByReference ppszLDU = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetLDUA(
                                                        hostname,
                                                        ppszLDU);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszLDU.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszLDU.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void setLDU(String hostname, String ldu)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetLDUA(
                                                        hostname,
                                                        ldu);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static String getDCName(String hostname)
    {
        PointerByReference ppszDCName = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetDCNameA(
                                                        hostname,
                                                        ppszDCName);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszDCName.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszDCName.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void setDCName(String hostname, String dc)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetDCNameA(
                                                        hostname,
                                                        dc);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static int getRHTTPProxyPort(String hostname)
    {
        IntByReference pdwPort = new IntByReference(0);
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetRHTTPProxyPortA(hostname, pdwPort);
        if( errCode != 0 )
        {
            throw new VmAfClientNativeException(errCode);
        }
        return pdwPort.getValue();
    }

    public static void setRHTTPProxyPort(String hostname, int port)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetRHTTPProxyPortA(
                                                        hostname,
                                                        port);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static void setDCPort(String hostname, int port)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetDCPortA(
                                                        hostname,
                                                        port);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static String getCMLocation(String hostname)
    {
        PointerByReference ppszCMLocation = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetCMLocationA(
                                                        hostname,
                                                        ppszCMLocation);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszCMLocation.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszCMLocation.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static String getLSLocation(String hostname)
    {
        PointerByReference ppszLSLocation = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetLSLocationA(
                                                        hostname,
                                                        ppszLSLocation);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszLSLocation.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszLSLocation.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static String getPNID(String hostname)
    {
        PointerByReference ppszPNID = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetPNIDA(
                                                        hostname,
                                                        ppszPNID);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszPNID.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszPNID.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }
    
    public static String getPNIDUrl(String hostname)
    {
        PointerByReference ppszPNID = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetPNIDForUrlA(
                                                        hostname,
                                                        ppszPNID);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszPNID.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszPNID.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void setPNID(String hostname, String PNID)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetPNIDA(
                                                        hostname,
                                                        PNID);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static String
    getMachineCert(String hostname)
    {
        PointerByReference ppszCertificate = new PointerByReference();
        PointerByReference ppszPrivateKey = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetSSLCertificate(
                                                        hostname,
                                                        ppszCertificate,
                                                        ppszPrivateKey);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszCertificate.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszCertificate.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }

            val = ppszPrivateKey.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }

        }
    }

    public static String
    getMachinePrivateKey(String hostname)
    {
        PointerByReference ppszCertificate = new PointerByReference();
        PointerByReference ppszPrivateKey = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetSSLCertificate(
                                                        hostname,
                                                        ppszCertificate,
                                                        ppszPrivateKey);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszPrivateKey.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszCertificate.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }

            val = ppszPrivateKey.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }

        }
    }

    public static void
    setMachineCert(String hostname, String certificate, String privatekey)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetSSLCertificate(hostname, certificate, privatekey);
         if (errCode != 0)
          {
                 throw new VmAfClientNativeException(errCode);
          }
    }

    public static void
    promoteVmDir(String pszServerName,    /* IN              */
                 String pszDomainName,    /* IN              */
                 String pszUserName,      /* IN              */
                 String pszPassword,      /* IN              */
                 String pszSiteName,      /* IN              */
                 String pszPartnerName)   /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdPromoteVmDirA(
                          pszServerName,
                          pszDomainName,
                          pszUserName,
                          pszPassword,
                          pszSiteName,
                          pszPartnerName);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static void
    demoteVmDir(String pszServerName,    /* IN              */
                String pszUserName,      /* IN              */
                String pszPassword)      /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdDemoteVmDirA(
                          pszServerName,
                          pszUserName,
                          pszPassword);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static void
    joinVmDir(String pszServerName,    /* IN              */
              String pszUserName,      /* IN              */
              String pszPassword,      /* IN              */
              String pszMachineName,   /* IN              */
              String pszDomainName,    /* IN              */
              String pszOrgUnit)       /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdJoinVmDirA(
                          pszServerName,
                          pszUserName,
                          pszPassword,
                          pszMachineName,
                          pszDomainName,
                          pszOrgUnit);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static void
    leaveVmDir(String pszServerName,    /* IN              */
               String pszUserName,      /* IN              */
               String pszPassword)      /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdLeaveVmDirA(
                          pszServerName,
                          pszUserName,
                          pszPassword);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static String
    createComputerAccount(
              String pszUserName,              /* IN              */
              String pszPassword,              /* IN              */
              String pszMachineName,           /* IN              */
              String pszOrgUnit)               /* IN              */
    {
        PointerByReference ppszOutPassword = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdCreateComputerAccountA(
                              pszUserName,
                              pszPassword,
                              pszMachineName,
                              pszOrgUnit,
                              ppszOutPassword);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszOutPassword.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszOutPassword.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void
    joinAD(String pszServerName,    /* IN              */
           String pszUserName,      /* IN              */
           String pszPassword,      /* IN              */
           String pszDomainName,    /* IN              */
           String pszOrgUnit)       /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdJoinADA(
                          pszServerName,
                          pszUserName,
                          pszPassword,
                          pszDomainName,
                          pszOrgUnit);
        VmAfClientAdapterErrorHandler.handleErrorCode(errCode);
    }

    public static void
    leaveAD(String pszServerName,    /* IN              */
            String pszUserName,      /* IN              */
            String pszPassword)      /* IN              */
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdLeaveADA(
                          pszServerName,
                          pszUserName,
                          pszPassword);
        VmAfClientAdapterErrorHandler.handleErrorCode(errCode);
    }

    public static DomainInfo
    queryAD(String pszServerName)
    {
        PointerByReference ppszComputer = new PointerByReference();
        PointerByReference ppszDomain = new PointerByReference();
        PointerByReference ppszDistinguishedName = new PointerByReference();
        PointerByReference ppszNetbiosName = new PointerByReference();
        Pointer ptr = null;
        String pszComputer = null;
        String pszDomain = null;
        String pszDistinguishedName = null;
        String pszNetbiosName = null;

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdQueryADA(
                              pszServerName,
                              ppszComputer,
                              ppszDomain,
                              ppszDistinguishedName,
                              ppszNetbiosName);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            if (ppszComputer != null)
            {
                ptr = ppszComputer.getValue();
                if (ptr != Pointer.NULL && ptr != null)
                {
                    pszComputer = ptr.getString(0);
                }
            }

            if (ppszDomain != null)
            {
                ptr = ppszDomain.getValue();
                if (ptr != Pointer.NULL && ptr != null)
                {
                    pszDomain = ptr.getString(0);
                }
            }

            if (ppszDistinguishedName != null)
            {
                ptr = ppszDistinguishedName.getValue();
                if (ptr != Pointer.NULL && ptr != null)
                {
                    pszDistinguishedName = ptr.getString(0);
                }
            }

            if (ppszNetbiosName != null)
            {
                ptr = ppszNetbiosName.getValue();
                if (ptr != Pointer.NULL && ptr != null)
                {
                    pszNetbiosName = ptr.getString(0);
                }
            }

            return new DomainInfo(
                         pszComputer,
                         pszDomain,
                         pszDistinguishedName,
                         pszNetbiosName);
        }
        finally
        {
            Pointer val = ppszComputer.getValue();
            if (val != Pointer.NULL && val != null)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
            val = ppszDomain.getValue();
            if (val != Pointer.NULL && val != null)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
            val = ppszDistinguishedName.getValue();
            if (val != Pointer.NULL && val != null)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
            val = ppszNetbiosName.getValue();
            if (val != Pointer.NULL && val != null)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static String
    getMachineID(String hostname)
    {
        PointerByReference ppszMachineId = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetMachineIDA(
                                                        hostname,
                                                        ppszMachineId);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszMachineId.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszMachineId.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static void
    setMachineID(String hostname, String machineId)
    {
        int errCode = VmAfClientLibrary.INSTANCE.VmAfdSetMachineIDA(
                                                        hostname,
                                                        machineId);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }
    }

    public static PasswordCredential
    getMachineAccountCredentials()
    {
	PointerByReference ppszAccount = new PointerByReference();
	PointerByReference ppszPassword = new PointerByReference();

	try
	{
	    int code = VmAfClientLibrary.INSTANCE.VmAfdGetMachineAccountInfoA(
		    "localhost",
		    ppszAccount,
		    ppszPassword);

	        if (code != 0)
	        {
	            throw new VmAfClientNativeException(code);
	        }

	        return new PasswordCredential(
	        ppszAccount.getValue().getString(0),
	        ppszPassword.getValue().getString(0));
	        }
	finally
	{
            Pointer pszAccount = ppszAccount.getValue();

            if (pszAccount != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(pszAccount);
            }

            Pointer pszPassword = ppszPassword.getValue();

            if (pszPassword != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(pszPassword);
            }
            }
    }

    public static String getSiteGUID(String hostname)
    {
        PointerByReference ppszSiteGUID = new PointerByReference();

        try
        {
            int errCode = VmAfClientLibrary.INSTANCE.VmAfdGetSiteGUIDA(
                                                        hostname,
                                                        ppszSiteGUID);
            if (errCode != 0)
            {
                throw new VmAfClientNativeException(errCode);
            }

            return ppszSiteGUID.getValue().getString(0);
        }
        finally
        {
            Pointer val = ppszSiteGUID.getValue();

            if (val != Pointer.NULL)
            {
                VmAfClientLibrary.INSTANCE.VmAfdFreeMemory(val);
            }
        }
    }

    public static Object
    startHeartbeat(String ServiceName, int Port)
    {
        PointerByReference ppHeartbeatHandle = new PointerByReference();

        int errCode = VmAfClientLibrary.INSTANCE.VmAfdStartHeartbeatA(
                                                               ServiceName,
                                                               Port,
                                                               ppHeartbeatHandle);
        if (errCode != 0)
        {
            throw new VmAfClientNativeException(errCode);
        }

        return ppHeartbeatHandle.getValue();
    }

    public static void
    stopHeartbeat(Object pHeartbeatHandle)
    {
        Pointer HeartbeatPointer= null;
        if (pHeartbeatHandle != null && pHeartbeatHandle instanceof Pointer)
        {
            HeartbeatPointer = (Pointer)pHeartbeatHandle;
            VmAfClientLibrary.INSTANCE.VmAfdStopHeartbeat(HeartbeatPointer);
            HeartbeatPointer = null;
        }
    }
}
