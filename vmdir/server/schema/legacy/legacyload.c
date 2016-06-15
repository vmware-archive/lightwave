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

#include "../includes.h"

DWORD
VmDirSchemaAttrIdMapLoadSubSchemaSubEntry(
    PVDIR_SCHEMA_ATTR_ID_MAP    pAttrIdMap,
    PVDIR_ENTRY                 pSchemaEntry
    )
{
    DWORD   dwError = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;
    PSTR    pszBuf = NULL;
    PSTR    pszToken = NULL;
    char*   save = NULL;
    PSTR    pszName = NULL;
    PSTR    pszId = NULL;
    USHORT  usId = 0;

    if (!pAttrIdMap || !pSchemaEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttr = VmDirFindAttrByName(pSchemaEntry, ATTR_VMW_ATTRIBUTE_TO_ID_MAP);

    if (!pAttr || pAttr->numVals != 1)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pAttr->vals[0].lberbv_val, &pszBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszToken = VmDirStringTokA(pszBuf, SCHEMA_ATTR_ID_MAP_TOKEN_SEP, &save);
    while (pszToken && VmDirStringChrA(pszToken, SCHEMA_ATTR_ID_MAP_SEP[0]))
    {
        pszId = VmDirStringTokA(pszToken, SCHEMA_ATTR_ID_MAP_SEP, &pszName);
        usId = (USHORT)VmDirStringToIA(pszId);

        if (VmDirSchemaAttrIdMapGetAttrId(pAttrIdMap, pszName, NULL) != 0)
        {
            dwError = VmDirSchemaAttrIdMapAddNewAttr(pAttrIdMap, pszName, usId);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pszToken = VmDirStringTokA(NULL, SCHEMA_ATTR_ID_MAP_TOKEN_SEP, &save);
    }
    pAttrIdMap->usNextId = (USHORT)VmDirStringToIA(pszToken);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszBuf);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirLdapSchemaLoadSubSchemaSubEntry(
    PVDIR_LDAP_SCHEMA   pLdapSchema,
    PVDIR_ENTRY         pSchemaEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_ATTRIBUTE pAttrAts = NULL;
    PVDIR_ATTRIBUTE pAttrOcs = NULL;
    PVDIR_ATTRIBUTE pAttrCrs = NULL;
    PVMDIR_STRING_LIST  pAtStrList = NULL;
    PVMDIR_STRING_LIST  pOcStrList = NULL;
    PVMDIR_STRING_LIST  pCrStrList = NULL;
    PSTR    pszDef = NULL;

    if (!pLdapSchema || !pSchemaEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttrAts = VmDirFindAttrByName(pSchemaEntry, ATTR_ATTRIBUTETYPES);
    pAttrOcs = VmDirFindAttrByName(pSchemaEntry, ATTR_OBJECTCLASSES);
    pAttrCrs = VmDirFindAttrByName(pSchemaEntry, ATTR_DITCONTENTRULES);

    if (!pAttrAts || !pAttrOcs || !pAttrCrs)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pAtStrList, 2048);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAttrAts->numVals; i++)
    {
        pszDef = pAttrAts->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pAtStrList, pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pOcStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAttrOcs->numVals; i++)
    {
        pszDef = pAttrOcs->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pOcStrList, pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pCrStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pAttrCrs->numVals; i++)
    {
        pszDef = pAttrCrs->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirStringListAdd(pCrStrList, pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLdapSchemaLoadStrLists(
            pLdapSchema, pAtStrList, pOcStrList, pCrStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirStringListFree(pAtStrList);
    VmDirStringListFree(pOcStrList);
    VmDirStringListFree(pCrStrList);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    goto cleanup;
}

DWORD
VmDirLegacySchemaLoadSubSchemaSubEntry(
    PVDIR_LEGACY_SCHEMA pLegacySchema,
    PVDIR_ENTRY         pSchemaEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_ATTRIBUTE pAttrAts = NULL;
    PVDIR_ATTRIBUTE pAttrOcs = NULL;
    PVDIR_ATTRIBUTE pAttrCrs = NULL;
    PVDIR_LDAP_ATTRIBUTE_TYPE   pAt = NULL;
    PVDIR_LDAP_OBJECT_CLASS     pOc = NULL;
    PVDIR_LDAP_CONTENT_RULE     pCr = NULL;
    PSTR    pszDef = NULL;

    if (!pLegacySchema || !pSchemaEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttrAts = VmDirFindAttrByName(pSchemaEntry, ATTR_ATTRIBUTETYPES);
    pAttrOcs = VmDirFindAttrByName(pSchemaEntry, ATTR_OBJECTCLASSES);
    pAttrCrs = VmDirFindAttrByName(pSchemaEntry, ATTR_DITCONTENTRULES);

    if (!pAttrAts || !pAttrOcs || !pAttrCrs)
    {
        dwError = ERROR_INVALID_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; i < pAttrAts->numVals; i++)
    {
        pszDef = pAttrAts->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapAtParseStr(pszDef, &pAt);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pLegacySchema->pAtDefStrMap,
                pAt->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddAt(pLegacySchema->pSchema, pAt);
        BAIL_ON_VMDIR_ERROR(dwError);
        pAt = NULL;
    }

    for (i = 0; i < pAttrOcs->numVals; i++)
    {
        pszDef = pAttrOcs->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapOcParseStr(pszDef, &pOc);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pLegacySchema->pOcDefStrMap,
                pOc->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddOc(pLegacySchema->pSchema, pOc);
        BAIL_ON_VMDIR_ERROR(dwError);
        pOc = NULL;
    }

    for (i = 0; i < pAttrCrs->numVals; i++)
    {
        pszDef = pAttrCrs->vals[i].lberbv_val;

        dwError = VmDirFixLegacySchemaDefSyntaxErr(pszDef, &pszDef);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapCrParseStr(pszDef, &pCr);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pLegacySchema->pCrDefStrMap,
                pCr->pszName, pszDef, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSchemaAddCr(pLegacySchema->pSchema, pCr);
        BAIL_ON_VMDIR_ERROR(dwError);
        pCr = NULL;
    }

    dwError = VmDirLdapSchemaResolveAndVerifyAll(pLegacySchema->pSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)", __FUNCTION__, dwError );

    VmDirFreeLdapAt(pAt);
    VmDirFreeLdapOc(pOc);
    VmDirFreeLdapCr(pCr);
    goto cleanup;
}
