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
VdcSchemaOpParamInit(
    PVDC_SCHEMA_OP_PARAM*   ppOpParam
    )
{
    DWORD   dwError = 0;
    PVDC_SCHEMA_OP_PARAM    pOpParam = NULL;

    if (!ppOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(sizeof(VDC_SCHEMA_OP_PARAM), (PVOID*)&pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppOpParam = pOpParam;

error:
    return dwError;
}

DWORD
VdcSchemaOpParamValidate(
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    DWORD   dwError = 0;

    if (!pOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pOpParam->opCode == OP_PATCH_SCHEMA_DEFS)
    {
        if (IsNullOrEmptyString(pOpParam->pszFileName))
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else if (pOpParam->opCode == OP_GET_SCHEMA_REPL_STATUS)
    {
        if (pOpParam->iTimeout < 0)
        {
            dwError = VMDIR_ERROR_INVALID_PARAMETER;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (pOpParam->iTimeout == 0)
        {
            pOpParam->iTimeout = 60; // default value
        }
    }

error:
    return dwError;
}

DWORD
VdcSchemaOpGetSupportedSyntaxes(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pSyntaxMap = NULL;

    if (!pConn || !pOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaGetSyntaxMap(pConn, &pSyntaxMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    (VOID)VmDirSchemaPrintSyntaxMap(pSyntaxMap);

cleanup:
    LwRtlHashMapClear(pSyntaxMap, VmDirSimpleHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pSyntaxMap);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcSchemaOpPatchSchemaDefs(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    DWORD   dwError = 0;
    PVDIR_LDAP_SCHEMA   pCurSchema = NULL;
    PVDIR_LDAP_SCHEMA   pTmpSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewSchema = NULL;
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff = NULL;

    if (!pConn || !pOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // get remote schema (tree)
    dwError = VmDirLdapSchemaInit(&pCurSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadRemoteSchema(pCurSchema, pConn->pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    // try loading file
    dwError = VmDirLdapSchemaDeepCopy(pCurSchema, &pTmpSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadFile(pTmpSchema, pOpParam->pszFileName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaMerge(pCurSchema, pTmpSchema, &pNewSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    // compute diff
    dwError = VmDirLdapSchemaGetDiff(pCurSchema, pNewSchema, &pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    // make sure that attributes' syntaxes are supported
    dwError = VmDirSchemaValidateSyntaxes(pNewSchema, pSchemaDiff, pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    // show diff
    (VOID)VmDirSchemaPrintDiff(pSchemaDiff);

    // perform patch (if not dryrun)
    if (!pOpParam->bDryrun)
    {
        dwError = VmDirPatchRemoteSchemaObjects(pConn->pLd, pNewSchema);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("\nSuccessfully patched schema definitions\n");
    }

cleanup:
    VmDirFreeLdapSchema(pCurSchema);
    VmDirFreeLdapSchema(pTmpSchema);
    VmDirFreeLdapSchema(pNewSchema);
    VmDirFreeLdapSchemaDiff(pSchemaDiff);
    return dwError;

error:
    goto cleanup;
}

DWORD
VdcSchemaOpGetSchemaReplStatus(
    PVDC_SCHEMA_CONN        pConn,
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    BOOLEAN bAllInSync = TRUE;
    PVDIR_SCHEMA_REPL_STATE*    ppReplStates = NULL;

    if (!pConn || !pOpParam)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaRefreshSchemaReplStatusEntries(pConn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaWaitForSchemaReplStatusEntries(pConn, pOpParam);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcSchemaGetSchemaReplStatusEntries(pConn, &ppReplStates);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; ppReplStates[i]; i++)
    {
        PVDIR_SCHEMA_REPL_STATE pReplState = ppReplStates[i];

        if (pOpParam->bVerbose)
        {
            printf("\nPartner %d\n-----------------\n", i + 1);
            dwError = VdcSchemaPrintSchemaReplStatusEntry(pReplState);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        bAllInSync &= pReplState->bTreeInSync;
    }

    dwError = bAllInSync ? 0 : VMDIR_ERROR_SCHEMA_MISMATCH;
    printf("\nSync: %s\n", bAllInSync ? "TRUE" : "FALSE");

cleanup:
    for (i = 0; ppReplStates && ppReplStates[i]; i++)
    {
        VmDirFreeSchemaReplState(ppReplStates[i]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppReplStates);
    return dwError;

error:
    printf("Failed to get schema repl status\n");
    goto cleanup;
}

VOID
VdcSchemaFreeOpParam(
    PVDC_SCHEMA_OP_PARAM    pOpParam
    )
{
    if (pOpParam)
    {
        VMDIR_SAFE_FREE_MEMORY(pOpParam->pszFileName);
    }
    VMDIR_SAFE_FREE_MEMORY(pOpParam);
}
