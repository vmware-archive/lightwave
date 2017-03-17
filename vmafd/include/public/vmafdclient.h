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



#ifndef __VMAFDCLIENT_H_
#define __VMAFDCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vmafdtypes.h"
#include "vecsclient.h"

typedef struct _VMAFD_HB_HANDLE* PVMAFD_HB_HANDLE;

DWORD
VmAfdOpenServerA(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PVMAFD_SERVER *ppServer);

DWORD
VmAfdOpenServerW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PVMAFD_SERVER *ppServer);

DWORD
VmAfdOpenServerWithTimeoutA(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    DWORD dwTimeout,
    PVMAFD_SERVER *ppServer);

DWORD
VmAfdOpenServerWithTimeoutW(
    PCWSTR pwszServerName,
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    DWORD  dwTimeout,
    PVMAFD_SERVER *ppServer);

VOID
VmAfdCloseServer(
    PVMAFD_SERVER pServer);

DWORD
VmAfdGetStatusA(
    PCSTR         pszServerName,  /* IN     OPTIONAL */
    PVMAFD_STATUS pStatus         /* IN OUT          */
    );

/**
 * Returns the status of the service
 *
 * @param pwszServerName The hostname where the service is running
 * @param pStatus        Receives the status of the service
 *
 * @return 0 if success, positive error code otherwise
 */
DWORD
VmAfdGetStatusW(
    PCWSTR        pwszServerName, /* IN     OPTIONAL */
    PVMAFD_STATUS pStatus         /* IN OUT          */
    );

DWORD
VmAfdGetStatusRPCA(
    PCSTR         pszServerName,  /* IN     OPTIONAL */
    PVMAFD_STATUS pStatus         /* IN OUT          */
);

DWORD
VmAfdGetStatusRPCW(
    PCWSTR        pwszServerName, /* IN     */
    PVMAFD_STATUS pStatus         /* IN OUT */
);

DWORD
VmAfdGetDomainNameA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszDomain     /*    OUT          */
    );

DWORD
VmAfdGetDomainNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszDomain     /*    OUT          */
    );

DWORD
VmAfdSetDomainNameA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PCSTR pszDomain      /* IN              */
    );

DWORD
VmAfdSetDomainNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pwszDomain      /* IN              */
    );

DWORD
VmAfdGetDomainStateA(
    PCSTR pszServerName,              /* IN     OPTIONAL */
    VMAFD_DOMAIN_STATE* pDomainState  /*    OUT          */
    );

DWORD
VmAfdGetDomainStateW(
    PCWSTR pwszServerName,            /* IN     OPTIONAL */
    VMAFD_DOMAIN_STATE* pDomainState  /*    OUT          */
    );

DWORD
VmAfdGetLDUA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszLDU        /* IN              */
    );

DWORD
VmAfdGetLDUW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszLDU        /*    OUT          */
    );
DWORD
VmAfdSetLDUA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PCSTR pszLDU         /* IN              */
    );

DWORD
VmAfdSetLDUW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pwszLDU         /* IN              */
    );

DWORD
VmAfdGetRHTTPProxyPortA(
    PCSTR   pszServerName, /* IN    OPTIONAL */
    UINT32* pdwPort        /* IN    OPTIONAL */
    );

DWORD
VmAfdGetRHTTPProxyPortW(
    PCWSTR  pwszServerName, /* IN    OPTIONAL */
    UINT32* pdwPort         /* IN    OPTIONAL */
    );

DWORD
VmAfdSetRHTTPProxyPortA(
    PCSTR  pwszServerName, /* IN    OPTIONAL */
    UINT32 dwPort          /* IN             */
    );

DWORD
VmAfdSetRHTTPProxyPortW(
    PCWSTR pwszServerName, /* IN    OPTIONAL */
    UINT32 dwPort          /* IN             */
    );

DWORD
VmAfdSetDCPortA(
    PCSTR  pwszServerName, /* IN    OPTIONAL */
    UINT32 dwPort          /* IN             */
    );

DWORD
VmAfdSetDCPortW(
    PCWSTR pwszServerName, /* IN    OPTIONAL */
    UINT32 dwPort          /* IN             */
    );

DWORD
VmAfdGetCMLocationA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszCMLocation /*    OUT          */
    );

DWORD
VmAfdGetCMLocationW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszCMLocation /*    OUT          */
    );

DWORD
VmAfdGetLSLocationA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszLSLocation /*    OUT          */
    );

DWORD
VmAfdGetLSLocationW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszLSLocation /*    OUT          */
    );

