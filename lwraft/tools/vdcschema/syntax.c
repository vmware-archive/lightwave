/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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
VdcSchemaGetSyntaxMap(
    PVDC_SCHEMA_CONN    pConn,
    PLW_HASHMAP*        ppSyntaxMap
    )
{
    DWORD   dwError = 0;
    int     i = 0;
    LDAPMessage*    pResult = NULL;
    LDAPMessage*    pEntry = NULL;
    struct berval** ppValues = NULL;
    int         iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPSyntax* pSx = NULL;
    PSTR        pszOid = NULL;
    PSTR        pszDesc = NULL;
    PLW_HASHMAP pSyntaxMap = NULL;

    if (!pConn || !ppSyntaxMap)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = LwRtlCreateHashMap(&pSyntaxMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get remote schema (blob)
    dwError = VmDirLdapSearchSubSchemaSubEntry(pConn->pLd, &pResult, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    // populate syntax map
    ppValues = ldap_get_values_len(pConn->pLd, pEntry, ATTR_LDAPSYNTAXES);
    dwError = ppValues ? 0 : ERROR_INVALID_DATA;
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < ldap_count_values_len(ppValues); i++)
    {
        pSx = ldap_str2syntax(ppValues[i]->bv_val, &iCode, &pErr, flags);
        if (!pSx)
        {
            VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "%s: ldap_str2contentrule failed (code:%d) (err:%s) %s",
                    __FUNCTION__, iCode, ldap_scherr2str(iCode), pErr);
            dwError = VMDIR_ERROR_INVALID_SCHEMA;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateStringA(pSx->syn_oid, &pszOid);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pSx->syn_desc, &pszDesc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pSyntaxMap, pszOid, pszDesc, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        ldap_syntax_free(pSx);
        pSx = NULL;
        pszOid = NULL;
        pszDesc = NULL;
    }

    *ppSyntaxMap = pSyntaxMap;

cleanup:
    if (pSx)
    {
        ldap_syntax_free(pSx);
    }
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    return dwError;

error:
    LwRtlHashMapClear(pSyntaxMap, VmDirSimpleHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pSyntaxMap);
    VMDIR_SAFE_FREE_MEMORY(pszOid);
    VMDIR_SAFE_FREE_MEMORY(pszDesc);
    goto cleanup;
}

DWORD
VmDirSchemaValidateSyntaxes(
    PVDIR_LDAP_SCHEMA       pSchema,
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff,
    PVDC_SCHEMA_CONN        pConn
    )
{
    DWORD   dwError = 0;
    PLW_HASHMAP pSyntaxMap = NULL;
    PVDIR_LINKED_LIST_NODE          pNode = NULL;
    PVDIR_LDAP_SCHEMA_OBJECT_DIFF   pDiff = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE       pAt = NULL;

    if (!pSchema || !pSchemaDiff || !pConn)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcSchemaGetSyntaxMap(pConn, &pSyntaxMap);
    BAIL_ON_VMDIR_ERROR(dwError);

    pNode = pSchemaDiff->attrToAdd->pHead;
    while (pNode)
    {
        pDiff = (PVDIR_LDAP_SCHEMA_OBJECT_DIFF)pNode->pElement;

        LwRtlHashMapFindKey(pSchema->attributeTypes, (PVOID*)&pAt, pDiff->pszCN);
        if (LwRtlHashMapFindKey(pSyntaxMap, NULL, pAt->pszSyntaxOid))
        {
            printf("Attribute type \"%s\" uses unsupported syntax \"%s\"\n",
                    pAt->pszName,
                    pAt->pszSyntaxOid);

            dwError = VMDIR_ERROR_INVALID_SYNTAX;
        }

        pNode = pNode->pPrev;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    LwRtlHashMapClear(pSyntaxMap, VmDirSimpleHashMapPairFree, NULL);
    LwRtlFreeHashMap(&pSyntaxMap);
    return dwError;

error:
    goto cleanup;
}
