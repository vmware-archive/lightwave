/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

DWORD
VmDirSchemaReplStateCreate(
    PSTR                        pszHostName,
    PSTR                        pszDomainName,
    PVDIR_SCHEMA_REPL_STATE*    ppReplState
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    if (IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszDomainName) ||
        !ppReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_SCHEMA_REPL_STATE),
            (PVOID*)&pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszHostName, &pReplState->pszHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszDomainName, &pReplState->pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppReplState = pReplState;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeSchemaReplState(pReplState);
    goto cleanup;
}

DWORD
VmDirSchemaReplStateCheck(
    PVDIR_SCHEMA_REPL_STATE pReplState,
    PVDIR_LDAP_SCHEMA       pTargetSchema
    )
{
    DWORD   dwError = 0;
    LDAP*   pLd = NULL;
    PVDIR_LDAP_SCHEMA   pTreeSchema = NULL;
    PVDIR_LDAP_SCHEMA_DIFF  pTreeDiff = NULL;

    if (!pReplState || !pTargetSchema)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pReplState->bCheckInitiated = TRUE;
    pReplState->bCheckSucceeded = FALSE;

    dwError = VmDirConnectLDAPServerWithMachineAccount(
            pReplState->pszHostName,
            pReplState->pszDomainName,
            &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get tree and compare with local
    dwError = VmDirLdapSchemaInit(&pTreeSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadRemoteSchema(pTreeSchema, pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaGetDiff(pTreeSchema, pTargetSchema, &pTreeDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    // store comparison data
    pReplState->dwAttrMissingInTree =
            VmDirLinkedListGetSize(pTreeDiff->attrToAdd);
    pReplState->dwAttrMismatchInTree =
            VmDirLinkedListGetSize(pTreeDiff->attrToModify);
    pReplState->dwClassMissingInTree =
            VmDirLinkedListGetSize(pTreeDiff->classToAdd);
    pReplState->dwClassMismatchInTree =
            VmDirLinkedListGetSize(pTreeDiff->classToModify);

    pReplState->bTreeInSync =
            pReplState->dwAttrMissingInTree == 0 &&
            pReplState->dwAttrMismatchInTree == 0 &&
            pReplState->dwClassMissingInTree == 0 &&
            pReplState->dwClassMismatchInTree == 0;

    pReplState->bCheckSucceeded = TRUE;

cleanup:
    VmDirLdapUnbind(&pLd);
    VmDirFreeLdapSchemaDiff(pTreeDiff);
    VmDirFreeLdapSchema(pTreeSchema);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirSchemaReplStateParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_SCHEMA_REPL_STATE*    ppReplState
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PLW_HASHMAP pStateInfo = NULL;
    struct berval** ppValues = NULL;
    PSTR    pszKey = NULL;
    PSTR    pszVal = NULL;
    PSTR    pszTmp = NULL;
    PSTR    pszHostName = NULL;
    PSTR    pszDomainName = NULL;
    PVDIR_SCHEMA_REPL_STATE pReplState = NULL;

    if (!pLd || !pEntry || !ppReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlCreateHashMap(
            &pStateInfo,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppValues = ldap_get_values_len(pLd, pEntry, ATTR_SERVER_RUNTIME_STATUS);

    for (i = 0; i < ldap_count_values_len(ppValues); i++)
    {
        dwError = VmDirAllocateStringA(ppValues[i]->bv_val, &pszKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszTmp = VmDirStringChrA(pszKey, ':');
        dwError = pszTmp == NULL ? VMDIR_ERROR_BAD_ATTRIBUTE_DATA : 0;
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszTmp + 2, &pszVal);
        BAIL_ON_VMDIR_ERROR(dwError);

        *pszTmp = '\0';
        dwError = LwRtlHashMapInsert(pStateInfo, pszKey, pszVal, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszKey = NULL;
        pszVal = NULL;
    }

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszHostName,
            SCHEMA_REPL_STATUS_HOST_NAME);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszDomainName,
            SCHEMA_REPL_STATUS_DOMAIN_NAME);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSchemaReplStateCreate(
            pszHostName, pszDomainName, &pReplState);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_CHECK_INITIATED);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->bCheckInitiated =
            VmDirStringCompareA(pszTmp, "TRUE", FALSE) == 0;

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_CHECK_SUCCEEDED);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->bCheckSucceeded =
            VmDirStringCompareA(pszTmp, "TRUE", FALSE) == 0;

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_TREE_IN_SYNC);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->bTreeInSync =
            VmDirStringCompareA(pszTmp, "TRUE", FALSE) == 0;

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_ATTR_MISSING_IN_TREE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->dwAttrMissingInTree = VmDirStringToIA(pszTmp);

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_ATTR_MISMATCH_IN_TREE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->dwAttrMismatchInTree = VmDirStringToIA(pszTmp);

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_CLASS_MISSING_IN_TREE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->dwClassMissingInTree = VmDirStringToIA(pszTmp);

    dwError = LwRtlHashMapFindKey(
            pStateInfo,
            (PVOID*)&pszTmp,
            SCHEMA_REPL_STATUS_CLASS_MISMATCH_IN_TREE);
    BAIL_ON_VMDIR_ERROR(dwError);

    pReplState->dwClassMismatchInTree = VmDirStringToIA(pszTmp);

    *ppReplState = pReplState;

cleanup:
    LwRtlHashMapClear(pStateInfo, VmDirNoopHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pStateInfo);
    ldap_value_free_len(ppValues);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeSchemaReplState(pReplState);
    VMDIR_SAFE_FREE_MEMORY(pszKey);
    VMDIR_SAFE_FREE_MEMORY(pszVal);
    goto cleanup;
}

DWORD
VmDirSchemaReplStateLog(
    PVDIR_SCHEMA_REPL_STATE pReplState
    )
{
    DWORD   dwError = 0;

    if (!pReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pReplState->bTreeInSync)
    {
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                "%s: %s schema in sync",
                __FUNCTION__,
                pReplState->pszHostName);
    }
    else
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                "%s: %s schema tree out of sync "
                "(%d attributes missing, "
                "%d attributes mismatch,"
                "%d classes missing,"
                "$d classes mismatch)",
                __FUNCTION__,
                pReplState->pszHostName,
                pReplState->dwAttrMissingInTree,
                pReplState->dwAttrMismatchInTree,
                pReplState->dwClassMissingInTree,
                pReplState->dwClassMismatchInTree);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

VOID
VmDirFreeSchemaReplState(
    PVDIR_SCHEMA_REPL_STATE pReplState
    )
{
    if (pReplState)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplState->pszHostName);
        VMDIR_SAFE_FREE_MEMORY(pReplState->pszDomainName);
        VMDIR_SAFE_FREE_MEMORY(pReplState);
    }
}