DWORD
VmAfdGetDCNameA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszDCName     /*    OUT          */
    );

DWORD
VmAfdGetDCNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszDCName     /*    OUT          */
    );

DWORD
VmAfdGetDCNameExA(
    PCSTR pszServerName, /* IN     OPTIONAL */
    PSTR* ppszDCName     /*    OUT          */
    );

DWORD
VmAfdGetDCNameExW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszDCName     /*    OUT          */
    );

DWORD
VmAfdSetDCNameA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PCSTR pszDCName         /* IN              */
    );

DWORD
VmAfdSetDCNameW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PCWSTR pswzDCName      /* IN              */
    );

DWORD
VmAfdGetMachineAccountInfoA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PSTR* ppszAccount,      /* IN              */
    PSTR* ppszPassword      /* IN              */
    );

DWORD
VmAfdGetMachineAccountInfoW(
    PCWSTR pwszServerName, /* IN     OPTIONAL */
    PWSTR* ppwszAccount,   /*    OUT          */
    PWSTR* ppwszPassword   /*    OUT          */
    );

DWORD
VmAfdGetSiteGUIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszSiteGUID    /*    OUT          */
    );

DWORD
VmAfdGetSiteGUIDW(
    PCWSTR pszServerName,  /* IN     OPTIONAL */
    PWSTR* ppszSiteGUID    /*    OUT          */
    );

DWORD
VmAfdGetSiteNameA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszSiteName    /*    OUT          */
    );

DWORD
VmAfdGetSiteNameW(
    PCWSTR pszServerName,  /* IN     OPTIONAL */
    PWSTR* ppszSiteName    /*    OUT          */
    );

DWORD
VmAfdGetSiteNameHA(
    PVMAFD_SERVER pServer,
    PSTR*  ppszSiteName    /*    OUT          */
    );

DWORD
VmAfdGetSiteNameHW(
    PVMAFD_SERVER pServer,
    PWSTR* ppwszSiteName   /*    OUT          */
    );

DWORD
VmAfdGetMachineIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PSTR*  ppszMachineID    /*    OUT          */
    );

DWORD
VmAfdGetMachineIDW(
    PCWSTR pszServerName,  /* IN     OPTIONAL */
    PWSTR* ppszMachineID   /*    OUT          */
    );

DWORD
VmAfdSetMachineIDA(
    PCSTR  pszServerName,  /* IN     OPTIONAL */
    PCSTR  pszMachineID    /* IN              */
    );

DWORD
VmAfdSetMachineIDW(
    PCWSTR  pszServerName,  /* IN     OPTIONAL */
    PCWSTR  pszMachineID    /* IN              */
    );

DWORD
VmAfdPromoteVmDirA(
    PCSTR pszServerName,     /* IN     OPTIONAL */
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword,       /* IN              */
    PCSTR pszSiteName,       /* IN     OPTIONAL */
    PCSTR pszPartnerHostName /* IN     OPTIONAL */
    );

DWORD
VmAfdPromoteVmDirW(
    PCWSTR pwszServerName,    /* IN     OPTIONAL */
    PCWSTR pwszDomainName,    /* IN     OPTIONAL */
    PCWSTR pwszUserName,      /* IN              */
    PCWSTR pwszPassword,      /* IN              */
    PCWSTR pwszSiteName,      /* IN     OPTIONAL */
    PCWSTR pszPartnerHostName /* IN     OPTIONAL */
    );

DWORD
VmAfdDemoteVmDirA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword       /* IN              */
    );

DWORD
VmAfdDemoteVmDirW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword     /* IN              */
    );

DWORD
VmAfdJoinValidateDomainCredentialsW(
    PCWSTR pwszDomainName,  /* IN              */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword     /* IN              */
    );

DWORD
VmAfdJoinValidateDomainCredentialsA(
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword        /* IN              */
    );

DWORD
VmAfdJoinVmDirA(
    PCSTR pszServerName,     /* IN     OPTIONAL */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword,       /* IN              */
    PCSTR pszMachineName,    /* IN              */
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszOrgUnit         /* IN     OPTIONAL */
    );

DWORD
VmAfdJoinVmDirW(
    PCWSTR pwszServerName,     /* IN     OPTIONAL */
    PCWSTR pwszUserName,       /* IN              */
    PCWSTR pwszPassword,       /* IN              */
    PCWSTR pwszMachineName,    /* IN              */
    PCWSTR pwszDomainName,     /* IN              */
    PCWSTR pwszOrgUnit         /* IN     OPTIONAL */
    );

