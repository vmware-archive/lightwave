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

/*
 * List of schema entry attributes we want to copy from partner.
 *
 * ATTR_ATTR_META_DATA is used to construct Mods for internal modification.
 * ATTR_USN_CHANGED    is replaced by local USN.
 */
static
PSTR ppszSchemaEntryAttrs[] =
{
        "attributeTypes",
        "dITContentRules",
        "dITStructureRules",
        "ldapSyntaxes",
        "matchingRules",
        "nameForms",
        "objectClasses",
        ATTR_ATTR_META_DATA,
        ATTR_CREATETIMESTAMP,
        ATTR_MODIFYTIMESTAMP,
        ATTR_OBJECT_GUID,
        ATTR_USN_CHANGED,
        NULL
};

static
DWORD
_OpenLdapConnection(
        PCSTR pszFQDomainName,
        PCSTR pszUsername,
        PCSTR pszPassword,
        PCSTR pszReplURI,
        LDAP **ppLd
        )
{
    DWORD dwError = 0;
    PSTR pszPartnerHostName = NULL;
    PSTR pszUPN = NULL;
    LDAP *pLd = NULL;

    dwError = VmDirReplURIToHostname((PSTR)pszReplURI, &pszPartnerHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
            &pszUPN,
            "%s@%s",
            pszUsername,
            pszFQDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
            &pLd,
            pszPartnerHostName,
            pszUPN,
            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLd = pLd;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPartnerHostName);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );

    if (pLd)
    {
        ldap_unbind_ext_s(pLd,NULL,NULL);
    }
    goto cleanup;
}

