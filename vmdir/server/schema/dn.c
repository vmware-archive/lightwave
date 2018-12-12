/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

// RFC 4514, Lightweight Directory Access Protocol (LDAP): String Representation of Distinguished Names

static
DWORD
_VmDirNormalizeRDN(
    LDAPRDN             pRDN,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

static
DWORD
_VmDirNormalizeDN(
    PVDIR_LDAP_DN       pLdapDN,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    );

/*
 * RFC 4514 syntax validation
 */
DWORD
VmDirDNStrToInternalDN(
    PVDIR_LDAP_DN   pLdapDN
    )
{
    DWORD   dwError = 0;

    if (!pLdapDN || !pLdapDN->dn.lberbv_val)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pLdapDN->internalDN)
    {
        goto cleanup;
    }

    // perform syntax validation
    dwError = ldap_str2dn(pLdapDN->dn.lberbv_val, &pLdapDN->internalDN, LDAP_DN_FORMAT_LDAPV3);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d) (%s)",
        __FUNCTION__,
        dwError,
        (pLdapDN && pLdapDN->dn.lberbv_val) ? pLdapDN->dn.lberbv_val : "NULL");
    goto cleanup;
}

/*
 * validate syntax + construct normalize value
 */
DWORD
VmDirNormDN(
    PVDIR_LDAP_DN       pLdapDN,
    PVDIR_SCHEMA_CTX    pSchemaCtx	/* optional */
    )
{
    DWORD   dwError = 0;
    PVDIR_SCHEMA_CTX    pCtx = pSchemaCtx;

    if (!pLdapDN || !pLdapDN->dn.lberbv_val)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pLdapDN->dn.bvnorm_val)
    {
        goto cleanup;
    }

    if (!pCtx)
    {
        dwError = VmDirSchemaCtxAcquire(&pCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pLdapDN->internalDN)
    {
        dwError = VmDirDNStrToInternalDN(pLdapDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirNormalizeDN(pLdapDN, pCtx);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (pCtx && pCtx != pSchemaCtx)
    {
        VmDirSchemaCtxRelease(pCtx);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * construct normalized parent dn.
 * NULL if no parent.
 */
DWORD
VmDirParentNormDN(
    PVDIR_LDAP_DN   pLdapDN
    )
{
    DWORD   dwError = 0;

    if (!pLdapDN || !pLdapDN->dn.lberbv_val)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (pLdapDN->pszParentNormDN)
    {
        goto cleanup;
    }

    dwError = VmDirNormDN(pLdapDN, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

VOID
VmDirFreeLDAPDNContent(
    PVDIR_LDAP_DN   pLdapDN
    )
{
    if (pLdapDN)
    {
        VDIR_SAFE_FREE_LDAPDN(pLdapDN->internalDN);
        VmDirFreeBervalContent(&pLdapDN->dn);
    }
}

VOID
VmDirFreeLDAPDN(
    PVDIR_LDAP_DN   pLdapDN
    )
{
    if (pLdapDN)
    {
        VmDirFreeLDAPDNContent(pLdapDN);
        VMDIR_SAFE_FREE_MEMORY(pLdapDN);
    }
}

/*
 * Normalize individual RDN
 */
static
DWORD
_VmDirNormalizeRDN(
    LDAPRDN             pRDN,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;
    DWORD   dwAVA = 0;
    DWORD   dwCnt = 0;
    PVDIR_SCHEMA_AT_DESC    pATDesc = NULL;
    VDIR_BERVALUE           bvNormVal = {0};

    for (dwAVA = 0; pRDN[dwAVA]; dwAVA++)
    {
          LDAPAVA     *pAVA = pRDN[dwAVA];

          if (dwAVA > 1)
          {   // Not yet ready to support multiple RDN case with rename operation.
              // Reject this format for now.
              BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_DN);
          }

          pATDesc = VmDirSchemaAttrNameToDesc(pSchemaCtx, pAVA->la_attr.bv_val);
          if (!pATDesc)
          {
              BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_NO_SUCH_ATTRIBUTE);
          }

          bvNormVal.lberbv_val = pAVA->la_value.bv_val;
          bvNormVal.lberbv_len = pAVA->la_value.bv_len;

          dwError = VmDirSchemaBervalNormalize(pSchemaCtx, pATDesc, &bvNormVal);
          BAIL_ON_VMDIR_ERROR(dwError);

          // lower RDN attribute name
          for (dwCnt = 0; dwCnt < pAVA->la_attr.bv_len; dwCnt++)
          {
              pAVA->la_attr.bv_val[dwCnt] = (char) tolower(pAVA->la_attr.bv_val[dwCnt]);
          }

          // length of normalized value should be smaller than original one
          if (bvNormVal.bvnorm_len > pAVA->la_value.bv_len)
          {
              BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_NORMALIZATION);
          }

          if (pAVA->la_value.bv_val != bvNormVal.bvnorm_val)
          {   // copy back to pAVA buffer only if norm has different value
              dwError = VmDirCopyMemory(
                  pAVA->la_value.bv_val,
                  pAVA->la_value.bv_len,
                  bvNormVal.bvnorm_val,
                  bvNormVal.bvnorm_len);
              BAIL_ON_VMDIR_ERROR(dwError);

              pAVA->la_value.bv_val[bvNormVal.bvnorm_len] = '\0';
              pAVA->la_value.bv_len = bvNormVal.bvnorm_len;
          }

          VmDirFreeBervalContent(&bvNormVal);
    }

cleanup:
    VmDirFreeBervalContent(&bvNormVal);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

/*
 * Normalize DN value by
 * 1. normalize individual RDNs (per attribute normalize function definition)
 * 2. ldap_dn2str: to convert/escape special chars per RFC 4514
 */
static
DWORD
_VmDirNormalizeDN(
    PVDIR_LDAP_DN       pLdapDN,
    PVDIR_SCHEMA_CTX    pSchemaCtx
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszStr2DN = NULL;
    PSTR    pszFirstComma = NULL;

    for (dwCnt=0; pLdapDN->internalDN && pLdapDN->internalDN[dwCnt]; dwCnt++)
    {
        dwError = _VmDirNormalizeRDN(pLdapDN->internalDN[dwCnt], pSchemaCtx);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    /*
     * convert from normalized LDAPDN to string format
     */
    dwError = ldap_dn2str(pLdapDN->internalDN, &pszStr2DN, LDAP_DN_FORMAT_LDAPV3);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(pszStr2DN, &pLdapDN->dn.bvnorm_val);
    BAIL_ON_VMDIR_ERROR(dwError);

    pLdapDN->dn.bvnorm_len = VmDirStringLenA(pLdapDN->dn.bvnorm_val);

    // ',' is converted to \2C in dn2str call.  so find the first comma + 1 for parent normalized DN
    if ((pszFirstComma = VmDirStringChrA(pLdapDN->dn.bvnorm_val, ',')) != NULL)
    {
        pLdapDN->pszParentNormDN = pszFirstComma + 1;
    }

cleanup:
    VMDIR_SAFE_LDAP_MEMFREE(pszStr2DN);
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%d)", __FUNCTION__, dwError);
    goto cleanup;
}