DWORD
VmAfdJoinVmDir2A(
    PCSTR            pszDomainName,  /* IN              */
    PCSTR            pszUserName,    /* IN              */
    PCSTR            pszPassword,    /* IN              */
    PCSTR            pszMachineName, /* IN     OPTIONAL */
    PCSTR            pszOrgUnit,     /* IN     OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags         /* IN              */
    );

DWORD
VmAfdJoinVmDir2W(
    PCWSTR           pwszDomainName,  /* IN            */
    PCWSTR           pwszUserName,    /* IN            */
    PCWSTR           pwszPassword,    /* IN            */
    PCWSTR           pwszMachineName, /* IN   OPTIONAL */
    PCWSTR           pwszOrgUnit,     /* IN   OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags          /* IN            */
    );

DWORD
VmAfdLeaveVmDirA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword,      /* IN              */
    DWORD dwLeaveFlags      /* IN              */
    );

DWORD
VmAfdLeaveVmDirW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pszPassword,     /* IN              */
    DWORD dwLeaveFlags      /* IN              */
    );

DWORD
VmAfdCreateComputerAccountA(
    PCSTR pszUserName,        /* IN              */
    PCSTR pszPassword,        /* IN              */
    PCSTR pszMachineName,     /* IN              */
    PCSTR pszOrgUnit,         /* IN     OPTIONAL */
    PSTR* ppszOutPassword     /* OUT             */
    );

DWORD
VmAfdCreateComputerAccountW(
    PCWSTR pwszUserName,      /* IN              */
    PCWSTR pwszPassword,      /* IN              */
    PCWSTR pwszMachineName,   /* IN              */
    PCWSTR pwszOrgUnit,       /* IN     OPTIONAL */
    PWSTR* ppwszOutPassword   /* OUT             */
    );

DWORD
VmAfdJoinADA(
    PCSTR pszServerName,     /* IN     OPTIONAL */
    PCSTR pszUserName,       /* IN              */
    PCSTR pszPassword,       /* IN              */
    PCSTR pszDomainName,     /* IN              */
    PCSTR pszOrgUnit         /* IN     OPTIONAL */
    );

DWORD
VmAfdJoinADW(
    PCWSTR pwszServerName,     /* IN     OPTIONAL */
    PCWSTR pwszUserName,       /* IN              */
    PCWSTR pwszPassword,       /* IN              */
    PCWSTR pwszDomainName,     /* IN              */
    PCWSTR pwszOrgUnit         /* IN     OPTIONAL */
    );

DWORD
VmAfdLeaveADA(
    PCSTR pszServerName,    /* IN        OPTIONAL */
    PCSTR pszUserName,      /* IN                 */
    PCSTR pszPassword       /* IN                 */
    );

DWORD
VmAfdLeaveADW(
    PCWSTR pwszServerName,  /* IN        OPTIONAL */
    PCWSTR pwszUserName,    /* IN                 */
    PCWSTR pwszPassword     /* IN                 */
    );

DWORD
VmAfdQueryADA(
    PCSTR pszServerName,        /* IN          OPTIONAL */
    PSTR *pszComputer,          /*    OUT               */
    PSTR *pszDomain,            /*    OUT               */
    PSTR *pszDistinguishedName, /*    OUT               */
    PSTR *pszNetbiosName        /*    OUT               */
    );

DWORD
VmAfdQueryADW(
    PCWSTR pwszServerName,        /* IN        OPTIONAL */
    PWSTR *pwszComputer,          /*    OUT             */
    PWSTR *pwszDomain,            /*    OUT             */
    PWSTR *pwszDistinguishedName, /*    OUT             */
    PWSTR *pwszNetbiosName        /*    OUT             */
    );

DWORD
VmAfdGetDCList (
    PCSTR  pszServerName,
    PCSTR  pszDomain,
    PDWORD pdwServerCount,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList
    );

DWORD
VmAfdForceReplicationA(
    PCSTR pszServerName       /* IN              */
    );

DWORD
VmAfdForceReplicationW(
    PCWSTR pwszServerName       /* IN              */
    );

VOID
VmAfdFreeCertArray(
    PVMAFD_CERT_ARRAY pArray
    );

VOID
VmAfdFreeString(
    PSTR pszString
    );

VOID
VmAfdFreeWString(
    PWSTR pwszString
    );

DWORD
VmAfdGetSSLCertificate(
    PCSTR pszServerName,
    PSTR *ppszCert,
    PSTR *ppszPrivateKey
);

