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

DWORD
VmDirGetIntraSiteTopology(
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszHostName,
    PCSTR   pszSiteName,
    PBOOLEAN    pbConsiderOfflineNodes,
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
VmDirGetInterSiteTopology(
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszHostName,
    PBOOLEAN    pbConsiderOfflineNodes,
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
    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pTopology)
    {
        VMDIR_SAFE_FREE_MEMORY(pTopology);
    }
}

VOID
VmDirFreeHAServer(
    PVMDIR_HA_SERVER_INFO   pServer
    )
{
    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pServer)
    {
        VMDIR_SAFE_FREE_MEMORY(pServer);
    }

}

VOID
VmDirFreeHAChanges(
    PVMDIR_HA_TOPOLOGY_CHANGES  pChanges
    )
{
    printf( "\t\t\t%s\n", __FUNCTION__); // For Debugging till final check-in
    if (pChanges)
    {
        VMDIR_SAFE_FREE_MEMORY(pChanges);
    }
}

