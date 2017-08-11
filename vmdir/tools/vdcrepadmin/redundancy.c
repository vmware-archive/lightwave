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
 * Filename: redundancy.c
 *
 * Abstract:
 *
 * vdcrepadmin's HA management feature entry point
 *
 */

#include "includes.h"

VOID
_PrintOptionSelected(
    BOOLEAN bNoInteraction,
    BOOLEAN bIncludeOffline,
    PCSTR   pszSiteName
    )
{
    printf("\t\t--------------------Configuration--------------------\n\n");
    if (bNoInteraction)
    {
        printf("\tRunning the tool in No Interaction Mode\n\n");
    }

    if (bIncludeOffline)
    {
        printf("\tTool is allowed to consider non-reachable servers for creating topology\n\n");
    }

    if (pszSiteName)
    {
        printf("\tTool is fixing Intra-Site Region with site-name as %s\n\n",pszSiteName);
    }
    else
    {
        printf("\tTool is fixing Inter-Site Region\n\n");
    }
    printf("\t\t-----------------------------------------------------\n\n");
}

DWORD
_PrintHAServerList(
    PVMDIR_HA_SERVER_INFO* ppList,
    DWORD   dwCount
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt   = 0;
    DWORD   dwPCnt  = 0;

    if (!ppList || !dwCount)
    {
        printf("\t\tInvalid Parameter to Print List\n\n");
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (dwCnt=0; dwCnt<dwCount; dwCnt++)
    {
        if (!(ppList[dwCnt]) || !(ppList[dwCnt]->pszHostName))
        {
            dwError = VMDIR_ERROR_INVALID_RESULT;
            break;
        }
        printf("\t\t%s\n\n",ppList[dwCnt]->pszHostName);
        if (ppList[dwCnt]->dwPartnerCnt)
        {
            printf("\t\t\tPartner of Server are as follow:\n");
            for (dwPCnt=0; dwPCnt<ppList[dwCnt]->dwPartnerCnt; dwPCnt++)
            {
                printf("\t\t\t\t%s\n",ppList[dwCnt]->ppPartnerList[dwPCnt]->pszHostName);
            }
        }
        else
        {
            printf("\t\t\tNo Partners of this server were found.\n");
        }
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    printf("%s failed. Error code [%d]\n",
           __FUNCTION__,
           dwError);
    goto cleanup;
}

DWORD
_PrintTopologyServers(
    PVMDIR_HA_REPLICATION_TOPOLOGY  pTopology
    )
{
    DWORD   dwError = 0;

    if (!pTopology)
    {
        printf("\n\tSomething Terribly is wrong! Received NULL Topology pointer\n\n");
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (!(pTopology->ppConsiderList) || !(pTopology->dwConsiderListCnt))
    {
        printf("\n\tNo Appropriate Servers Found\n\n");
    }
    else
    {
        printf("\n\tConsidered Servers are as follow:\n\n");
        dwError = _PrintHAServerList(
                        pTopology->ppConsiderList,
                        pTopology->dwConsiderListCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (!(pTopology->ppOnlineList) || !(pTopology->dwOnlineListCnt))
    {
        printf("\n\tNo Online Servers Found\n\n");
    }
    else
    {
        printf("\n\tOnline Servers are as follow:\n\n");
        dwError = _PrintHAServerList(
                        pTopology->ppOnlineList,
                        pTopology->dwOnlineListCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (!(pTopology->ppOfflineList) || !(pTopology->dwOfflineListCnt))
    {
        printf("\n\tNo Offline Servers Found\n\n");
    }
    else
    {
        printf("\n\tOffline Servers are as follow:\n\n");
        dwError = _PrintHAServerList(
                        pTopology->ppOfflineList,
                        pTopology->dwOfflineListCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
cleanup:
    return dwError;

error:
    printf("[%s] failed. Error code [%d]\n",
           __FUNCTION__,
           dwError);
    goto cleanup;

}

DWORD
_PrintTopologyChanges(
    PVMDIR_HA_TOPOLOGY_CHANGES  pChanges
    )
{
    DWORD   dwError = 0;

    if (!pChanges)
    {
        printf("\n\tSomething Terribly is wrong! Received NULL pointer\n\n");
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (!(pChanges->ppAddLinkList) || !(pChanges->dwAddListCnt))
    {
        printf("\n\tNo Links to be Added\n\n");
    }
    else
    {
        printf("\n\tLinks to be added are as follow:\n\n");
        dwError = _PrintHAServerList(
                        pChanges->ppAddLinkList,
                        pChanges->dwAddListCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    if (!(pChanges->ppDelLinkList) || !(pChanges->dwDelListCnt))
    {
        printf("\n\tNo Links to be Deleted\n\n");
    }
    else
    {
        printf("\n\tLinks to be deleted are as follow:\n\n");
        dwError = _PrintHAServerList(
                        pChanges->ppDelLinkList,
                        pChanges->dwDelListCnt);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
cleanup:
    return dwError;

error:
    printf("[%s] failed. Error code [%d]\n",
           __FUNCTION__,
           dwError);
    goto cleanup;
}

DWORD
_PromptForContinuation()
{
    CHAR    pszContinueStr[1 + 1] = {""}; // 1- for '0' or '1', 1- for '\0' character
    DWORD   dwContinueVal = 1;

    // read Integer to continue from stdin
    VmDirReadString(
        "Enter 1 to Continue, 0 to Abort:",
        pszContinueStr,
        sizeof(pszContinueStr),
        FALSE);
    dwContinueVal = (DWORD)VmDirStringToIA(pszContinueStr);
    if (dwContinueVal == 0)
    {
        printf("\tUser Decided to Abort and Therefore Aborting Task\n\n");
    }
    else if (dwContinueVal != 1)
    {
        printf("\tUser provided unrecognized input and Therefore Continuing Task\n\n");
        dwContinueVal = 1;
    }

    return dwContinueVal;
}

DWORD
VmDirEnableRedundantTopology(
    BOOLEAN             bNoInteraction,
    BOOLEAN             bIncludeOffline,
    PCSTR               pszSrcHostName,
    PCSTR               pszSrcPort,
    PCSTR               pszSrcUserName,
    PCSTR               pszSrcPassword,
    PCSTR               pszSiteName
    )
{
    DWORD   dwError = 0;
    DWORD   dwContinueVal = 1;
    CHAR    flsh = '\0';
    PVMDIR_HA_REPLICATION_TOPOLOGY  pCurTopology = NULL;
    PVMDIR_HA_REPLICATION_TOPOLOGY  pNewTopology = NULL;
    PVMDIR_HA_TOPOLOGY_CHANGES  pTopologyChanges = NULL;

    if (IsNullOrEmptyString(pszSrcHostName) ||
        IsNullOrEmptyString(pszSrcUserName) ||
        IsNullOrEmptyString(pszSrcPassword))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // VOID function, no return type to check
    _PrintOptionSelected(
        bNoInteraction,
        bIncludeOffline,
        pszSiteName
        );

    if (pszSiteName)
    {
        dwError = VmDirGetCurrentTopologyAtSite(
                            pszSrcUserName,
                            pszSrcPassword,
                            pszSrcHostName,
                            pszSiteName,
                            bIncludeOffline,
                            &pCurTopology);
    }
    else
    {
        dwError = VmDirGetCurrentGlobalTopology(
                            pszSrcUserName,
                            pszSrcPassword,
                            pszSrcHostName,
                            bIncludeOffline,
                            &pCurTopology);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("\t\t----------------------Current Topology-----------------\n");
    dwError = _PrintTopologyServers(pCurTopology);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("\t\t-------------------------------------------------------\n");

    if (!bNoInteraction)
    {
        dwContinueVal = _PromptForContinuation();
        if (!dwContinueVal)
        {
            goto cleanup;
        }
    }

    dwError = VmDirGetProposedTopology(
                    pCurTopology,
                    &pNewTopology);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("\t\t----------------------New Topology-----------------\n");
    dwError = _PrintTopologyServers(pNewTopology);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("\t\t---------------------------------------------------\n");

    if (!bNoInteraction)
    {
        scanf("%c", &flsh); // To read '\n' which was entered during last prompt
        dwContinueVal = _PromptForContinuation();
        if (!dwContinueVal)
        {
            goto cleanup;
        }
    }

    dwError = VmDirGetChangesInTopology(
                        pCurTopology,
                        pNewTopology,
                        &pTopologyChanges);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("\t\t----------------------Proposed Topology Changes-----------------\n");
    // dwError = _PrintTopologyChanges(pTopologyChanges);// Uncomment after Implementing respective API
    // BAIL_ON_VMDIR_ERROR(dwError);
    printf("\t\t----------------------------------------------------------------\n");

    if (!bNoInteraction)
    {
        scanf("%c", &flsh); // To read '\n' which was entered during last prompt
        dwContinueVal = _PromptForContinuation();
        if (!dwContinueVal)
        {
            goto cleanup;
        }
    }

    dwError = VmDirApplyTopologyChanges(pTopologyChanges);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeHATopologyData(pCurTopology);
    VmDirFreeHATopologyData(pNewTopology);
    VmDirFreeHATopologyChanges(pTopologyChanges);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed with Error code [%d]\n",
            __FUNCTION__,
            dwError);
    goto cleanup;
}