static
DWORD
_GetSchemaEntryFromPartner(
        LDAP *pLd,
        LDAPMessage **ppResult,
        LDAPMessage **ppEntry
        )
{
    DWORD dwError = 0;
    PCSTR pcszFilter = "(objectclass=*)";
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    struct timeval tv = {0};
    struct timeval *pTv = NULL;

    if (gVmdirGlobals.dwLdapSearchTimeoutSec > 0)
    {
        tv.tv_sec =  gVmdirGlobals.dwLdapSearchTimeoutSec;
        pTv = &tv;
    }

    dwError = ldap_search_ext_s(
            pLd,
            SUB_SCHEMA_SUB_ENTRY_DN,
            LDAP_SCOPE_BASE,
            pcszFilter,
            ppszSchemaEntryAttrs,
            FALSE, /* get values      */
            NULL,  /* server controls */
            NULL,  /* client controls */
            pTv,  /* timeout         */
            0,     /* size limit      */
            &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Note: searching by name should yield just one
    if (ldap_count_entries(pLd, pResult) != 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pEntry = ldap_first_entry(pLd, pResult);
    *ppEntry = pEntry;
    *ppResult = pResult;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );

    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    goto cleanup;
}

static
DWORD
_CreateCopyOperation(
        LDAPMessage *pEntry,
        PVDIR_OPERATION pLdapOp
        )
{
    DWORD dwError = 0;

    dwError = VmDirInitStackOperation(
            pLdapOp,
            VDIR_OPERATION_TYPE_REPL,
            LDAP_REQ_MODIFY,
            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLdapOp->pBEIF = VmDirBackendSelect(NULL);
    assert(pLdapOp->pBEIF);

    pLdapOp->reqDn.lberbv.bv_val = SUB_SCHEMA_SUB_ENTRY_DN;
    pLdapOp->reqDn.lberbv.bv_len = VmDirStringLenA(SUB_SCHEMA_SUB_ENTRY_DN);
    pLdapOp->request.modifyReq.dn.lberbv.bv_val = pLdapOp->reqDn.lberbv.bv_val;
    pLdapOp->request.modifyReq.dn.lberbv.bv_len = pLdapOp->reqDn.lberbv.bv_len;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );

    goto cleanup;
}

static
VOID
_FreeStringPair(
    PLW_HASHMAP_PAIR pPair,
    LW_PVOID pUnused
    )
{
    VMDIR_SAFE_FREE_MEMORY(pPair->pKey);
    VMDIR_SAFE_FREE_MEMORY(pPair->pValue);
}

static
DWORD
_PopulateOperationModAttributes(
        LDAP *pLd,
        LDAPMessage *pEntry,
        PVDIR_OPERATION pLdapOp
        )
{
    DWORD dwError = 0;
    struct berval** ppValues = NULL;
    PLW_HASHMAP pHashMap = NULL;

    PSTR pszBuf = NULL;
    PSTR pszAttrName = NULL;
    PSTR pszAttrMetaData = NULL;
    PSTR pszAttrUsnlessMetaData = NULL;
    PSTR pszAttrNewMetaData = NULL;
    PSTR pszKey = NULL;
    PSTR pszValue = NULL;
    USN localUsn = 0;
    PVDIR_BERVALUE pBerValue = NULL;
    size_t iBerValueSize = 0;
    DWORD i = 0, j = 0;

    dwError = LwRtlCreateHashMap(
            &pHashMap,
            LwRtlHashDigestPstrCaseless,
            LwRtlHashEqualPstrCaseless,
            NULL
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = pLdapOp->pBEIF->pfnBEGetNextUSN(pLdapOp->pBECtx, &localUsn);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!(ppValues = ldap_get_values_len(pLd, pEntry, ATTR_ATTR_META_DATA)))
    {
        dwError = VMDIR_ERROR_SCHEMA_BAD_METADATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i = 0; ppValues[i] != NULL; i++)
    {
        dwError = VmDirAllocateStringA(ppValues[i]->bv_val, &pszBuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszAttrName = VmDirStringTokA(pszBuf, ":", &pszAttrMetaData);
        VmDirStringTokA(pszAttrMetaData, ":", &pszAttrUsnlessMetaData);

        dwError = VmDirAllocateStringPrintf(
                &pszAttrNewMetaData,
                "%ld:%s",
                localUsn,
                pszAttrUsnlessMetaData);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(pszAttrName, &pszKey);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = LwRtlHashMapInsert(pHashMap, pszKey, pszAttrNewMetaData, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszBuf);
    }
    ldap_value_free_len(ppValues);
    ppValues = NULL;

    for (i = 0; ppszSchemaEntryAttrs[i] != NULL; i++)
    {
        if (VmDirStringCompareA(ppszSchemaEntryAttrs[i], ATTR_ATTR_META_DATA, FALSE) != 0)
        {
            ppValues = ldap_get_values_len(pLd, pEntry, ppszSchemaEntryAttrs[i]);
            if (ppValues)
            {
                iBerValueSize = ldap_count_values_len(ppValues);
                dwError = VmDirAllocateMemory(
                        sizeof(VDIR_BERVALUE) * iBerValueSize,
                        (PVOID*)&pBerValue);
                BAIL_ON_VMDIR_ERROR(dwError);

                if (VmDirStringCompareA(ppszSchemaEntryAttrs[i], ATTR_USN_CHANGED, FALSE) == 0)
                {
                    assert(iBerValueSize == 1);

                    dwError = VmDirAllocateStringPrintf(&pBerValue[0].lberbv_val, "%ld", localUsn);
                    BAIL_ON_VMDIR_ERROR(dwError);
                    pBerValue[0].lberbv_len = VmDirStringLenA(pBerValue[0].lberbv_val);
                    pBerValue[0].bOwnBvVal = TRUE;
                }
                else
                {
                    for (j = 0; j < iBerValueSize; j++)
                    {
                        dwError = VmDirAllocateStringA(ppValues[j]->bv_val, &pBerValue[j].lberbv_val);
                        BAIL_ON_VMDIR_ERROR(dwError);
                        pBerValue[j].lberbv_len = ppValues[j]->bv_len;
                        pBerValue[j].bOwnBvVal = TRUE;
                    }
                }

                dwError = VmDirOperationAddModReq(
                        pLdapOp,
                        MOD_OP_REPLACE,
                        ppszSchemaEntryAttrs[i],
                        pBerValue,
                        iBerValueSize);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = LwRtlHashMapFindKey(
                        pHashMap,
                        (PVOID*)&pszValue,
                        ppszSchemaEntryAttrs[i]);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = VmDirStringCpyA(
                        pLdapOp->request.modifyReq.mods->attr.metaData,
                        VMDIR_MAX_ATTR_META_DATA_LEN,
                        pszValue);
                BAIL_ON_VMDIR_ERROR(dwError);

                ldap_value_free_len(ppValues);
                ppValues = NULL;
                for (j = 0; j < iBerValueSize; j++)
                {
                    VmDirFreeBervalContent(&pBerValue[j]);
                }
                VMDIR_SAFE_FREE_MEMORY(pBerValue);
            }
        }
    }

cleanup:
    LwRtlHashMapClear(pHashMap, _FreeStringPair, NULL);
    LwRtlFreeHashMap(&pHashMap);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );

    VMDIR_SAFE_FREE_MEMORY(pszBuf);
    VMDIR_SAFE_FREE_MEMORY(pszAttrNewMetaData);
    if (ppValues)
    {
        ldap_value_free_len(ppValues);
    }
    for (j = 0; j < iBerValueSize; j++)
    {
        VmDirFreeBervalContent(&pBerValue[j]);
    }
    VMDIR_SAFE_FREE_MEMORY(pBerValue);
    goto cleanup;
}

static
DWORD
 _VmDirCopyAttrToStrList(
    PVDIR_ENTRY         pEntry,
    PCSTR               pszAttrName,
    PVMDIR_STRING_LIST  pStrList
    )
{
    DWORD           dwError = 0;
    size_t          i = 0;
    PVDIR_ATTRIBUTE pAttr = NULL;

    pAttr = VmDirFindAttrByName(pEntry, (PSTR)pszAttrName);
    if (!pAttr)
    {
        dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    for (i=0; i< pAttr->numVals; i++)
    {
        dwError = VmDirStringListAddStrClone(pAttr->vals[i].lberbv_val, pStrList);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirGetEntrySchemaDefStr(
    PVDIR_ENTRY                pEntry,
    PVMDIR_LDAP_SCHEMA_DEF_STR pSchemaDefs
    )
{
    DWORD dwError = 0;

    dwError = _VmDirCopyAttrToStrList(pEntry, ATTR_ATTRIBUTETYPES, pSchemaDefs->pATStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCopyAttrToStrList(pEntry, ATTR_OBJECTCLASSES, pSchemaDefs->pOCStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirCopyAttrToStrList(pEntry, ATTR_DITCONTENTRULES, pSchemaDefs->pCRStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirAddOneOPMod(
    PVDIR_OPERATION pOperation,
    int             modOpType,
    PCSTR           pszName,
    PVMDIR_STRING_LIST  pStrList
    )
{
    DWORD           dwError = 0;
    DWORD           dwCnt = 0;
    PVDIR_BERVALUE  pBerValue = NULL;

    if (pStrList->dwCount < 1 || pStrList->dwCount > USHRT_MAX)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

     dwError = VmDirAllocateMemory(
             sizeof(VDIR_BERVALUE) * (pStrList->dwCount+1),
             (PVOID*)&pBerValue);
     BAIL_ON_VMDIR_ERROR(dwError);

     VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Modify schema entry:");
     VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "OP type %d (ADD/0,DEL/1), %s", modOpType ,pszName);

     for (dwCnt = 0; dwCnt < pStrList->dwCount; dwCnt++)
     {
         dwError = VmDirAllocateStringA(pStrList->pStringList[dwCnt], &pBerValue[dwCnt].lberbv_val);
         BAIL_ON_VMDIR_ERROR(dwError);
         pBerValue[dwCnt].lberbv_len = VmDirStringLenA(pBerValue[dwCnt].lberbv_val);
         pBerValue[dwCnt].bOwnBvVal = TRUE;

         VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "%s", pStrList->pStringList[dwCnt]);
     }

     dwError = VmDirOperationAddModReq(
             pOperation,
             modOpType,
             (PSTR)pszName,
             pBerValue,
             (size_t)pStrList->dwCount);
     BAIL_ON_VMDIR_ERROR(dwError);
     pBerValue = NULL; // pOperation takes over pBerValue

cleanup:
    return dwError;

error:
    if (pBerValue)
    {
        VmDirFreeBervalArrayContent(pBerValue, (USHORT)pStrList->dwCount);
        VMDIR_SAFE_FREE_MEMORY(pBerValue);
    }
    goto cleanup;
}

static
DWORD
_VmDirSchemaDefToOPMod(
    PVDIR_OPERATION             pOperation,
    PVMDIR_LDAP_SCHEMA_MOD_STR  pSchemaModStr
    )
{
    struct
    {
        int                 opMod;
        PCSTR               pszName;
        PVMDIR_STRING_LIST  pStrList;
    }
    values[] =
    {
        {LDAP_MOD_DELETE, ATTR_ATTRIBUTETYPES,  pSchemaModStr->pDelATStrList},
        {LDAP_MOD_ADD,    ATTR_ATTRIBUTETYPES,  pSchemaModStr->pAddATStrList},
        {LDAP_MOD_DELETE, ATTR_OBJECTCLASSES,   pSchemaModStr->pDelOCStrList},
        {LDAP_MOD_ADD,    ATTR_OBJECTCLASSES,   pSchemaModStr->pAddOCStrList},
        {LDAP_MOD_DELETE, ATTR_DITCONTENTRULES, pSchemaModStr->pDelCRStrList},
        {LDAP_MOD_ADD,    ATTR_DITCONTENTRULES, pSchemaModStr->pAddCRStrList}
    };
    int   iValue = 0;
    DWORD dwError = 0;

    for (iValue = sizeof(values)/sizeof(values[0]) -1; iValue >= 0; iValue--)
    {
        if (values[iValue].pStrList->dwCount > 0)
        {
            dwError = _VmDirAddOneOPMod(pOperation,
                                        values[iValue].opMod,
                                        values[iValue].pszName,
                                        values[iValue].pStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirCopyPartnerSchema(
        PCSTR pszFQDomainName,
        PCSTR pszUsername,
        PCSTR pszPassword,
        PCSTR pszReplURI
        )
{
    DWORD dwError = 0;
    LDAP *pLd = NULL;
    LDAPMessage *pResult = NULL;
    LDAPMessage *pEntry = NULL;
    VDIR_OPERATION ldapOp = {0};
    BOOLEAN bHasTxn = FALSE;
    DWORD dwDeadlockRetry = 0;

    dwError = _OpenLdapConnection(
            pszFQDomainName,
            pszUsername,
            pszPassword,
            pszReplURI,
            &pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _GetSchemaEntryFromPartner(pLd, &pResult, &pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

txnretry:
    if (bHasTxn)
    {
        ldapOp.pBEIF->pfnBETxnAbort(ldapOp.pBECtx);
        bHasTxn = FALSE;
    }

    if (dwDeadlockRetry++ > MAX_DEADLOCK_RETRIES)
    {
        dwError = LDAP_LOCK_DEADLOCK;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _CreateCopyOperation(pEntry, &ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldapOp.pBEIF->pfnBETxnBegin(ldapOp.pBECtx, VDIR_BACKEND_TXN_WRITE);
    BAIL_ON_VMDIR_ERROR(dwError);
    bHasTxn = TRUE;

    dwError = _PopulateOperationModAttributes(pLd, pEntry, &ldapOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry(&ldapOp) ? ldapOp.ldapResult.errCode : 0;
    if (dwError == LDAP_LOCK_DEADLOCK)
    {
        VmDirFreeOperationContent(&ldapOp);
        goto txnretry;
    }
    else if (dwError && ldapOp.ldapResult.vmdirErrCode)
    {
        dwError = ldapOp.ldapResult.vmdirErrCode;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldapOp.pBEIF->pfnBETxnCommit(ldapOp.pBECtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pLd)
    {
        ldap_unbind_ext_s(pLd,NULL,NULL);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }
    VmDirFreeOperationContent(&ldapOp);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
            "%s,%d failed, error(%d)", __FUNCTION__, __LINE__, dwError );

    if (bHasTxn)
    {
        ldapOp.pBEIF->pfnBETxnAbort(ldapOp.pBECtx);
    }
    goto cleanup;
}

/*
 * Compare DB and file schema definition.
 * Generate LDAP VDIR_MODIFICATION to perform internal modify.
 */
DWORD
VmDirSchemaPatchSetOPMod(
    PVDIR_OPERATION     pOperation,
    PVDIR_ENTRY         pEntry,
    PCSTR               pszSchemaFile
    )
{
    DWORD dwError = 0;
    PVMDIR_LDAP_SCHEMA_STRUCT   pLdapSchema = NULL;
    PVMDIR_LDAP_SCHEMA_STRUCT   pFileSchema = NULL;
    VMDIR_LDAP_SCHEMA_DEF_STR   schemaDefs = {0};
    VMDIR_LDAP_SCHEMA_MOD_STR   schemaModStr = {0};

    if (!pOperation || !pEntry || !pszSchemaFile)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitModStrContent(&schemaModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInitLdapSchemaDefsContent(&schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetEntrySchemaDefStr(pEntry,&schemaDefs);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSchemaFromDefStr(&schemaDefs, &pLdapSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSchemaFromLocalFile(pszSchemaFile, &pFileSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAnalyzeSchemaUpgrade(pLdapSchema, pFileSchema, &schemaModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirSchemaDefToOPMod(pOperation, &schemaModStr);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirFreeLdapSchemaDefsContent(&schemaDefs);
    VmDirFreeModStrContent(&schemaModStr);
    VmDirFreeLdapSchemaStruct(pLdapSchema);
    VmDirFreeLdapSchemaStruct(pFileSchema);
    return dwError;

error:
    goto cleanup;
}
