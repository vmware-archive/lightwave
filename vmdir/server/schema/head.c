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
VmDirSubSchemaSubEntry(
    PVDIR_ENTRY*    ppEntry
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;
    PVDIR_LDAP_SCHEMA   pLdapSchema = NULL;
    PVDIR_ENTRY         pEntry = NULL;

    PSTR    ppszBaseAttr[] = {
            ATTR_CN,            "aggregate",
            ATTR_OBJECT_CLASS,  OC_TOP,
            ATTR_OBJECT_CLASS,  OC_SUB_SCHEMA,
            ATTR_OBJECT_CLASS,  "subentry",
            NULL };

    USHORT  usNumAt = 0;
    USHORT  usNumOc = 0;
    USHORT  usNumCr = 0;
    PVDIR_BERVALUE  pBervAt = NULL;
    PVDIR_BERVALUE  pBervOc = NULL;
    PVDIR_BERVALUE  pBervCr = NULL;
    LW_HASHMAP_ITER iter = LW_HASHMAP_ITER_INIT;
    LW_HASHMAP_PAIR pair = {NULL, NULL};

    if (!ppEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLdapSchema = pSchemaCtx->pLdapSchema;

    dwError = VmDirAttrListToNewEntry(pSchemaCtx,
            SUB_SCHEMA_SUB_ENTRY_DN,
            ppszBaseAttr,
            &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    usNumAt = (USHORT)LwRtlHashMapGetCount(pLdapSchema->attributeTypes);
    usNumOc = (USHORT)LwRtlHashMapGetCount(pLdapSchema->objectClasses);
    usNumCr = (USHORT)LwRtlHashMapGetCount(pLdapSchema->contentRules);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (usNumAt + 1),
            (PVOID*)&pBervAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (usNumOc + 1),
            (PVOID*)&pBervOc);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VDIR_BERVALUE) * (usNumCr + 1),
            (PVOID*)&pBervCr);
    BAIL_ON_VMDIR_ERROR(dwError);

    while (LwRtlHashMapIterate(pLdapSchema->attributeTypes, &iter, &pair))
    {
        PVDIR_LDAP_ATTRIBUTE_TYPE pAt = (PVDIR_LDAP_ATTRIBUTE_TYPE)pair.pValue;
        dwError = VmDirLdapAtToStr(pAt, &pBervAt[i].lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pBervAt[i].lberbv_len = VmDirStringLenA(pBervAt[i].lberbv_val);
        pBervAt[i].bOwnBvVal = TRUE;
        i += 1;
    }

    dwError = VmDirEntryAddBervArrayAttribute(
            pEntry, ATTR_ATTRIBUTETYPES, pBervAt, usNumAt);
    BAIL_ON_VMDIR_ERROR(dwError);

    i = 0;
    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pLdapSchema->objectClasses, &iter, &pair))
    {
        PVDIR_LDAP_OBJECT_CLASS pOc = (PVDIR_LDAP_OBJECT_CLASS)pair.pValue;
        dwError = VmDirLdapOcToStr(pOc, &pBervOc[i].lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pBervOc[i].lberbv_len = VmDirStringLenA(pBervOc[i].lberbv_val);
        pBervOc[i].bOwnBvVal = TRUE;
        i += 1;
    }

    dwError = VmDirEntryAddBervArrayAttribute(
            pEntry, ATTR_OBJECTCLASSES, pBervOc, usNumOc);
    BAIL_ON_VMDIR_ERROR(dwError);

    i = 0;
    LwRtlHashMapResetIter(&iter);
    while (LwRtlHashMapIterate(pLdapSchema->contentRules, &iter, &pair))
    {
        PVDIR_LDAP_CONTENT_RULE pCr = (PVDIR_LDAP_CONTENT_RULE)pair.pValue;
        dwError = VmDirLdapCrToStr(pCr, &pBervCr[i].lberbv_val);
        BAIL_ON_VMDIR_ERROR(dwError);

        pBervCr[i].lberbv_len = VmDirStringLenA(pBervCr[i].lberbv_val);
        pBervCr[i].bOwnBvVal = TRUE;
        i += 1;
    }

    dwError = VmDirEntryAddBervArrayAttribute(
            pEntry, ATTR_DITCONTENTRULES, pBervCr, usNumCr);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntry = pEntry;

cleanup:
    VmDirSchemaCtxRelease(pSchemaCtx);
    VmDirFreeBervalArrayContent(pBervAt, usNumAt);
    VmDirFreeBervalArrayContent(pBervOc, usNumOc);
    VmDirFreeBervalArrayContent(pBervCr, usNumCr);
    VMDIR_SAFE_FREE_MEMORY(pBervAt);
    VMDIR_SAFE_FREE_MEMORY(pBervOc);
    VMDIR_SAFE_FREE_MEMORY(pBervCr);
    return dwError;

error:
    VmDirFreeEntry(pEntry);
    goto cleanup;
}