DWORD
VmAfdSetSSLCertificate(
    PCSTR pszServerName,
    PSTR  pszCert,
    PSTR  pszPrivateKey
);

DWORD
VmAfdTriggerRootCertsRefresh(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword
    );


DWORD
VmAfdGetPNIDForUrlA(
    PCSTR pszServerName,
    PSTR* ppszPNIDUrl
    );

DWORD
VmAfdGetPNIDForUrlW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPNIDUrl
    );

DWORD
VmAfdGetPNIDA(
    PCSTR pszServerName,
    PSTR* ppszPNID
    );

DWORD
VmAfdGetPNIDW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPNID
    );

DWORD
VmAfdSetPNIDA(
    PCSTR pszServerName,
    PCSTR pszPNID
    );

DWORD
VmAfdSetPNIDW(
    PCWSTR pwszServerName,
    PCWSTR pwszPNID
    );

DWORD
VmAfdSetCAPathA(
    PCSTR pszServerName,
    PCSTR pszPath
    );

DWORD
VmAfdSetCAPathW(
    PCWSTR pwszServerName,
    PCWSTR pwszPath
    );

DWORD
VmAfdGetCAPathA(
    PCSTR pszServerName,
    PSTR* ppszPath
    );

DWORD
VmAfdGetCAPathW(
    PCWSTR pwszServerName,
    PWSTR* ppwszPath
    );

DWORD
VmAfdStartHeartbeatA(
    PCSTR  pszServiceName,
    DWORD  dwServicePort,
    PVMAFD_HB_HANDLE *ppHandle
    );

DWORD
VmAfdStartHeartbeatW(
    PCWSTR pwszServiceName,
    DWORD  dwServicePort,
    PVMAFD_HB_HANDLE *ppHandle
    );

DWORD
VmAfdGetHeartbeatStatusA(
    PVMAFD_SERVER       pServer,
    PVMAFD_HB_STATUS_A* ppHeartbeatStatus
    );

DWORD
VmAfdGetHeartbeatStatusW(
    PVMAFD_SERVER       pServer,
    PVMAFD_HB_STATUS_W* ppHeartbeatStatus
    );

VOID
VmAfdStopHeartbeat(
    PVMAFD_HB_HANDLE pHandle
    );

VOID
VmAfdFreeHeartbeatStatusA(
    PVMAFD_HB_STATUS_A pHeartbeatStatus
    );

VOID
VmAfdFreeHeartbeatStatusW(
    PVMAFD_HB_STATUS_W pHeartbeatStatus
    );

DWORD
VmAfdRefreshSiteName(
    );

DWORD
VmAfdGetErrorMsgByCode(
    DWORD dwErrorCode,
    PSTR *pszErrMsg
    );

DWORD
VmAfdConfigureDNSA(
    PCSTR pszUserName,
    PCSTR pszPassword
    );

DWORD
VmAfdConfigureDNSW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword
    );

DWORD
VmAfdSuperLogEnable(
    PVMAFD_SERVER       pServer
    );

DWORD
VmAfdSuperLogGetSize(
    PVMAFD_SERVER       pServer,
    PDWORD    pdwSize
    );

DWORD
VmAfdSuperLogGetEntries(
    PVMAFD_SERVER       pServer,
    UINT32 **ppEnumerationCookie,
    DWORD     dwCount, // 0 ==> all
    PVMAFD_SUPERLOG_ENTRY_ARRAY *ppEntries
    );

DWORD
VmAfdIsSuperLogEnabled(
    PVMAFD_SERVER       pServer,
    PBOOLEAN pbEnabled
    );

DWORD
VmAfdSuperLogSetSize(
    PVMAFD_SERVER       pServer,
    DWORD    dwSize
    );

DWORD
VmAfdClearSuperLog(
    PVMAFD_SERVER       pServer
    );

DWORD
VmAfdSuperLogDisable(
    PVMAFD_SERVER    pServer
    );

DWORD
VmAfdChangePNIDA(
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszPNID
    );

DWORD
VmAfdChangePNIDW(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    );

#ifdef UNICODE

