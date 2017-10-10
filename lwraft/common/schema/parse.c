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
VmDirLdapAtParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPAttributeType*  pSource = NULL;

    if (!pcszStr || !ppAt)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IS_ATTRIBUTETYPES_TAG(pcszStr))
    {
        pcszStr += ATTRIBUTETYPES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2attributetype(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2attributetype failed (code:%d) (err:%s) %s",
                __FUNCTION__,
                iCode,
                ldap_scherr2str(iCode),
                pErr);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    dwError = VmDirLdapAtCreate(pSource, ppAt);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_attributetype_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapOcParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPObjectClass*    pSource = NULL;

    if (!pcszStr || !ppOc)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IS_OBJECTCLASSES_TAG(pcszStr))
    {
        pcszStr += OBJECTCLASSES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2objectclass(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2objectclass failed (code:%d) (err:%s) %s",
                __FUNCTION__,
                iCode,
                ldap_scherr2str(iCode),
                pErr);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    pSource->oc_kind = gVdirOpenLdapToADClassType[pSource->oc_kind];

    dwError = VmDirLdapOcCreate(pSource, ppOc);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_objectclass_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapCrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPContentRule*    pSource = NULL;

    if (!pcszStr || !ppCr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IS_CONTENTRULES_TAG(pcszStr))
    {
        pcszStr += CONTENTRULES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2contentrule(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2contentrule failed (code:%d) (err:%s) %s",
                __FUNCTION__,
                iCode,
                ldap_scherr2str(iCode),
                pErr);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    dwError = VmDirLdapCrCreate(pSource, ppCr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_contentrule_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapSrParseStr(
    PCSTR                       pcszStr,
    PVDIR_LDAP_STRUCTURE_RULE*  ppSr
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPStructureRule*  pSource = NULL;

    if (!pcszStr || !ppSr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IS_STRUCTURERULES_TAG(pcszStr))
    {
        pcszStr += STRUCTURERULES_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2structurerule(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2structurerule failed (code:%d) (err:%s) %s",
                __FUNCTION__,
                iCode,
                ldap_scherr2str(iCode),
                pErr);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    dwError = VmDirLdapSrCreate(pSource, ppSr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_structurerule_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapNfParseStr(
    PCSTR                   pcszStr,
    PVDIR_LDAP_NAME_FORM*   ppNf
    )
{
    DWORD   dwError = 0;
    int     iCode = 0;
    const char* pErr = NULL;
    const int   flags = LDAP_SCHEMA_ALLOW_ALL;
    LDAPNameForm*   pSource = NULL;

    if (!pcszStr || !ppNf)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IS_NAMEFORM_TAG(pcszStr))
    {
        pcszStr += NAMEFORM_TAG_LEN;
        while (isspace(*pcszStr)) pcszStr++;
    }

    pSource = ldap_str2nameform(pcszStr, &iCode, &pErr, flags);

    if (!pSource)
    {
        VMDIR_LOG_ERROR(
                VMDIR_LOG_MASK_ALL,
                "%s: ldap_str2nameform failed (code:%d) (err:%s) %s",
                __FUNCTION__,
                iCode,
                ldap_scherr2str(iCode),
                pErr);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    dwError = VmDirLdapNfCreate(pSource, ppNf);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_nameform_free(pSource);
    }
    goto cleanup;
}

DWORD
VmDirLdapIdxParseStr(
    PCSTR       pcszStr,
    PSTR*       ppszAtName,
    PBOOLEAN    pbGlobalUniq
    )
{
    DWORD   dwError = 0;
    SIZE_T  lenName = 0;
    PCSTR   pszName = NULL;
    PSTR    pszNameCpy = NULL;
    BOOLEAN bGlobalUniq = FALSE;
    PVMDIR_STRING_LIST  pTokList = NULL;

    if (IsNullOrEmptyString(pcszStr) || !ppszAtName || !pbGlobalUniq)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringToTokenList(pcszStr, " ", &pTokList);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pTokList->dwCount < 4 || pTokList->dwCount > 5)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    if (VmDirStringCompareA("(", pTokList->pStringList[0], TRUE) ||
        VmDirStringCompareA("NAME", pTokList->pStringList[1], TRUE) ||
        VmDirStringCompareA(")", pTokList->pStringList[pTokList->dwCount - 1], TRUE))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    pszName = pTokList->pStringList[2];
    lenName = VmDirStringLenA(pszName);

    if (lenName < 3 || pszName[0] != '\'' || pszName[lenName - 1] != '\'')
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
    }

    dwError = VmDirAllocateStringA(pszName + 1, &pszNameCpy);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszNameCpy[lenName - 2] = '\0';

    if (pTokList->dwCount == 5)
    {
        if (VmDirStringCompareA("GLOBALLY-UNIQUE", pTokList->pStringList[3], TRUE))
        {
            BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_SCHEMA);
        }
        bGlobalUniq = TRUE;
    }

    *ppszAtName = pszNameCpy;
    *pbGlobalUniq = bGlobalUniq;

cleanup:
    VmDirStringListFree(pTokList);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed to parse \"%s\" (%d)",
            __FUNCTION__,
            VDIR_SAFE_STRING(pcszStr),
            dwError);

    VMDIR_SAFE_FREE_MEMORY(pszNameCpy);
    goto cleanup;
}

DWORD
VmDirLdapAtParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_ATTRIBUTE_TYPE*  ppAt
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    DWORD   dwVmwAttrUsage = 0;
    DWORD   dwSearchFlags = 0;
    PSTR*   ppszUniqueScopes = NULL;
    BerValue**  ppBerVals = NULL;
    LDAPAttributeType*  pSource = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;

    if (!pEntry || !ppAt)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPAttributeType), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_IS_SINGLE_VALUED);
    if (ppBerVals && ppBerVals[0])
    {
        pSource->at_single_value =
                VmDirStringCompareA(
                        "TRUE", ppBerVals[0]->bv_val, FALSE) == 0;

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_VMW_ATTRIBUTE_USAGE);
    if (ppBerVals && ppBerVals[0])
    {
        dwVmwAttrUsage = VmDirStringToIA(ppBerVals[0]->bv_val);

        pSource->at_no_user_mod = dwVmwAttrUsage & 0x8 ? 1 : 0;

        pSource->at_usage = 0;
        dwVmwAttrUsage &= 0x7;
        while (dwVmwAttrUsage)
        {
            pSource->at_usage++;
            dwVmwAttrUsage >>= 1;
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->at_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_ATTRIBUTE_SYNTAX);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_syntax_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_ATTRIBUTE_ID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_DESCRIPTION);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->at_desc);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SEARCH_FLAGS);
    if (ppBerVals && ppBerVals[0])
    {
        dwSearchFlags = VmDirStringToIA(ppBerVals[0]->bv_val);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_UNIQUENESS_SCOPE);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&ppszUniqueScopes);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &ppszUniqueScopes[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    dwError = VmDirLdapAtCreate(pSource, &pAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    pAt->dwSearchFlags = dwSearchFlags;
    pAt->ppszUniqueScopes = ppszUniqueScopes;

    *ppAt = pAt;

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_attributetype_free(pSource);
    }
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    VmDirFreeStrArray(ppszUniqueScopes);
    VmDirFreeLdapAt(pAt);
    goto cleanup;
}

