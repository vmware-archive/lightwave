/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
_VmDirGetServersOnSite(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PCSTR               pszSiteName,
    PVMDIR_SERVER_INFO* ppServerInfo,   // output
    DWORD*              pdwNumServer    // output
);

static
DWORD
_VmDirGetListFromServerInfo(
    PVMDIR_SERVER_INFO  pServerInfo,
    DWORD               dwServerInfoCount,
    PVMDIR_STRING_LIST* ppList
    );

static
DWORD
_VmDirGetHAServerFromStringList(
    PVMDIR_STRING_LIST      pAllServerList,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PVMDIR_HA_SERVER_INFO** pppList,
    PDWORD                  pdwListCount,
    PDWORD                  pdwOnlineCount,
    PDWORD                  pdwOfflineCount
    );

static
DWORD
_VmDirFillPartnersInList(
    PVMDIR_HA_SERVER_INFO*  ppList,
    DWORD                   Count
    );

static
DWORD
_VmDirFillIntraTopology(
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_SERVER_INFO*          ppServerList,
    DWORD                           dwListCount,
    DWORD                           dwOnlineCount,
    DWORD                           dwOfflineCount,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppTopology
    );

static
VOID
_VmDirFreeHAServerList(
    PVMDIR_HA_SERVER_INFO*  ppList,
    DWORD                   dwCount
    );

static
DWORD
_VmDirCreateTopologyStructure(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology,
    PDWORD**                        pppdwCostMatrix,
    PDWORD*                         ppFinalResult
    );

static
DWORD
_VmDirFindFinalTopology(
    DWORD   dwNodeCount,
    PDWORD* ppdwCostMatrix,
    PDWORD  pdwFinalResult, // Output
    PDWORD  pdwCost // Output
    );

static
DWORD
_VmDirFindNNA(
    DWORD   dwNodeCount,
    PDWORD* ppdwCostMatrix,
    PDWORD  pdwFinalResult, // Output
    PDWORD  pdwCost // Output
    );

static
DWORD
_VmDirCreateNewTopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology,
    PDWORD                          pFinalResult,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppNewTopology
    );

static
VOID
_VmDirFreeTopologyStructure(
    PDWORD* ppdwCostMatrix,
    PDWORD  pFinalResult,
    DWORD   dwCount
    );