#define VmAfdOpenServer                 VmAfdOpenServerW
#define VmAfdGetStatus                  VmAfdGetStatusW
#define VmAfdGetStatusRPC               VmAfdGetStatusRPCW
#define VmAfdGetDomainName              VmAfdGetDomainNameW
#define VmAfdSetDomainName              VmAfdSetDomainNameW
#define VmAfdGetDomainState             VmAfdGetDomainStateW
#define VmAfdSetDomainState             VmAfdSetDomainStateW
#define VmAfdGetLDU                     VmAfdGetLDUW
#define VmAfdSetLDU                     VmAfdSetLDUW
#define VmAfdGetRHTTPProxyPort          VmAfdGetRHTTPProxyPortW
#define VmAfdSetRHTTPProxyPort          VmAfdSetRHTTPProxyPortW
#define VmAfdSetDCPort                  VmAfdSetDCPortW
#define VmAfdGetCMLocation              VmAfdGetCMLocationW
#define VmAfdGetLSLocation              VmAfdGetLSLocationW
#define VmAfdGetDCName                  VmAfdGetDCNameW
#define VmAfdSetDCName                  VmAfdSetDCNameW
#define VmAfdGetMachineAccountInfo      VmAfdGetMachineAccountInfoW
#define VmAfdPromoteVmDir               VmAfdPromoteVmDirW
#define VmAfdDemoteVmDir                VmAfdDemoteVmDirW
#define VmAfdJoinVmDir                  VmAfdJoinVmDirW
#define VmAfdLeaveVmDir                 VmAfdLeaveVmDirW
#define VmAfdCreateComputerAccount      VmAfdCreateComputerAccountW
#define VmAfdJoinAD                     VmAfdJoinADW
#define VmAfdLeaveAD                    VmAfdLeaveADW
#define VmAfdQueryAD                    VmAfdQueryADW
#define VmAfdGetPNIDForUrl              VmAfdGetPNIDForUrlW
#define VmAfdGetPNID                    VmAfdGetPNIDW
#define VmAfdSetPNID                    VmAfdSetPNIDW
#define VmAfdSetCAPath                  VmAfdSetCAPathW
#define VmAfdGetCAPath                  VmAfdGetCAPathW
#define VmAfdGetDCNameEx                VmAfdGetDCNameExW
#define VmAfdConfigureDNS               VmAfdConfigureDNSW

#else

#define VmAfdOpenServer                 VmAfdOpenServerA
#define VmAfdGetStatus                  VmAfdGetStatusA
#define VmAfdGetStatusRPC               VmAfdGetStatusRPCA
#define VmAfdGetDomainName              VmAfdGetDomainNameA
#define VmAfdSetDomainName              VmAfdSetDomainNameA
#define VmAfdGetDomainState             VmAfdGetDomainStateA
#define VmAfdSetDomainState             VmAfdSetDomainStateA
#define VmAfdGetLDU                     VmAfdGetLDUA
#define VmAfdSetLDU                     VmAfdSetLDUA
#define VmAfdGetRHTTPProxyPort          VmAfdGetRHTTPProxyPortA
#define VmAfdSetRHTTPProxyPort          VmAfdSetRHTTPProxyPortA
#define VmAfdSetDCPort                  VmAfdSetDCPortA
#define VmAfdGetCMLocation              VmAfdGetCMLocationA
#define VmAfdGetLSLocation              VmAfdGetLSLocationA
#define VmAfdGetDCName                  VmAfdGetDCNameA
#define VmAfdSetDCName                  VmAfdSetDCNameA
#define VmAfdGetMachineAccountInfo      VmAfdGetMachineAccountInfoA
#define VmAfdPromoteVmDir               VmAfdPromoteVmDirA
#define VmAfdDemoteVmDir                VmAfdDemoteVmDirA
#define VmAfdJoinVmDir                  VmAfdJoinVmDirA
#define VmAfdLeaveVmDir                 VmAfdLeaveVmDirA
#define VmAfdCreateComputerAccount      VmAfdCreateComputerAccountA
#define VmAfdJoinAD                     VmAfdJoinADA
#define VmAfdLeaveAD                    VmAfdLeaveADA
#define VmAfdQueryAD                    VmAfdQueryADA
#define VmAfdGetPNIDForUrl              VmAfdGetPNIDForUrlA
#define VmAfdGetPNID                    VmAfdGetPNIDA
#define VmAfdSetPNID                    VmAfdSetPNIDA
#define VmAfdSetCAPath                  VmAfdSetCAPathA
#define VmAfdGetCAPath                  VmAfdGetCAPathA
#define VmAfdGetDCNameEx                VmAfdGetDCNameExA
#define VmAfdConfigureDNS               VmAfdConfigureDNSA

#endif

#ifdef __cplusplus
}
#endif

#endif /* VMAFDCLIENT_H_ */
