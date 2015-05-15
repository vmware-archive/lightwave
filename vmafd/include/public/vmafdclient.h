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
VmAfdLeaveVmDirA(
    PCSTR pszServerName,    /* IN     OPTIONAL */
    PCSTR pszUserName,      /* IN              */
    PCSTR pszPassword       /* IN              */
    );

DWORD
VmAfdLeaveVmDirW(
    PCWSTR pwszServerName,  /* IN     OPTIONAL */
    PCWSTR pwszUserName,    /* IN              */
    PCWSTR pwszPassword     /* IN              */
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
#define VmAfdJoinAD                     VmAfdJoinADW
#define VmAfdLeaveAD                    VmAfdLeaveADW
#define VmAfdQueryAD                    VmAfdQueryADW
#define VmAfdGetPNID                    VmAfdGetPNIDW
#define VmAfdSetPNID                    VmAfdSetPNIDW
#define VmAfdSetCAPath                  VmAfdSetCAPathW
#define VmAfdGetCAPath                  VmAfdGetCAPathW

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
#define VmAfdJoinAD                     VmAfdJoinADA
#define VmAfdLeaveAD                    VmAfdLeaveADA
#define VmAfdQueryAD                    VmAfdQueryADA
#define VmAfdGetPNID                    VmAfdGetPNIDA
#define VmAfdSetPNID                    VmAfdSetPNIDA
#define VmAfdSetCAPath                  VmAfdSetCAPathA
#define VmAfdGetCAPath                  VmAfdGetCAPathA

#endif

#ifdef __cplusplus
}
#endif

#endif /* VMAFDCLIENT_H_ */