DWORD
VmDirGetIntraSiteTopology(
    PCSTR                           pszUserName,
    PCSTR                           pszPassword,
    PCSTR                           pszHostName,
    PCSTR                           pszSiteName,
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppCurTopology // Output
    )
{
    DWORD                           dwError = 0;
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology = NULL;
    PVMDIR_STRING_LIST              pAllServerList = NULL;
    PVMDIR_SERVER_INFO              pServerInfo = NULL;
    DWORD                           dwServerInfoCount = 0;
    PVMDIR_HA_SERVER_INFO*          ppHAServerList = NULL;
    DWORD                           dwHAListCount = 0;
    DWORD                           dwHAOnlineCount = 0;
    DWORD                           dwHAOfflineCount = 0;
    DWORD                           dwCnt = 0;

    // printf( "\t\t\t%s\t%d\n", __FUNCTION__, __LINE__); // For Debugging till final check-in
    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszSiteName) ||
        !ppCurTopology)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirGetServersOnSite(
                    pszHostName,
                    pszUserName,
                    pszPassword,
                    pszSiteName,
                    &pServerInfo,
                    &dwServerInfoCount
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetListFromServerInfo(
                    pServerInfo,
                    dwServerInfoCount,
                    &pAllServerList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetHAServerFromStringList(
                    pAllServerList,
                    pszUserName,
                    pszPassword,
                    &ppHAServerList,
                    &dwHAListCount,
                    &dwHAOnlineCount,
                    &dwHAOfflineCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * In this call servers formed in ppHAServerList are transferred to
     * pTopology Servers i.e. servers in ppHAServerList and pTopology are same
     * and can be referenced from one - another. That's the reason, there is
     * difference in way of freeing memory of ppHAServerList in case of normal
     * flow and error condition.
     */
    dwError = _VmDirFillIntraTopology(
                    bConsiderOfflineNodes,
                    ppHAServerList,
                    dwHAListCount,
                    dwHAOnlineCount,
                    dwHAOfflineCount,
                    &pTopology);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppCurTopology = pTopology;

cleanup:
    VmDirStringListFree(pAllServerList);

    if (pServerInfo)
    {
        for (dwCnt=0; dwCnt<dwServerInfoCount; dwCnt++)
        {
            VMDIR_SAFE_FREE_MEMORY(pServerInfo[dwCnt].pszServerDN);
        }
    }
    VMDIR_SAFE_FREE_MEMORY(pServerInfo);
    if (ppHAServerList)
    {
        VMDIR_SAFE_FREE_MEMORY(ppHAServerList);
    }

    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    _VmDirFreeHAServerList(
                ppHAServerList,
                dwHAListCount);
    VmDirFreeHATopologyData(pTopology);

    goto cleanup;
}

DWORD
VmDirGetInterSiteTopology(
    PCSTR                           pszUserName,
    PCSTR                           pszPassword,
    PCSTR                           pszHostName,
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppCurTopology // Output
    )
{
    DWORD   dwError = 0;

    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    BAIL_ON_VMDIR_ERROR(dwError); // For removing build issue till real code is plugged in
cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    goto cleanup;
}

DWORD
VmDirGetNewTopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppNewTopology // Output
    )
{
    DWORD                           dwError         = 0;
    PDWORD*                         ppdwCostMatrix  = NULL;
    PDWORD                          pFinalArray     = NULL;
    DWORD                           dwCost          = 0;
    PVMDIR_HA_REPLICATION_TOPOLOGY  pNewTopology    = NULL;

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    dwError = _VmDirCreateTopologyStructure(pTopology,
                                            &ppdwCostMatrix,
                                            &pFinalArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirFindFinalTopology(pTopology->dwConsiderListCnt,
                                      ppdwCostMatrix,
                                      pFinalArray,
                                      &dwCost);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCreateNewTopology(pTopology,
                                      pFinalArray,
                                      &pNewTopology);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppNewTopology = pNewTopology;
cleanup:
    _VmDirFreeTopologyStructure(ppdwCostMatrix,
                                pFinalArray,
                                pTopology->dwConsiderListCnt);
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    VmDirFreeHATopologyData(pNewTopology);

    goto cleanup;
}

DWORD
VmDirGetTopologyChanges(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pCurTopology,
    PVMDIR_HA_REPLICATION_TOPOLOGY  pNewTopology,
    PVMDIR_HA_TOPOLOGY_CHANGES* ppTopologyChanges //Output
    )
{
    DWORD   dwError = 0;

    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    BAIL_ON_VMDIR_ERROR(dwError); // For removing build issue till real code is plugged in
cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    goto cleanup;
}

DWORD
VmDirModifyLinks(
    PVMDIR_HA_TOPOLOGY_CHANGES  pTopologyChanges
    )
{
    DWORD   dwError = 0;

    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    BAIL_ON_VMDIR_ERROR(dwError); // For removing build issue till real code is plugged in
cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    goto cleanup;
}

VOID
VmDirFreeHATopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology
    )
{
    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pTopology)
    {
        _VmDirFreeHAServerList(
                        pTopology->ppOnlineList,
                        pTopology->dwOnlineListCnt);
        _VmDirFreeHAServerList(
                        pTopology->ppOfflineList,
                        pTopology->dwOfflineListCnt);
        VMDIR_SAFE_FREE_MEMORY(pTopology->ppConsiderList);// Not Freeing the List Contents
                                                          // as they would be freed by either of above 2
        VMDIR_SAFE_FREE_MEMORY(pTopology);
    }
}

VOID
VmDirFreeHAServer(
    PVMDIR_HA_SERVER_INFO   pServer
    )
{
    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pServer)
    {
        VMDIR_SAFE_FREE_MEMORY(pServer->pszHostName);

        VMDIR_SAFE_FREE_MEMORY(pServer->pszServerName);

        VMDIR_SAFE_FREE_MEMORY(pServer->pszSiteName);

        VmDirConnectionClose(pServer->pConnection);

        VMDIR_SAFE_FREE_MEMORY(pServer->ppPartnerList);

        VMDIR_SAFE_FREE_MEMORY(pServer);
    }

}

VOID
VmDirFreeHAChanges(
    PVMDIR_HA_TOPOLOGY_CHANGES  pChanges
    )
{
    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pChanges)
    {
        VMDIR_SAFE_FREE_MEMORY(pChanges);
    }
}