DWORD
VmDirLdapOcParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_OBJECT_CLASS*    ppOc
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    BerValue**  ppBerVals = NULL;
    LDAPObjectClass*    pSource = NULL;

    if (!pEntry || !ppOc)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPObjectClass), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SUBCLASSOF);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->oc_sup_oids);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_sup_oids[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->oc_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_GOVERNSID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_DESCRIPTION);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->oc_desc);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_OBJECTCLASS_CATEGORY);
    if (ppBerVals && ppBerVals[0])
    {
        pSource->oc_kind = VmDirStringToIA(ppBerVals[0]->bv_val);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMMUSTCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->oc_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->oc_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMMAYCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->oc_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->oc_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    dwError = VmDirLdapOcCreate(pSource, ppOc);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_objectclass_free(pSource);
    }
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    goto cleanup;
}

DWORD
VmDirLdapCrParseLDAPEntry(
    LDAP*                       pLd,
    LDAPMessage*                pEntry,
    PVDIR_LDAP_CONTENT_RULE*    ppCr
    )
{
    BOOLEAN bHasCr = FALSE;
    DWORD   dwError = 0;
    DWORD   i = 0, dwAux = 0;
    BerValue**  ppBerVals = NULL;
    LDAPContentRule*    pSource = NULL;

    if (!pEntry || !ppCr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirAllocateMemory(
            sizeof(LDAPContentRule), (PVOID*)&pSource);
    BAIL_ON_VMDIR_ERROR(dwError);

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_CN);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateMemory(
                sizeof(char*) * 2, (PVOID*)&pSource->cr_names);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->cr_names[0]);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_GOVERNSID);
    if (ppBerVals && ppBerVals[0])
    {
        dwError = VmDirAllocateStringA(
                ppBerVals[0]->bv_val, &pSource->cr_oid);
        BAIL_ON_VMDIR_ERROR(dwError);

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_MUSTCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_at_oids_must);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_at_oids_must[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_MAYCONTAIN);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_at_oids_may);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_at_oids_may[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_AUXILIARY_CLASS);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirAllocateMemory(
                    sizeof(char*) * (dwNumVals + 1),
                    (PVOID*)&pSource->cr_oc_oids_aux);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_oc_oids_aux[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            dwAux = dwNumVals;
            bHasCr = TRUE;
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    ppBerVals = ldap_get_values_len(pLd, pEntry, ATTR_SYSTEMAUXILIARY_CLASS);
    if (ppBerVals)
    {
        DWORD dwNumVals = ldap_count_values_len(ppBerVals);
        if (dwNumVals > 0)
        {
            dwError = VmDirReallocateMemoryWithInit(
                    (PVOID)pSource->cr_oc_oids_aux,
                    (PVOID*)&pSource->cr_oc_oids_aux,
                    sizeof(char*) * (dwAux + dwNumVals + 1),
                    dwAux ? sizeof(char*) * (dwAux + 1) : 0);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i = 0; i < dwNumVals; i++, dwAux++)
            {
                dwError = VmDirAllocateStringA(
                        ppBerVals[i]->bv_val, &pSource->cr_oc_oids_aux[dwAux]);
                BAIL_ON_VMDIR_ERROR(dwError);
            }

            bHasCr = TRUE;
        }

        VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    }

    if (!bHasCr)
    {
        goto error;
    }

    dwError = VmDirLdapCrCreate(pSource, ppCr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    if (pSource)
    {
        ldap_contentrule_free(pSource);
    }
    if (ppCr)
    {
        *ppCr = NULL;
    }
    VDIR_SAFE_LDAP_VALUE_FREE_LEN(ppBerVals);
    goto cleanup;
}

