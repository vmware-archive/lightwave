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



/*
 * Module Name: vdcrepadmin
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcrepadmin main module entry point
 *
 */

#include "includes.h"

DWORD
VmDirShowReplicationPartnerInfo(
    DWORD               dwNumPartner,
    PVMDIR_REPL_PARTNER_INFO   pReplPartnerInfo
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    for (i=0; i<dwNumPartner; i++)
    {
        printf("%s\n", pReplPartnerInfo[i].pszURI);
    }
    return dwError;
}

DWORD
VmDirShowReplicationPartnerStatus(
    DWORD               dwNumPartner,
    PVMDIR_REPL_PARTNER_STATUS   pReplPartnerStatus
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    USN lag = 0;
    for (i=0; i<dwNumPartner; i++)
    {
        printf("Partner: %s\n", pReplPartnerStatus[i].pszHost);
        printf("Host available:   %s\n", pReplPartnerStatus[i].bHostAvailable ? "Yes" : "No");
        if (pReplPartnerStatus[i].bHostAvailable)
        {
            printf("Status available: %s\n", pReplPartnerStatus[i].bStatusAvailable ? "Yes" : "No");
            if (pReplPartnerStatus[i].bStatusAvailable)
            {
                lag = pReplPartnerStatus[i].targetUsn - pReplPartnerStatus[i].partnerUsn;
#ifdef _WIN32
                printf("My last change number:             %I64d\n", (_Longlong)pReplPartnerStatus[i].targetUsn);
                printf("Partner has seen my change number: %I64d\n", (_Longlong)pReplPartnerStatus[i].partnerUsn);
                printf("Partner is %I64d changes behind.\n", (_Longlong)lag);
#else
                printf("My last change number:             %" PRId64 "\n", (int64_t)pReplPartnerStatus[i].targetUsn);
                printf("Partner has seen my change number: %" PRId64 "\n", (int64_t)pReplPartnerStatus[i].partnerUsn);
                printf("Partner is %" PRId64 " changes behind.\n", (int64_t)lag);
#endif
            }
        }
        if (i < dwNumPartner - 1)
        {
            printf("\n");
        }
    }
    return dwError;
}

DWORD
VmDirShowServerInfo(
    DWORD               dwNumServer,
    PVMDIR_SERVER_INFO   pServerInfo
)
{
    DWORD dwError = 0;
    DWORD i = 0;
    for (i=0; i<dwNumServer; i++)
    {
        printf("%s\n", pServerInfo[i].pszServerDN);
    }
    return dwError;
}

int
main(int argc, char* argv[])
{
    DWORD       dwError = 0;
    DWORD       i       = 0;
    PSTR        pszFeatureSet  = NULL;
    PSTR        pszSrcHostName = NULL;
    PSTR        pszSrcPort     = NULL;
    PSTR        pszSrcUserName = NULL;
    PSTR        pszSrcPassword = NULL;
    PSTR        pszTgtHostName = NULL;
    PSTR        pszTgtPort     = NULL;
    BOOLEAN     bVerbose       = FALSE;
    BOOLEAN     bTwoWayRepl    = FALSE;

    PVMDIR_REPL_PARTNER_INFO    pReplPartnerInfo       = NULL;
    PVMDIR_REPL_PARTNER_STATUS  pReplPartnerStatus     = NULL;
    PVMDIR_SERVER_INFO          pServerInfo            = NULL;
    DWORD                       dwReplPartnerInfoCount = 0;
    DWORD                       dwReplPartnerStatusCount = 0;
    DWORD                       dwServerInfoCount      = 0;

    CHAR        pszPath[MAX_PATH];

#ifndef _WIN32
    setlocale(LC_ALL,"");
#endif

    dwError = VmDirGetVmDirLogPath(pszPath, "vdcrepadmin.log");
    BAIL_ON_VMDIR_ERROR(dwError);
    dwError = VmDirLogInitialize(pszPath, FALSE, NULL, VMDIR_LOG_INFO, VMDIR_LOG_MASK_ALL );
    BAIL_ON_VMDIR_ERROR(dwError);

    //get commandline parameters
    dwError = VmDirParseArgs(
                    argc,
                    argv,
                    &pszFeatureSet,
                    &bTwoWayRepl,
                    &pszSrcHostName,
                    &pszSrcPort,
                    &pszSrcUserName,
                    &pszSrcPassword,
                    &pszTgtHostName,
                    &pszTgtPort,
                    &bVerbose
                    );

    if (bVerbose)
    {
        VmDirSetLogLevel( "VERBOSE" );
    }

    if (dwError)
    {
        ShowUsage();
        goto cleanup;
    }

    if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNERS, pszFeatureSet, TRUE) == 0 )
    {
        dwError = VmDirGetReplicationPartners(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pReplPartnerInfo,
                        &dwReplPartnerInfoCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowReplicationPartnerInfo(
                                dwReplPartnerInfoCount,
                                pReplPartnerInfo
                                );

        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_PARTNER_STATUS, pszFeatureSet, TRUE) == 0 )
    {
        dwError = VmDirGetReplicationPartnerStatus(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pReplPartnerStatus,
                        &dwReplPartnerStatusCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowReplicationPartnerStatus(
                                dwReplPartnerStatusCount,
                                pReplPartnerStatus
                                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_SHOW_SERVER_ATTRIBUTE, pszFeatureSet, TRUE) == 0 )
    {
        dwError = VmDirGetServers(
                        pszSrcHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pServerInfo,
                        &dwServerInfoCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        //Show replication partner info
        dwError = VmDirShowServerInfo(
                                dwServerInfoCount,
                                pServerInfo
                                );

        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_CREATE_AGREEMENT, pszFeatureSet, TRUE) == 0 )
    {
        dwError = VmDirAddReplicationAgreement(
                    bTwoWayRepl,
                    pszSrcHostName,
                    pszSrcPort,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtHostName,
                    pszTgtPort
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if ( VmDirStringCompareA(VDCREPADMIN_FEATURE_REMOVE_AGREEMENT, pszFeatureSet, TRUE) == 0 )
    {
        dwError = VmDirRemoveReplicationAgreement(
                    bTwoWayRepl,
                    pszSrcHostName,
                    pszSrcPort,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtHostName,
                    pszTgtPort
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
cleanup:
    // Free internal memory used
    for (i=0; i<dwReplPartnerInfoCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo[i].pszURI);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);

    for (i=0; i<dwReplPartnerStatusCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus[i].pszHost);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus);

    // Free internal memory used
    for (i=0; i<dwServerInfoCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pServerInfo[i].pszServerDN);
    }

    VMDIR_SAFE_FREE_MEMORY(pServerInfo);

    return dwError;

error:
    printf("Vdcrepadmin failed. Error[%d]\n", dwError);
    goto cleanup;
}