DWORD
_VmDirGetServersOnSite(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PCSTR               pszSiteName,
    PVMDIR_SERVER_INFO* ppServerInfo,   // output
    DWORD*              pdwNumServer    // output
)
{
    DWORD       dwError                   = 0;
    DWORD       i                         = 0;
    PSTR        pszDomain                 = NULL;
    LDAP*       pLd                       = NULL;
    DWORD       dwInfoCount               = 0;
    PSTR        pszServerName             = NULL;
    char        bufUPN[VMDIR_MAX_UPN_LEN] = {0};

    PINTERNAL_SERVER_INFO   pInternalServerInfo = NULL;
    PVMDIR_SERVER_INFO      pServerInfo         = NULL;

    // parameter check
    if (IsNullOrEmptyString (pszHostName) ||
        IsNullOrEmptyString (pszUserName) ||
        IsNullOrEmptyString (pszSiteName) ||
        pszPassword == NULL ||
        pdwNumServer == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get domain name
    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s", pszUserName, pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLd,
                                 pszServerName,
                                 bufUPN,
                                 pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    //get all vmdir servers in the site.
    dwError = VmDirGetServersInfoOnSite(
                            pLd,
                            pszSiteName,
                            pszServerName,
                            pszDomain,
                            &pInternalServerInfo,
                            &dwInfoCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppServerInfo)
    {
        dwError = VmDirAllocateMemory(
                dwInfoCount*sizeof(VMDIR_SERVER_INFO),
                (PVOID*)&pServerInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( i=0; i<dwInfoCount; i++)
        {
            dwError = VmDirAllocateStringA(
                    pInternalServerInfo[i].pszServerDN,
                    &(pServerInfo[i].pszServerDN));
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *ppServerInfo = pServerInfo;
    *pdwNumServer = dwInfoCount;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pInternalServerInfo);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    // unbind
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "_VmDirGetServersOnSite failed. Error[%d]\n",
            dwError
            );
    goto cleanup;
}

static
DWORD
_VmDirGetListFromServerInfo(
    PVMDIR_SERVER_INFO  pServerInfo,
    DWORD               dwServerInfoCount,
    PVMDIR_STRING_LIST* ppList
    )
{
    DWORD dwError = 0;
    DWORD dwCnt   = 0;
    PSTR  pszName = NULL;
    PVMDIR_STRING_LIST  pList = NULL;

    if (!pServerInfo || !ppList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pList, 16);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<dwServerInfoCount; dwCnt++)
    {
        // Getting Hostname of Domain Controllers
        dwError = VmDirDnLastRDNToCn(pServerInfo[dwCnt].pszServerDN,
                                    &pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pList, pszName);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszName = NULL;
    }

    *ppList = pList;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed. Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    VmDirStringListFree(pList);

    goto cleanup;
}

static
DWORD
_VmDirGetHAServerFromStringList(
    PVMDIR_STRING_LIST      pAllServerList,
    PCSTR                   pszUserName,
    PCSTR                   pszPassword,
    PVMDIR_HA_SERVER_INFO** pppList,
    PDWORD                  pdwListCount,
    PDWORD                  pdwOnlineCount,
    PDWORD                  pdwOfflineCount
    )
{
    DWORD   dwError        = 0;
    DWORD   dwCnt          = 0;
    DWORD   dwOnlineCount  = 0;
    DWORD   dwOfflineCount = 0;
    DWORD   dwListCount    = 0;

    PSTR    pszHost     = NULL;
    PSTR    pszServer   = NULL;
    PSTR    pszSite     = NULL;
    PSTR    pszDomain   = NULL;
    PSTR    pszHostName = NULL;

    PVMDIR_CONNECTION       pCon   = NULL;
    PVMDIR_HA_SERVER_INFO*  ppList = NULL;
    PVMDIR_HA_SERVER_INFO   pServer = NULL;

    BOOLEAN bIsNodeOffline = 0;

    if (!pAllServerList ||
        !pppList ||
        !pdwListCount ||
        !pdwOnlineCount ||
        !pdwOfflineCount ||
        !pAllServerList->dwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                    sizeof(PVMDIR_HA_SERVER_INFO)*pAllServerList->dwCount,
                    (PVOID*)&ppList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pAllServerList->dwCount; dwCnt++)
    {
        pszHost   = NULL;
        pszServer = NULL;
        pszSite   = NULL;
        pCon      = NULL;
        pServer   = NULL;

        VMDIR_SAFE_FREE_MEMORY(pszHostName);
        VMDIR_SAFE_FREE_MEMORY(pszDomain);

        dwError = VmDirAllocateStringA( pAllServerList->pStringList[dwCnt], &pszHostName );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetDomainName( pszHostName, &pszDomain );
        if (dwError == VMDIR_ERROR_SERVER_DOWN ||
            dwError == VMDIR_ERROR_CANNOT_CONNECT_VMDIR ||
            dwError == VMDIR_ERROR_TIMELIMIT_EXCEEDED)
        {
            dwError = 0; // Handling Offline Node Scenario
            bIsNodeOffline = 1;
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA( pszHostName, &pszHost );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Commented Code below because apparently it seems hostname and server name are same
        // For Offline node this is creating the issue as we will set it to NULL in case of error.

        // dwError = VmDirGetServerName( pszHostName, &pszServer);
        // VMDIR_HANDLE_OFFLINE_ERROR(dwError, pszServer);
        dwError = VmDirAllocateStringA(pszHostName, &pszServer);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (!bIsNodeOffline)
        {
            dwError = VmDirConnectionOpenByHost(
                            pszHostName,
                            pszDomain,
                            pszUserName,
                            pszPassword,
                            &pCon);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwOnlineCount += 1;
            dwError = VmDirGetSiteName(pCon, &pszSite);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            pCon = NULL;
            dwOfflineCount += 1;
            pszSite = NULL;
        }

        dwError = VmDirAllocateMemory(sizeof(VMDIR_HA_SERVER_INFO), (PVOID*)&pServer);
        BAIL_ON_VMDIR_ERROR(dwError);

        pServer->pszHostName    = pszHost;
        pServer->pszServerName  = pszServer;
        pServer->pConnection    = pCon;
        pServer->pszSiteName    = pszSite;
        pServer->ppPartnerList  = NULL;

        if (!pCon)
        {
            pServer->dwIdx = -1;
        }
        else
        {
            pServer->dwIdx = 0;
        }
        ppList[dwCnt] = pServer;
        dwListCount += 1;
    }
    dwError = _VmDirFillPartnersInList(ppList, dwListCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppList         = ppList;
    *pdwOnlineCount  = dwOnlineCount;
    *pdwOfflineCount = dwOfflineCount;
    *pdwListCount    = pAllServerList->dwCount;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszHostName);
    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed. Error[%d]",
        __FUNCTION__,
        dwError
        );
    VMDIR_SAFE_FREE_MEMORY(pszHost);
    VMDIR_SAFE_FREE_MEMORY(pszServer);
    VMDIR_SAFE_FREE_MEMORY(pszSite);
    VmDirConnectionClose(pCon);
    _VmDirFreeHAServerList(ppList, dwListCount);
    goto cleanup;

}

static
DWORD
_VmDirFillPartnersInList(
    PVMDIR_HA_SERVER_INFO*  ppList,
    DWORD                   dwCount
    )
{
    DWORD       dwError      = 0;
    DWORD       dwPartnerCnt = 0;
    PSTR*       ppszPartners = NULL;
    DWORD       dwCnt        = 0;
    DWORD       i            = 0;
    DWORD       j            = 0;
    DWORD       k            = 0;
    PBOOLEAN    pbFound      = NULL;
    LONG        lCompareRes  = 0;

    if (!ppList || !dwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (dwCnt = 0; dwCnt < dwCount; dwCnt++)
    {
        VmDirFreeStringArray(ppszPartners, dwPartnerCnt);
        dwPartnerCnt = 0;
        ppszPartners = NULL;

        if (ppList[dwCnt]->pConnection)
        {
            dwError = VmDirFindAllReplPartnerHostByPLd(
                                ppList[dwCnt]->pszServerName,
                                ppList[dwCnt]->pConnection->pszDomain,
                                ppList[dwCnt]->pConnection->pLd,
                                &ppszPartners,
                                &dwPartnerCnt);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (dwPartnerCnt)
            {
                dwError = VmDirAllocateMemory(sizeof(BOOLEAN)*dwPartnerCnt, (PVOID*)&pbFound);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwPartnerCnt, (PVOID*)&(ppList[dwCnt]->ppPartnerList));
                BAIL_ON_VMDIR_ERROR(dwError);

                (VOID) memset(pbFound, 0, sizeof(BOOLEAN)*dwPartnerCnt);

                k = 0;

                for (i=0; i<dwCount; i++)
                {
                    for (j=0; j<dwPartnerCnt; j++)
                    {
                        if (!pbFound[j])
                        {
                            lCompareRes = VmDirStringCompareA(ppList[i]->pszHostName,
                                                              ppszPartners[j],
                                                              0);
                            if(!lCompareRes)
                            {
                                ppList[dwCnt]->ppPartnerList[k] = ppList[i];
                                k++;
                                pbFound[j] = 1;
                                break;
                            }
                        }
                    }
                }
                VMDIR_SAFE_FREE_MEMORY(pbFound);
                ppList[dwCnt]->dwPartnerCnt = k;
            }
            else
            {
                ppList[dwCnt]->ppPartnerList = NULL;
                ppList[dwCnt]->dwPartnerCnt  = 0;
            }
        }
        else
        {
            ppList[dwCnt]->ppPartnerList = NULL;
            ppList[dwCnt]->dwPartnerCnt  = 0;
        }
    }

cleanup:
    VmDirFreeStringArray(ppszPartners, dwPartnerCnt);
    VMDIR_SAFE_FREE_MEMORY(pbFound);
    return dwError;

error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed. Error[%d]\n",
        __FUNCTION__,
        dwError
        );
    goto cleanup;
}

