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

// Relevant RFC for this code: RFC 4514, Lightweight Directory Access Protocol (LDAP): String Representation of
// Distinguished Names

DWORD
_VmDirFindNextRdn(
    PCSTR pszDn,
    ber_len_t len,
    int *next)
{
    DWORD retVal = 0;
    int i = 0;
    BOOLEAN bEscaped = FALSE;

    for (i = 0; i < len; i++)
    {
        int c = pszDn[i];
        if (!bEscaped)
        {
            if (c == RDN_SEPARATOR_CHAR)
            {
                break;
            }
            else if (c == RDN_VALUE_ESC_CHAR)
            {
                bEscaped = TRUE;
            }
        }
        else
        {
            if (c == RDN_VALUE_ESC_CHAR)
            {
                bEscaped = FALSE;
            }
        }
    }

    if (bEscaped)
    {
        retVal = VMDIR_ERROR_INVALID_DN;
        BAIL_ON_VMDIR_ERROR(retVal);
    }

    *next = i; // i is at separator or end of string

cleanup:
    return retVal;

error:
    goto cleanup;
}

DWORD
VmDirGetParentDN(
    VDIR_BERVALUE * dn,
    VDIR_BERVALUE * pdn
    )
{
    int retVal = LDAP_SUCCESS;
    int i = 0;
    VDIR_BERVALUE bv = VDIR_BERVALUE_INIT;

    memset(pdn, 0, sizeof(VDIR_BERVALUE));

    retVal = _VmDirFindNextRdn(dn->lberbv.bv_val, dn->lberbv.bv_len, &i);
    BAIL_ON_VMDIR_ERROR(retVal);

    if (!(i < dn->lberbv.bv_len)) // No parent
        goto cleanup;

    retVal = VmDirAllocateStringA(dn->lberbv.bv_val + i + 1, &bv.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(retVal);

    bv.bOwnBvVal = TRUE;
    bv.lberbv.bv_len = VmDirStringLenA(bv.lberbv.bv_val);

    if (dn->bvnorm_len)
    {
        retVal = _VmDirFindNextRdn(dn->bvnorm_val, dn->bvnorm_len, &i);
        BAIL_ON_VMDIR_ERROR(retVal);

        if (i < dn->bvnorm_len)
        {
            retVal = VmDirAllocateStringA(dn->bvnorm_val + i + 1, &bv.bvnorm_val);
            BAIL_ON_VMDIR_ERROR(retVal);

            bv.bvnorm_len = VmDirStringLenA(bv.bvnorm_val);
        }
    }

    *pdn = bv;

cleanup:
    return retVal;

error:
    VmDirFreeBervalContent( &bv );
    goto cleanup;
}

DWORD
VmDirGetRdn(
    VDIR_BERVALUE * dn,
    VDIR_BERVALUE * rdn
    )
{
    int retVal = LDAP_SUCCESS;
    int i = 0;

    memset(rdn, 0, sizeof(VDIR_BERVALUE));

    retVal = _VmDirFindNextRdn(dn->lberbv.bv_val, dn->lberbv.bv_len, &i);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirAllocateStringOfLenA(dn->lberbv.bv_val, i, &rdn->lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(retVal);

    rdn->bOwnBvVal = TRUE;
    rdn->lberbv.bv_len = VmDirStringLenA(rdn->lberbv.bv_val);

    if (dn->bvnorm_len)
    {
        retVal = _VmDirFindNextRdn(dn->bvnorm_val, dn->bvnorm_len, &i);
        BAIL_ON_VMDIR_ERROR(retVal);

        retVal = VmDirAllocateStringOfLenA(dn->bvnorm_val, i, &rdn->bvnorm_val);
        BAIL_ON_VMDIR_ERROR(retVal);

        rdn->bvnorm_len = VmDirStringLenA(rdn->bvnorm_val);
    }

cleanup:

    return retVal;

error:

    VmDirFreeBervalContent( rdn );

    goto cleanup;
}

DWORD
VmDirRdnToNameValue(
    VDIR_BERVALUE * rdn,
    PSTR *ppszName,
    PSTR *ppszValue
    )
{
    DWORD dwError = 0;
    PCSTR pszStr = NULL;
    PCSTR pszEqual = NULL;
    PSTR pszName = NULL;
    PSTR pszValue = NULL;

    if (rdn->bvnorm_val)
        pszStr = rdn->bvnorm_val;
    else
        pszStr = rdn->lberbv.bv_val;

    pszEqual = strchr(pszStr, '=');
    assert(pszEqual != NULL);

    dwError = VmDirAllocateStringOfLenA(pszStr, (DWORD)(pszEqual - pszStr), &pszName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszEqual + 1, &pszValue);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszName = pszName;
    *ppszValue = pszValue;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszName);
    VMDIR_SAFE_FREE_STRINGA(pszValue);
    goto cleanup;
}

DWORD
VmDirCatDN(
    PVDIR_BERVALUE pRdn,
    PVDIR_BERVALUE pDn,
    PVDIR_BERVALUE pResult
    )
{
    DWORD dwError = 0;
    PSTR pszDn = NULL;

    if (pRdn->lberbv.bv_len == 0 || pDn->lberbv.bv_len == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszDn, "%s,%s", pRdn->lberbv.bv_val, pDn->lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pResult->lberbv.bv_val = pszDn;
    pResult->lberbv.bv_len = VmDirStringLenA(pszDn);
    pResult->bOwnBvVal = TRUE;
    pszDn = NULL;

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDn);
    return dwError;

error:
    goto cleanup;
}
