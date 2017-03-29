/*
 * Copyright © 2016-2017 VMware, Inc.  All Rights Reserved.
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
VdcSchemaReadPassword(
    PSTR*   ppszPassword
    )
{
    DWORD   dwError = 0;
    CHAR    szPassword[VMDIR_MAX_PWD_LEN+1] = {0};

    if (!ppszPassword)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VmDirReadString(NULL, szPassword, VMDIR_MAX_PWD_LEN+1, TRUE);

    dwError = VmDirAllocateStringA(szPassword, ppszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    memset(szPassword, 0, strlen(szPassword));
    return dwError;

error:
    goto cleanup;
}

VOID
VmDirSchemaPrintSyntaxMap(
    PLW_HASHMAP pSyntaxMap
    )
{
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    printf("\nList of supported syntaxes:\n");

    while (LwRtlHashMapIterate(pSyntaxMap, &iter, &pair))
    {
        PSTR pszSxOid = (PSTR)pair.pKey;
        PSTR pszSxDesc = (PSTR)pair.pValue;
        printf("%s (%s)\n", pszSxOid, pszSxDesc);
    }
}

VOID
VmDirSchemaPrintDiff(
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff
    )
{
    PVDIR_LINKED_LIST_NODE          pNode = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pDiff = NULL;
    PVDIR_LDAP_MOD                  pMod = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    DWORD   i = 0;

    static PCSTR ppszModOp[4] = { "add", "delete", "replace", NULL };

    printf("\nDiff to patch:\n");

    pNode = pSchemaDiff->attrToAdd->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;
        printf("\n");
        printf("dn: %s\n", pDiff->pszDN);
        printf("changetype: add\n");

        LwRtlHashMapResetIter(&iter);
        while (LwRtlHashMapIterate(pDiff->mods, &iter, &pair))
        {
            pMod = (PVDIR_LDAP_MOD)pair.pValue;
            for (i = 0; pMod->pVals->pStringList[i]; i++)
            {
                printf("%s: %s\n", pMod->pszType, pMod->pVals->pStringList[i]);
            }
        }
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->attrToModify->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;
        printf("\n");
        printf("dn: %s\n", pDiff->pszDN);
        printf("changetype: modify\n");

        LwRtlHashMapResetIter(&iter);
        while (LwRtlHashMapIterate(pDiff->mods, &iter, &pair))
        {
            pMod = (PVDIR_LDAP_MOD)pair.pValue;
            printf("%s: %s\n", ppszModOp[pMod->op], pMod->pszType);
            for (i = 0; pMod->pVals->pStringList[i]; i++)
            {
                printf("%s: %s\n", pMod->pszType, pMod->pVals->pStringList[i]);
            }
            printf("%s", iter.Inner.pNext ? "-\n" : "");
        }
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToAdd->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;
        printf("\n");
        printf("dn: %s\n", pDiff->pszDN);
        printf("changetype: add\n");

        LwRtlHashMapResetIter(&iter);
        while (LwRtlHashMapIterate(pDiff->mods, &iter, &pair))
        {
            pMod = (PVDIR_LDAP_MOD)pair.pValue;
            for (i = 0; pMod->pVals->pStringList[i]; i++)
            {
                printf("%s: %s\n", pMod->pszType, pMod->pVals->pStringList[i]);
            }
        }
        pNode = pNode->pPrev;
    }

    pNode = pSchemaDiff->classToModify->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;
        printf("\n");
        printf("dn: %s\n", pDiff->pszDN);
        printf("changetype: modify\n");

        LwRtlHashMapResetIter(&iter);
        while (LwRtlHashMapIterate(pDiff->mods, &iter, &pair))
        {
            pMod = (PVDIR_LDAP_MOD)pair.pValue;
            printf("%s: %s\n", ppszModOp[pMod->op], pMod->pszType);
            for (i = 0; pMod->pVals->pStringList[i]; i++)
            {
                printf("%s: %s\n", pMod->pszType, pMod->pVals->pStringList[i]);
            }
            printf("%s", iter.Inner.pNext ? "-\n" : "");
        }
        pNode = pNode->pPrev;
    }
}

DWORD
VdcSchemaRefreshSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN    pConn
    )
{
    DWORD   dwError = 0;
    PSTR    pszAttrs[] = { "refresh", NULL };
    LDAPMessage*    pResult = NULL;

    if (!pConn)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    printf("Request server to refresh schema repl status entries\n");

    dwError = ldap_search_ext_s(
            pConn->pLd,
            SCHEMA_REPL_STATUS_DN,
            LDAP_SCOPE_SUBTREE,
            "(objectclass=*)",
            pszAttrs,
            FALSE,
            NULL,
            NULL,
            NULL,
            0,
            &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Request succeeded\n");

cleanup:
    ldap_msgfree(pResult);
    return dwError;

error:
    printf("Failed to request server to refresh schema repl status entries\n");
    goto cleanup;
}

DWORD
VdcSchemaWaitForSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    DWORD   dwError = 0;
    DWORD   dwTimeRemaining = 0;
    DWORD   dwEachWait = 10;
    DWORD   i = 0;
    PSTR    pszAttrs[] = { "+", NULL };
    BOOLEAN bReady = FALSE;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppValues = NULL;

    if (!pConn || !pOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwTimeRemaining = (DWORD)pOpParam->iTimeout;

    printf("Wait for schema repl status entries for %d seconds\n",
            dwTimeRemaining);

    while (!bReady && dwTimeRemaining)
    {
        VmDirSleep(dwEachWait * 1000);

        ldap_msgfree(pResult);
        dwError = ldap_search_ext_s(
                pConn->pLd,
                SCHEMA_REPL_STATUS_DN,
                LDAP_SCOPE_BASE,
                "(objectclass=*)",
                pszAttrs,
                FALSE,
                NULL,
                NULL,
                NULL,
                0,
                &pResult);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = ldap_count_entries(pConn->pLd, pResult) == 0 ?
                VMDIR_ERROR_ENTRY_NOT_FOUND : 0;
        BAIL_ON_VMDIR_ERROR(dwError);

        pEntry = ldap_first_entry(pConn->pLd, pResult);

        ldap_value_free_len(ppValues);
        ppValues = ldap_get_values_len(
                pConn->pLd, pEntry, ATTR_SERVER_RUNTIME_STATUS);

        for (i = 0; i < ldap_count_values_len(ppValues); i++)
        {
            PSTR pszPrefix = SCHEMA_REPL_STATUS_REFRESH_IN_PROGRESS;
            PSTR pszSuffix = "FALSE";
            PSTR pszVal = ppValues[i]->bv_val;

            if (VmDirStringStartsWith(pszVal, pszPrefix, FALSE) &&
                VmDirStringEndsWith(pszVal, pszSuffix, FALSE))
            {
                bReady = TRUE;
                break;
            }
        }

        dwTimeRemaining = dwTimeRemaining > dwEachWait ?
                dwTimeRemaining - dwEachWait : 0;
    }

    if (bReady)
    {
        printf("Schema repl status entries are ready\n");
    }
    else
    {
        printf("Timed out waiting for schema repl status entries\n");
        dwError = VMDIR_ERROR_TIMELIMIT_EXCEEDED;
    }

cleanup:
    ldap_value_free_len(ppValues);
    ldap_msgfree(pResult);
    return dwError;

error:
    printf("Failed during waiting for schema repl status entries\n");
    goto cleanup;
}

DWORD
VdcSchemaGetSchemaReplStatusEntries(
    PVDC_SCHEMA_CONN            pConn,
    PVDIR_SCHEMA_REPL_STATE**   pppReplStates
    )
{
    DWORD   dwError = 0;
    DWORD   dwNumEntries = 0;
    DWORD   i = 0;
    PSTR    pszAttrs[] = { "+", NULL };
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    PVDIR_SCHEMA_REPL_STATE*    ppReplStates = NULL;

    if (!pConn || !pppReplStates)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    printf("Retrieve schema repl status entries\n");

    dwError = ldap_search_ext_s(
            pConn->pLd,
            SCHEMA_REPL_STATUS_DN,
            LDAP_SCOPE_ONELEVEL,
            "(objectclass=*)",
            pszAttrs,
            FALSE,
            NULL,
            NULL,
            NULL,
            0,
            &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pConn->pLd, pResult);

    dwError = VmDirAllocateMemory(
            sizeof(PVDIR_SCHEMA_REPL_STATE) * (dwNumEntries + 1),
            (PVOID*)&ppReplStates);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (pEntry = ldap_first_entry(pConn->pLd, pResult); pEntry;
         pEntry = ldap_next_entry(pConn->pLd, pEntry))
    {
        dwError = VmDirSchemaReplStateParseLDAPEntry(
                pConn->pLd, pEntry, &ppReplStates[i++]);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppReplStates = ppReplStates;

    printf("Retrieval succeeded\n");

cleanup:
    ldap_msgfree(pResult);
    return dwError;

error:
    printf("Failed to retrieve schema repl status entries\n");

    for (i = 0; ppReplStates && ppReplStates[i]; i++)
    {
        VmDirFreeSchemaReplState(ppReplStates[i]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppReplStates);
    goto cleanup;
}

DWORD
VdcSchemaPrintSchemaReplStatusEntry(
    PVDIR_SCHEMA_REPL_STATE pReplState
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;

    if (!pReplState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    {
        PSTR pszStrKeys[] = {
                SCHEMA_REPL_STATUS_HOST_NAME,
                SCHEMA_REPL_STATUS_DOMAIN_NAME,
                NULL
        };
        PSTR pszStrVals[] = {
                pReplState->pszHostName,
                pReplState->pszDomainName
        };
        PSTR pszBoolKeys[] = {
                SCHEMA_REPL_STATUS_CHECK_INITIATED,
                SCHEMA_REPL_STATUS_CHECK_SUCCEEDED,
                SCHEMA_REPL_STATUS_TREE_IN_SYNC,
                NULL
        };
        PSTR pszBoolVals[] = {
                pReplState->bCheckInitiated ? "TRUE" : "FALSE",
                pReplState->bCheckSucceeded ? "TRUE" : "FALSE",
                pReplState->bTreeInSync ? "TRUE" : "FALSE"
        };
        PSTR pszDwordKeys[] = {
                SCHEMA_REPL_STATUS_ATTR_MISSING_IN_TREE,
                SCHEMA_REPL_STATUS_ATTR_MISMATCH_IN_TREE,
                SCHEMA_REPL_STATUS_CLASS_MISSING_IN_TREE,
                SCHEMA_REPL_STATUS_CLASS_MISMATCH_IN_TREE,
                NULL
        };
        DWORD dwDwordVals[] = {
                pReplState->dwAttrMissingInTree,
                pReplState->dwAttrMismatchInTree,
                pReplState->dwClassMissingInTree,
                pReplState->dwClassMismatchInTree
        };

        for (i = 0; pszStrKeys[i]; i++)
        {
            printf("%s: %s\n", pszStrKeys[i], pszStrVals[i]);
        }

        for (i = 0; pszBoolKeys[i]; i++)
        {
            printf("%s: %s\n", pszBoolKeys[i], pszBoolVals[i]);
        }

        for (i = 0; pszDwordKeys[i]; i++)
        {
            printf("%s: %d\n", pszDwordKeys[i], dwDwordVals[i]);
        }
    }

cleanup:
    return dwError;

error:
    printf("Failed to print schema repl status entry\n");
    goto cleanup;
}