static
VOID
_VmDirFreeHAServerList(
    PVMDIR_HA_SERVER_INFO*  ppList,
    DWORD                   dwCount
    )
{
    DWORD   dwCnt = 0;

    if (ppList)
    {
        // printf("%s Count is %d\n", __FUNCTION__, dwCount); // For Debugging till final check-in
        for (dwCnt = 0; dwCnt < dwCount; dwCnt++)
        {
            if (ppList[dwCnt])
            {
                VmDirFreeHAServer(ppList[dwCnt]);
            }
        }
        VMDIR_SAFE_FREE_MEMORY(ppList);
    }
}

static
DWORD
_VmDirFillIntraTopology(
    BOOLEAN                         bConsiderOfflineNodes,
    PVMDIR_HA_SERVER_INFO*          ppServerList,
    DWORD                           dwListCount,
    DWORD                           dwOnlineCount,
    DWORD                           dwOfflineCount,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppTopology
    )
{

    DWORD   dwError         = 0;
    DWORD   dwConsiderCnt   = 0;
    DWORD   dwCnt           = 0;
    DWORD   dwPosOn         = 0;
    DWORD   dwPosOff        = 0;
    DWORD   dwPosCon        = 0;
    BOOLEAN bOffline        = bConsiderOfflineNodes;

    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology = NULL;

    if (!ppServerList || !ppTopology)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VMDIR_HA_REPLICATION_TOPOLOGY), (PVOID*)&pTopology);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwOnlineCount)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwOnlineCount, (PVOID*)&(pTopology->ppOnlineList));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pTopology->ppOnlineList = NULL;
        pTopology->dwOnlineListCnt = 0;
    }

    if (dwOfflineCount)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwOfflineCount, (PVOID*)&(pTopology->ppOfflineList));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pTopology->ppOfflineList = NULL;
        pTopology->dwOfflineListCnt = 0;
    }

    if (bOffline)
    {
        dwConsiderCnt = dwListCount;
    }
    else
    {
        dwConsiderCnt = dwOnlineCount;
    }

    if (dwConsiderCnt)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwConsiderCnt, (PVOID*)&(pTopology->ppConsiderList));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pTopology->ppConsiderList = NULL;
        pTopology->dwConsiderListCnt = 0;
    }

    dwPosOn = dwPosOff = dwPosCon = 0;

    for (dwCnt=0; dwCnt < dwListCount; dwCnt++)
    {
        if (ppServerList[dwCnt])
        {
            if (ppServerList[dwCnt]->pConnection)
            {
                if (pTopology->ppOnlineList)
                {
                    pTopology->ppOnlineList[dwPosOn] = ppServerList[dwCnt];
                    dwPosOn++;
                }
                if (pTopology->ppConsiderList)
                {
                    pTopology->ppConsiderList[dwPosCon] = ppServerList[dwCnt];
                    pTopology->ppConsiderList[dwPosCon]->dwIdx = dwPosCon;
                    dwPosCon++;
                }
            }
            else
            {
                if (pTopology->ppOfflineList)
                {
                    pTopology->ppOfflineList[dwPosOff] = ppServerList[dwCnt];
                    dwPosOff++;
                }
                if (bOffline && pTopology->ppConsiderList)
                {
                    pTopology->ppConsiderList[dwPosCon] = ppServerList[dwCnt];
                    pTopology->ppConsiderList[dwPosCon]->dwIdx = dwPosCon;
                    dwPosCon++;
                }
            }
        }
    }

    pTopology->dwConsiderListCnt = dwPosCon;
    pTopology->dwOnlineListCnt   = dwPosOn;
    pTopology->dwOfflineListCnt  = dwPosOff;

    *ppTopology = pTopology;

cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
        VMDIR_LOG_MASK_ALL,
        "%s failed. Error[%d]",
        __FUNCTION__,
        dwError
        );
    VmDirFreeHATopology(pTopology);
    goto cleanup;
}

static
DWORD
_VmDirCreateTopologyStructure(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology,
    PDWORD**                        pppdwCostMatrix,
    PDWORD*                         ppFinalResult
    )
{
    DWORD   dwError = 0;
    PDWORD* ppdwCostMatrix          = NULL;
    PDWORD  pdwFinalResult            = NULL;// list that will mantain the ring
                                            // start - node - node - end
    PVMDIR_HA_SERVER_INFO* ppList   = NULL;
    DWORD   dwCnt                   = 0;
    DWORD   dwCount                 = 0;
    DWORD   i                       = 0;
    DWORD   j                       = 0;
    DWORD   src                     = 0;
    DWORD   tgt                     = 0;
    DWORD   dwMinStartIdx           = 0;
    DWORD   dwMinStartCount         = 1999999999;
    DWORD   dw1WayReplCost          = 200;
    DWORD   dwNoNewReplCost         = 50;
    DWORD   dwOfflineNoNewReplCost  = 0; // Reason Offline Node has zero cost
                                        // if edge exist and make algorithm use this
                                        // instead of Online node one

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    if (!pTopology ||
        !pppdwCostMatrix ||
        !ppFinalResult ||
        !pTopology->ppConsiderList ||
        !pTopology->dwConsiderListCnt)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    ppList  = pTopology->ppConsiderList;
    dwCount = pTopology->dwConsiderListCnt;

    dwError = VmDirAllocateMemory(sizeof(PDWORD)*dwCount, (PVOID*)&ppdwCostMatrix);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<dwCount; i++)
    {
        dwError = VmDirAllocateMemory(sizeof(DWORD)*dwCount, (PVOID*)&ppdwCostMatrix[i]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i<dwCount; i++)
    {
        for (j=0; j<dwCount; j++)
        {
            ppdwCostMatrix[i][j] = ppdwCostMatrix[j][i] = dw1WayReplCost;
        }
    }

    for (i=0; i<dwCount; i++)
    {
        if (ppList[i])
        {
            src = ppList[i]->dwIdx;
            dwCnt = ppList[i]->dwPartnerCnt;
            if (dwCnt)
            {
                if (dwCnt < dwMinStartCount)
                {
                    dwMinStartIdx   = i;
                    dwMinStartCount = dwCnt;
                }
                for(j=0; j<dwCnt; j++)
                {
                    if (ppList[i]->ppPartnerList[j] && ppList[i]->ppPartnerList[j]->dwIdx != (DWORD)-1)
                    {
                        tgt = ppList[i]->ppPartnerList[j]->dwIdx;
                        if (ppList[i]->pConnection)
                        {
                            ppdwCostMatrix[src][tgt] = dwNoNewReplCost;
                        }
                        else
                        {
                            ppdwCostMatrix[src][tgt] = dwOfflineNoNewReplCost;
                        }
                    }
                }
            }
        }
    }

    dwError = VmDirAllocateMemory(sizeof(DWORD)*(dwCount+1), (PVOID*)&pdwFinalResult); // +1 because of ring
    BAIL_ON_VMDIR_ERROR(dwError);

    pdwFinalResult[0]       = dwMinStartIdx; // start point
    pdwFinalResult[dwCount] = dwMinStartIdx; // end point

    *pppdwCostMatrix = ppdwCostMatrix;
    *ppFinalResult   = pdwFinalResult;

cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    for (i=0; i<dwCount; i++)
    {
        if(ppdwCostMatrix && ppdwCostMatrix[i])
        {
            VMDIR_SAFE_FREE_MEMORY(ppdwCostMatrix[i]);
        }
    }
    if (ppdwCostMatrix)
    {
        VMDIR_SAFE_FREE_MEMORY(ppdwCostMatrix);
    }
    if (pdwFinalResult)
    {
        VMDIR_SAFE_FREE_MEMORY(pdwFinalResult);
    }
    goto cleanup;
}

