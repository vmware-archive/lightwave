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

#include "includes.h"

DWORD
VmDirLegacySchemaModInit(
    PVDIR_LEGACY_SCHEMA_MOD*    ppLegacySchemaMod
    )
{
    DWORD   dwError = 0;
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod = NULL;

    if (!ppLegacySchemaMod)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_LEGACY_SCHEMA_MOD),
            (PVOID*)&pLegacySchemaMod);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pDelAt, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pAddAt, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pDelOc, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pAddOc, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pDelCr, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pLegacySchemaMod->pAddCr, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLegacySchemaMod = pLegacySchemaMod;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLegacySchemaMod(pLegacySchemaMod);
    goto cleanup;
}

DWORD
VmDirLegacySchemaModPopulate(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod,
    PVDIR_LEGACY_SCHEMA     pLegacySchema,
    PVDIR_LDAP_SCHEMA       pNewSchema
    )
{
    DWORD   dwError = 0;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOrgOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;
    PSTR    pszOrgStr = NULL;
    PSTR    pszNewStr = NULL;
    PSTR*   ppszOcSup = NULL;

    while (LwRtlHashMapIterate(pNewSchema->attributeTypes, &iter, &pair))
    {
        pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;

        if (VmDirIsMultiNameAttribute(pAt->pszName))
        {
            continue;
        }

        dwError = VmDirLdapAtToStr(pAt, &pszNewStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (LwRtlHashMapFindKey(pLegacySchema->pAtDefStrMap,
                (PVOID*)&pszOrgStr, pAt->pszName))
        {
            dwError = VmDirStringListAdd(pLegacySchemaMod->pAddAt, pszNewStr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pszOrgStr, pszNewStr, TRUE))
        {
            dwError = VmDirAllocateStringA(pszOrgStr, &pszOrgStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pLegacySchemaMod->pDelAt, pszOrgStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pLegacySchemaMod->pAddAt, pszNewStr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            VMDIR_SAFE_FREE_MEMORY(pszNewStr);
        }
    }

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pNewSchema->objectClasses, &iter, &pair))
    {
        pOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;

        dwError = VmDirLdapOcToStr(pOc, &pszNewStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (LwRtlHashMapFindKey(pLegacySchema->pOcDefStrMap,
                (PVOID*)&pszOrgStr, pOc->pszName))
        {
            dwError = VmDirStringListAdd(pLegacySchemaMod->pAddOc, pszNewStr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            // 6.x object classes have inconsistent default sup value
            // (some have 'top' and other leave it blank)
            // Make sure to leave sup value blank if it's blank in 6.x
            dwError = VmDirLdapOcParseStr(pszOrgStr, &pOrgOc);
            BAIL_ON_VMDIR_ERROR(dwError);

            if (!pOrgOc->pszSup)
            {
                VMDIR_SAFE_FREE_MEMORY(pszNewStr);

                ppszOcSup = pOc->pSource->oc_sup_oids;
                pOc->pSource->oc_sup_oids = NULL;

                dwError = VmDirLdapOcToStr(pOc, &pszNewStr);
                BAIL_ON_VMDIR_ERROR(dwError);

                pOc->pSource->oc_sup_oids = ppszOcSup;
                ppszOcSup = NULL;
            }
            VmDirFreeLdapOc(pOrgOc);
            pOrgOc = NULL;

            if (VmDirStringCompareA(pszOrgStr, pszNewStr, TRUE))
            {
                dwError = VmDirAllocateStringA(pszOrgStr, &pszOrgStr);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAdd(pLegacySchemaMod->pDelOc, pszOrgStr);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringListAdd(pLegacySchemaMod->pAddOc, pszNewStr);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
            else
            {
                VMDIR_SAFE_FREE_MEMORY(pszNewStr);
            }
        }
    }

    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pNewSchema->contentRules, &iter, &pair))
    {
        pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;

        dwError = VmDirLdapCrToStr(pCr, &pszNewStr);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (LwRtlHashMapFindKey(pLegacySchema->pCrDefStrMap,
                (PVOID*)&pszOrgStr, pCr->pszName))
        {
            dwError = VmDirStringListAdd(pLegacySchemaMod->pAddCr, pszNewStr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(pszOrgStr, pszNewStr, TRUE))
        {
            dwError = VmDirAllocateStringA(pszOrgStr, &pszOrgStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pLegacySchemaMod->pDelCr, pszOrgStr);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pLegacySchemaMod->pAddCr, pszNewStr);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            VMDIR_SAFE_FREE_MEMORY(pszNewStr);
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapOc(pOrgOc);
    goto cleanup;
}

VOID
VmDirFreeLegacySchemaMod(
    PVDIR_LEGACY_SCHEMA_MOD pLegacySchemaMod
    )
{
    if (pLegacySchemaMod)
    {
        VmDirStringListFree(pLegacySchemaMod->pDelAt);
        VmDirStringListFree(pLegacySchemaMod->pAddAt);
        VmDirStringListFree(pLegacySchemaMod->pDelOc);
        VmDirStringListFree(pLegacySchemaMod->pAddOc);
        VmDirStringListFree(pLegacySchemaMod->pDelCr);
        VmDirStringListFree(pLegacySchemaMod->pAddCr);
        VMDIR_SAFE_FREE_MEMORY(pLegacySchemaMod);
    }
}