DWORD
VmDirLdapAtToStr(
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt,
    PSTR*                       ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pAt || !ppszStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszStr = ldap_attributetype2str(pAt->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapOcToStr(
    PVDIR_LDAP_OBJECT_CLASS pOc,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pOc || !ppszStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pOc->pSource->oc_kind = gVdirADToOpenLdapClassType[pOc->type - 1];

    pszStr = ldap_objectclass2str(pOc->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    pOc->pSource->oc_kind = pOc->type;
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapCrToStr(
    PVDIR_LDAP_CONTENT_RULE pCr,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pCr || !ppszStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszStr = ldap_contentrule2str(pCr->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapSrToStr(
    PVDIR_LDAP_STRUCTURE_RULE   pSr,
    PSTR*                       ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pSr || !ppszStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszStr = ldap_structurerule2str(pSr->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}

DWORD
VmDirLdapNfToStr(
    PVDIR_LDAP_NAME_FORM    pNf,
    PSTR*                   ppszStr
    )
{
    DWORD   dwError = 0;
    PSTR    pszStr = NULL;

    if (!pNf || !ppszStr)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    pszStr = ldap_nameform2str(pNf->pSource);
    dwError = pszStr ? 0 : ERROR_INVALID_SCHEMA;
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszStr = pszStr;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pszStr);
    goto cleanup;
}