static
DWORD
_VmDirFindFinalTopology(
    DWORD   dwNodeCount,
    PDWORD* ppdwCostMatrix,
    PDWORD  pdwFinalResult, // Output
    PDWORD  pdwCost // Output
    )
{
    DWORD   dwError = 0;

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    if (!ppdwCostMatrix || !pdwFinalResult || !pdwCost)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirFindNNA(dwNodeCount, ppdwCostMatrix, pdwFinalResult, pdwCost);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    goto cleanup;
}

static
DWORD
_VmDirFindNNA(
    DWORD   dwNodeCount,
    PDWORD* ppdwCostMatrix,
    PDWORD  pdwFinalResult, // Output
    PDWORD  pdwCost // Output
    )
{
    DWORD       dwError = 0;
    DWORD       i       = 0;
    PBOOLEAN    pMark   = NULL;
    DWORD       dwMarkCnt   = 1;
    DWORD       dwMinIdx    = 0;
    DWORD       dwMinCost   = 1999999999;
    DWORD       dwTmpCost   = 1999999999;
    DWORD       dwSrc       = 0;

    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    if (!ppdwCostMatrix || !pdwFinalResult || !pdwCost || !dwNodeCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(BOOLEAN)*dwNodeCount,(PVOID*)&pMark);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i=0; i<dwNodeCount; i++)
    {
        pMark[i] = 0;
    }

    dwSrc = pdwFinalResult[0];
    pMark[dwSrc] = 1;

    *pdwCost = 0;

    while(dwMarkCnt != dwNodeCount)
    {
        dwMinIdx  = -1;
        dwMinCost = 1999999999;
        for(i=0; i<dwNodeCount; i++)
        {
            if (!pMark[i])
            {
                dwTmpCost = ppdwCostMatrix[dwSrc][i] + ppdwCostMatrix[i][dwSrc];
                if (dwTmpCost < dwMinCost)
                {
                    dwMinCost = dwTmpCost;
                    dwMinIdx  = i;
                }
            }
        }
        if (dwMinIdx == -1)
        {
            dwError = VMDIR_ERROR_INVALID_RESULT;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        pMark[dwMinIdx] = 1;
        pdwFinalResult[dwMarkCnt++] = dwMinIdx;
        dwSrc = dwMinIdx;
        *pdwCost += dwMinCost;
    }
    *pdwCost +=  (ppdwCostMatrix[pdwFinalResult[dwNodeCount-1]][pdwFinalResult[dwNodeCount]]);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pMark);
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    goto cleanup;
}

