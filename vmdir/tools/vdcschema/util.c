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
