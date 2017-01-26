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



/*
 * Module Name: Directory middle layer
 *
 * Filename: krb.c
 *
 * Abstract:
 *
 * kerberos support
 *
 */

#include "includes.h"

#ifndef VMKDC_DEFAULT_KVNO
#define VMKDC_DEFAULT_KVNO 1
#endif

static
DWORD
_VmDirKrbCreateKeyBlob(
    PVDIR_BERVALUE      pBervPrincipalName,
    PVDIR_BERVALUE      pBervPasswd,
    DWORD               dwKvno,
    PVDIR_BERVALUE      pOutKeyBlob
    );

/*
 * TODO, need to revisit this function to confirm assumptions and improve error to reject
 * TODO, invalid/unexpected realm.
 *
 * Simple DN To Realm function.
 * Assume DC components are located at the TOP of DIT:
 * i.e. cn=users,dc=vsphere,dc=local (GOOD) => VSPHERE.LOCAL
 * i.e.          dc=vsphere,dc=local (GOOD) => VSPHERE.LOCAL
 * i.e. dc=123,ou=IT,dc=456 (BAD and NOT covered)
 */
DWORD
VmDirKrbSimpleDNToRealm(
    PVDIR_BERVALUE  pBervDN,
    PSTR*           ppszRealm
    )
{
    static PCSTR    pszDCSep = "dc=";

    DWORD       dwError = 0;
    PSTR        pszLocalRealm = NULL;
    PSTR        pszSep = NULL;
    CHAR        pszRealmBuf[VMDIR_MAX_REALM_LEN+1] = {0};
    size_t      iRealmLen = 0;
    PCSTR       pszDN = NULL;
    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    if (pBervDN == NULL || ppszRealm == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pBervDN->bvnorm_val == NULL)
    {
        dwError = VmDirSchemaCtxAcquire(&pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirNormalizeDN( pBervDN, pSchemaCtx );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszDN = pBervDN->bvnorm_val;

    pszSep = VmDirStringCaseStrA(pszDN, pszDCSep);
    while (pszSep != NULL)
    {
        PSTR    pszNextSep = VmDirStringChrA(pszSep+3, ',');
        size_t  iDCLen = 0;

        if (pszNextSep == NULL)
        {
            iDCLen = VmDirStringLenA(pszSep + 3);
            dwError = VmDirCopyMemory(  (PVOID) (pszRealmBuf+iRealmLen),
                                        (VMDIR_MAX_REALM_LEN - iRealmLen),
                                        (PVOID) (pszSep + 3),
                                        iDCLen);
            BAIL_ON_VMDIR_ERROR(dwError);

            iRealmLen += iDCLen;
            pszSep = NULL;
        }
        else
        {
            iDCLen = pszNextSep - pszSep - 3;
            dwError = VmDirCopyMemory(  (PVOID) (pszRealmBuf+iRealmLen),
                                        (VMDIR_MAX_REALM_LEN - iRealmLen),
                                        (PVOID) (pszSep + 3),
                                        iDCLen);
            BAIL_ON_VMDIR_ERROR(dwError);

            pszRealmBuf[iRealmLen + iDCLen] = '.';
            iRealmLen += (iDCLen + 1);

            pszSep = VmDirStringCaseStrA(pszNextSep, pszDCSep);
            if (pszSep == NULL)
            {   // DC RDN NOT at the top of DIT
                dwError = ERROR_INVALID_REALM;
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }

    if (iRealmLen == 0)
    {
        dwError = ERROR_INVALID_REALM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszRealmBuf, &pszLocalRealm);
    BAIL_ON_VMDIR_ERROR(dwError);

    {   // realm name upper case
        size_t iCnt=0;
        size_t iLen = VmDirStringLenA(pszLocalRealm);

        for (iCnt = 0; iCnt < iLen; iCnt++)
        {
            VMDIR_ASCII_LOWER_TO_UPPER(pszLocalRealm[iCnt]);
        }
    }

    *ppszRealm = pszLocalRealm;

cleanup:

    if (pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pSchemaCtx);
    }

    return dwError;

error:

    VmDirLog(LDAP_DEBUG_ANY, "VmDirKrbSimpleDNToRealm failed. (%u)(%s)", dwError, VDIR_SAFE_STRING(pszDN));

    VMDIR_SAFE_FREE_MEMORY(pszLocalRealm);

    goto cleanup;
}

/*
 * Create krb ATTR_KRB_PRINCIPAL_KEY if -
 *  1. Entry contain ATTR_KRB_UPN &&
 *  2. Entry ATTR_KRB_UPN is the same as gVmdirKrbGlobals.pszRealm
 */
DWORD
VmDirKrbUPNKeySet(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    PVDIR_BERVALUE   pBervPasswd        // this contains clear text password
    )
{
    DWORD           dwError = 0;
    VDIR_BERVALUE   bervKeyBlob = VDIR_BERVALUE_INIT;
    PVDIR_ATTRIBUTE pAttrUPN = NULL;
    DWORD           kvno = VMKDC_DEFAULT_KVNO;
    PVDIR_ATTRIBUTE pOldKeyBlob = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PSTR            pszUpnName = NULL;

    if ( !pOperation || !pEntry || !pBervPasswd )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pAttrUPN = VmDirFindAttrByName(pEntry, ATTR_KRB_UPN);
    if ( pAttrUPN && pAttrUPN->numVals == 1
         &&
         pAttrUPN->vals[0].lberbv.bv_len > 0
         &&
         gVmdirKrbGlobals.pszRealm != NULL
       )
    {
        pOldKeyBlob = VmDirFindAttrByName(pEntry, ATTR_KRB_PRINCIPAL_KEY);
        if (pOldKeyBlob && pOldKeyBlob->numVals == 1 &&
            pAttrUPN->vals[0].lberbv.bv_len > 0)
        {
            /* Decode existing key to get kvno value */
            dwError = VmDirKeySetGetKvno(
                          pAttrUPN->vals[0].lberbv.bv_val,
                          (DWORD) pAttrUPN->vals[0].lberbv.bv_len,
                          &kvno);
        }

        pszUpnName = pAttrUPN[0].vals[0].lberbv.bv_val;

        /* Lookup UPN to obtain KRB_PRINCIPAL_KEY */
        dwError = VmDirSimpleEqualFilterInternalSearch(
                      "",
                      LDAP_SCOPE_SUBTREE,
                      ATTR_KRB_UPN,
                      pszUpnName,
                      &entryArray);
        BAIL_ON_VMDIR_ERROR(dwError);

        if (entryArray.iSize == 1)
        {
            pOldKeyBlob = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_KRB_PRINCIPAL_KEY);
        }

        if (pOldKeyBlob && pOldKeyBlob->numVals == 1 &&
            pOldKeyBlob->vals[0].lberbv.bv_len > 0)
        {
            /* Decode existing key to get kvno value */
            dwError = VmDirKeySetGetKvno(
                          pOldKeyBlob->vals[0].lberbv.bv_val,
                          (DWORD) pOldKeyBlob->vals[0].lberbv.bv_len,
                          &kvno);
            BAIL_ON_VMDIR_ERROR(dwError);
            kvno++;
        }

        if ( pBervPasswd->lberbv.bv_len > 0 )
        {
            // create key blob
            dwError = _VmDirKrbCreateKeyBlob(
                            &(pAttrUPN->vals[0]),
                            pBervPasswd,
                            kvno,
                            &bervKeyBlob);
            BAIL_ON_VMDIR_ERROR(dwError);

            switch ( pOperation->reqCode)
            {
                case LDAP_REQ_ADD:
                    // add krbPrincipalKey attribute
                    dwError = VmDirEntryAddSingleValueAttribute(
                                    pEntry,
                                    ATTR_KRB_PRINCIPAL_KEY,
                                    bervKeyBlob.lberbv.bv_val,
                                    bervKeyBlob.lberbv.bv_len);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;

                case LDAP_REQ_MODIFY:
                    // add mod structure to operation->request.modifyReq
                    dwError = VmDirAppendAMod(  pOperation,
                                                MOD_OP_REPLACE,
                                                ATTR_KRB_PRINCIPAL_KEY,
                                                ATTR_KRB_PRINCIPAL_KEY_LEN,
                                                bervKeyBlob.lberbv.bv_val,
                                                bervKeyBlob.lberbv.bv_len);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;

                default:
                    break;
            }
        }
        else
        {   // no clear password value, delete corresponding krb key
            switch ( pOperation->reqCode)
            {
                case LDAP_REQ_MODIFY:
                    // add mod structure to operation->request.modifyReq
                    dwError = VmDirAppendAMod(  pOperation,
                                                MOD_OP_DELETE,
                                                ATTR_KRB_PRINCIPAL_KEY,
                                                ATTR_KRB_PRINCIPAL_KEY_LEN,
                                                NULL,
                                                0);
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;

                default:
                    break;
            }
        }
    }

cleanup:

    VmDirFreeBervalContent(&bervKeyBlob);

    return dwError;

error:

    VmDirLog(LDAP_DEBUG_ANY, "VmDirKrbUPNKeySet failed. (%u)(%s)", dwError, VDIR_SAFE_STRING(pEntry->dn.lberbv.bv_val));

    goto cleanup;
}

static
DWORD
_VmDirKrbCreateKeyBlob(
        PVDIR_BERVALUE      pBervPrincipalName,
        PVDIR_BERVALUE      pBervPasswd,
        DWORD               dwKvno,
        PVDIR_BERVALUE      pOutKeyBlob
    )
{
    DWORD       dwError = 0;
    PBYTE       pKeyBlob = NULL;
    DWORD       dwKeyLen = 0;

    assert ( pBervPrincipalName && pBervPasswd && pOutKeyBlob );

    // WARNING, WARNING, WARNING, WARNING, WARNING, WARNING, WARNING, WARNING, WARNING
    // gVmdirKrbGlobals is NOT protected for now.  It is initialized during server startup
    // and should never change after since.
    if (gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len > 0)
    {
        dwError = VmKdcStringToKeysEncrypt(
                      pBervPrincipalName->lberbv.bv_val,
                      pBervPasswd->lberbv.bv_val,
                      gVmdirKrbGlobals.bervMasterKey.lberbv.bv_val,
                      (DWORD)gVmdirKrbGlobals.bervMasterKey.lberbv.bv_len,
                      dwKvno,
                      &pKeyBlob,
                      &dwKeyLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        pOutKeyBlob->lberbv.bv_val = pKeyBlob;
        pOutKeyBlob->lberbv.bv_len = dwKeyLen;
        pOutKeyBlob->bOwnBvVal = TRUE;
        pKeyBlob = NULL;
    }

cleanup:

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pKeyBlob);

    goto cleanup;
}