static
DWORD
_VmDirCopyHAServer(
    PVMDIR_HA_SERVER_INFO   pSrcServer,
    DWORD                   dwPartnerCnt,
    PVMDIR_HA_SERVER_INFO*  ppDestServer
    )
{
    DWORD   dwError = 0;

    PVMDIR_HA_SERVER_INFO   pServer = NULL;

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    dwError = VmDirAllocateMemory(sizeof(VMDIR_HA_SERVER_INFO), (PVOID*)&pServer);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrcServer->pszHostName, &(pServer->pszHostName));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pSrcServer->pszServerName, &(pServer->pszServerName));
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pSrcServer->pszSiteName)
    {
        dwError = VmDirAllocateStringA(pSrcServer->pszSiteName, &(pServer->pszSiteName));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pServer->pszSiteName = NULL;
    }

    pServer->pConnection = pSrcServer->pConnection;
    pSrcServer->pConnection = NULL;

    if (dwPartnerCnt)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwPartnerCnt,(PVOID*)&(pServer->ppPartnerList));
        BAIL_ON_VMDIR_ERROR(dwError);
        pServer->dwPartnerCnt = dwPartnerCnt;
    }
    else
    {
        pServer->ppPartnerList  = NULL;
        pServer->dwPartnerCnt   = 0;
    }

    pServer->dwIdx = pSrcServer->dwIdx;

    *ppDestServer = pServer;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    if (pServer)
    {
        VmDirFreeHAServerInfo(pServer);
    }
    goto cleanup;
}

static
DWORD
_VmDirCreateNewTopology(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology,
    PDWORD                          pFinalResult,
    PVMDIR_HA_REPLICATION_TOPOLOGY* ppNewTopology
    )
{
    DWORD   dwError         = 0;
    DWORD   i               = 0;
    DWORD   j               = 0;
    DWORD   k               = 0;
    DWORD   dwCount         = 0;
    DWORD   dwOnlineCnt     = 0;
    DWORD   dwOfflineCnt    = 0;

    PVMDIR_HA_REPLICATION_TOPOLOGY pNewTopology = NULL;

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    if (!pTopology ||
        !pFinalResult ||
        !ppNewTopology ||
        !pTopology->ppConsiderList ||
        !pTopology->dwConsiderListCnt)
    {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwCount = pTopology->dwConsiderListCnt;

    dwError = VmDirAllocateMemory(sizeof(VMDIR_HA_REPLICATION_TOPOLOGY), (PVOID*)&pNewTopology);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwCount, (PVOID*)&(pNewTopology->ppConsiderList));
    BAIL_ON_VMDIR_ERROR(dwError);

    pNewTopology->dwConsiderListCnt = dwCount;

    for (i=0; i<dwCount; i++)
    {
        dwError = _VmDirCopyHAServer(pTopology->ppConsiderList[i], 2, &(pNewTopology->ppConsiderList[i])); // 2 because ring topology has 2 partners
        BAIL_ON_VMDIR_ERROR(dwError);
        if (pNewTopology->ppConsiderList[i]->pConnection)
        {
            dwOnlineCnt += 1;
        }
        else
        {
            dwOfflineCnt += 1;
        }
    }
    // printf("OnlineCnt: %u, OfflineCnt: %u\n",dwOnlineCnt,dwOfflineCnt);
    if (dwOnlineCnt)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwOnlineCnt, (PVOID*)&(pNewTopology->ppOnlineList));
        BAIL_ON_VMDIR_ERROR(dwError);
        pNewTopology->dwOnlineListCnt = dwOnlineCnt;
    }

    if (dwOfflineCnt)
    {
        dwError = VmDirAllocateMemory(sizeof(PVMDIR_HA_SERVER_INFO)*dwOfflineCnt, (PVOID*)&(pNewTopology->ppOfflineList));
        BAIL_ON_VMDIR_ERROR(dwError);
        pNewTopology->dwOfflineListCnt = dwOfflineCnt;
    }

    // Setting Partner List for start node
    (pNewTopology->ppConsiderList[pFinalResult[0]])->ppPartnerList[0] = pNewTopology->ppConsiderList[pFinalResult[1]]; // second node in ring
    (pNewTopology->ppConsiderList[pFinalResult[0]])->ppPartnerList[1] = pNewTopology->ppConsiderList[pFinalResult[dwCount-1]]; // second last node in ring, last node is itself

    for (i=1; i<dwCount; i++)
    {
         j = pFinalResult[i-1];
         k = pFinalResult[i+1];
         (pNewTopology->ppConsiderList[pFinalResult[i]])->ppPartnerList[0] = pNewTopology->ppConsiderList[j];
         (pNewTopology->ppConsiderList[pFinalResult[i]])->ppPartnerList[1] = pNewTopology->ppConsiderList[k];
    }

    j = 0;
    k = 0;

    for (i=0; i<dwCount; i++)
    {
        if(pNewTopology->ppConsiderList[i]->pConnection)
        {
            pNewTopology->ppOnlineList[j] = pNewTopology->ppConsiderList[i];
            j++;
        }
        else
        {
            pNewTopology->ppOfflineList[k] = pNewTopology->ppConsiderList[i];
            k++;
        }
    }

    *ppNewTopology = pNewTopology;
cleanup:
    return dwError;
error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    if (pNewTopology)
    {
        if (pNewTopology->ppConsiderList)
        {
            _VmDirFreeHAServerList(pNewTopology->ppConsiderList, pNewTopology->dwConsiderListCnt);
        }

        if (pNewTopology->ppOnlineList)
        {
            VMDIR_SAFE_FREE_MEMORY(pNewTopology->ppOnlineList);
        }

        if (pNewTopology->ppOfflineList)
        {
            VMDIR_SAFE_FREE_MEMORY(pNewTopology->ppOfflineList);
        }

        VMDIR_SAFE_FREE_MEMORY(pNewTopology);

    }
    goto cleanup;
}

static
VOID
_VmDirFreeTopologyStructure(
    PDWORD* ppdwCostMatrix,
    PDWORD  pFinalResult,
    DWORD   dwCount
    )
{
    DWORD   i = 0;

    // printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in

    for (i=0; i<dwCount; i++)
    {
        if(ppdwCostMatrix && ppdwCostMatrix[i])
        {
            VMDIR_SAFE_FREE_MEMORY(ppdwCostMatrix[i]);
        }
    }
    if (ppdwCostMatrix)
    {
        VMDIR_SAFE_FREE_MEMORY(ppdwCostMatrix);
    }
    if (pFinalResult)
    {
        VMDIR_SAFE_FREE_MEMORY(pFinalResult);
    }
}
