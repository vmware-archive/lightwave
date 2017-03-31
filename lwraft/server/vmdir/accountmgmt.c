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

static
DWORD
VmDirSrvGetCallerDomain(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken,
    PSTR*                   ppszDomain
    );

DWORD
VmDirCreateAccount(
    PCSTR   pszUPNName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,        // optional?
    PCSTR   pszEntryDN
    )
{
    DWORD   dwError = 0;
    PSTR    ppszAttributes[] =
    {
            ATTR_OBJECT_CLASS,      (PSTR)OC_USER,
            ATTR_CN,                (PSTR)pszUserName,
            ATTR_USER_PASSWORD,     (PSTR)pszPassword,
            ATTR_SAM_ACCOUNT_NAME,  (PSTR)pszUserName,
            ATTR_KRB_UPN,           (PSTR)pszUPNName,
            NULL
    };

    PVDIR_SCHEMA_CTX    pSchemaCtx = NULL;

    if (    IsNullOrEmptyString(pszUPNName)                 ||
            IsNullOrEmptyString(pszUserName)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwError = VmDirSimpleEntryCreate(
                    pSchemaCtx,
                    ppszAttributes,
                    (PSTR)pszEntryDN,
                    0);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirSchemaCtxRelease(pSchemaCtx);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirCreateAccount UPN(%s) failed, (%u)",
                              VDIR_SAFE_STRING(pszUPNName), dwError);

    goto cleanup;
}

DWORD
VmDirCreateAccountEx(
    PVMDIR_SRV_ACCESS_TOKEN       pAccessToken,
    PVMDIR_USER_CREATE_PARAMS_RPC pCreateParams
    )
{
    DWORD dwError = 0;
    PVDIR_SCHEMA_CTX pSchemaCtx = NULL;
    VMDIR_USER_CREATE_PARAMS_W  createParamsW = {0};
    PVMDIR_USER_CREATE_PARAMS_A pCreateParamsA = NULL;
    PSTR  pszName_local = NULL; // Do not free
    PSTR  pszName      = NULL;
    PSTR  pszUPN_local = NULL;  // Do not free
    PSTR  pszUPN       = NULL;
    PSTR  pszAccountDN = NULL;
    PSTR  pszDomain    = NULL;
    PSTR  pszDomainDN  = NULL;
    PSTR  pszUpperDomain = NULL;
    DWORD i = 0;

    dwError = VmDirSrvValidateUserCreateParams(pCreateParams);
    BAIL_ON_VMDIR_ERROR(dwError);

    createParamsW.pwszName      = pCreateParams->pwszName;
    createParamsW.pwszAccount   = pCreateParams->pwszAccount;
    createParamsW.pwszUPN       = pCreateParams->pwszUPN;
    createParamsW.pwszFirstname = pCreateParams->pwszFirstname;
    createParamsW.pwszLastname  = pCreateParams->pwszLastname;
    createParamsW.pwszPassword  = pCreateParams->pwszPassword;

    dwError = VmDirAllocateUserCreateParamsAFromW(
                    &createParamsW,
                    &pCreateParamsA);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pCreateParamsA->pszName))
    {
        dwError = VmDirAllocateStringPrintf(
                        &pszName,
                        "%s %s",
                        pCreateParamsA->pszFirstname,
                        pCreateParamsA->pszLastname);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszName_local = pszName;
    }
    else
    {
        pszName_local = pCreateParamsA->pszName;
    }

    dwError = VmDirSrvGetCallerDomain(pAccessToken, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (IsNullOrEmptyString(pCreateParamsA->pszUPN))
    {
        if (VmDirStringChrA(pCreateParamsA->pszAccount, '@') == NULL)
        {
            dwError = VmDirAllocateStringPrintf(
                            &pszUpperDomain,
                            "%s",
                            pszDomain);
            BAIL_ON_VMDIR_ERROR(dwError);

            for (i=0; pszUpperDomain[i]; i++)
            {
                VMDIR_ASCII_LOWER_TO_UPPER(pszUpperDomain[i]);
            }

            dwError = VmDirAllocateStringPrintf(
                            &pszUPN,
                            "%s@%s",
                            pCreateParamsA->pszAccount,
                            pszUpperDomain);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            dwError = VmDirAllocateStringA(pCreateParamsA->pszAccount, &pszUPN);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        pszUPN_local = pszUPN;
    }
    else
    {
        pszUPN_local = pCreateParamsA->pszUPN;
    }

    dwError = VmDirFQDNToDN(pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(
                    &pszAccountDN,
                    "CN=%s,%s,%s",
                    pszName_local,
                    DEFAULT_USER_CONTAINER_RDN,
                    pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    do
    {
        PSTR    ppszAttributes[] =
        {
                ATTR_OBJECT_CLASS,      (PSTR)OC_USER,
                ATTR_CN,                pszName_local,
                ATTR_USER_PASSWORD,     pCreateParamsA->pszPassword,
                ATTR_SAM_ACCOUNT_NAME,  pCreateParamsA->pszAccount,
                ATTR_KRB_UPN,           pszUPN_local,
                ATTR_SN,                pCreateParamsA->pszLastname,
                ATTR_GIVEN_NAME,        pCreateParamsA->pszFirstname,
                NULL
        };

        dwError = VmDirSchemaCtxAcquire( &pSchemaCtx );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSimpleEntryCreate(
                        pSchemaCtx,
                        ppszAttributes,
                        pszAccountDN,
                        0);
        BAIL_ON_VMDIR_ERROR(dwError);

    } while (0);

cleanup:

    VmDirSchemaCtxRelease(pSchemaCtx);

    VMDIR_SAFE_FREE_MEMORY(pszName);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pszAccountDN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperDomain);

    if (pCreateParamsA)
    {
        VmDirFreeUserCreateParamsA(pCreateParamsA);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(
         VMDIR_LOG_MASK_ALL,
         "VmDirCreateAccountEx Account(%s) failed, (%u)",
         VDIR_SAFE_STRING(pCreateParamsA ? pCreateParamsA->pszAccount : NULL),
         dwError);

    goto cleanup;
}

DWORD
VmDirUPNToAccountDN(
    PCSTR       pszUPNName,
    PCSTR       pszAccountRDNAttr,
    PCSTR       pszAccountRDNValue,
    PSTR*       ppszAccountDN
    )
{
    DWORD       dwError = 0;
    PSTR        pszRealm = NULL;
    PSTR        pszLocalDomainDN = NULL;
    PSTR        pszLocalAccountDN = NULL;

    if (    IsNullOrEmptyString(pszUPNName)                 ||
            IsNullOrEmptyString(pszAccountRDNAttr)          ||
            IsNullOrEmptyString(pszAccountRDNValue)         ||
            (ppszAccountDN == NULL)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszRealm = VmDirStringRChrA(pszUPNName, VMDIR_UPN_REALM_SEPARATOR);
    if (pszRealm == NULL || pszRealm[1] == '\0')
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirFQDNToDN(pszRealm+1, &pszLocalDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                    &pszLocalAccountDN,
                    "%s=%s,%s,%s",
                    pszAccountRDNAttr,
                    pszAccountRDNValue,
                    DEFAULT_USER_CONTAINER_RDN,
                    pszLocalDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszAccountDN = pszLocalAccountDN;

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLocalDomainDN);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pszLocalAccountDN);
    goto cleanup;
}


DWORD
VmDirResetPassword(
    PCSTR       pszUPN,
    PCSTR       pszNewPassword
    )
{
    DWORD               dwError = 0;
    VDIR_BERVALUE       bvPassword = VDIR_BERVALUE_INIT;
    PSTR                pszDN = NULL;

    if ( IsNullOrEmptyString(pszUPN) || IsNullOrEmptyString(pszNewPassword) )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirUPNToDN( pszUPN, &pszDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    bvPassword.lberbv_val = (PSTR)pszNewPassword;
    bvPassword.lberbv_len = VmDirStringLenA(pszNewPassword);
    dwError = VmDirInternalEntryAttributeReplace( NULL,
                                                  pszDN,
                                                  ATTR_USER_PASSWORD,
                                                  &bvPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeBervalContent(&bvPassword);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirResetPassword (%s) failed, (%u)",
                              VDIR_SAFE_STRING(pszUPN), dwError);
    goto cleanup;
}

/*
 * Retrieve SRP Identifier's secret and salt
 */
DWORD
VmDirSRPGetIdentityData(
    PCSTR       pszUPN,
    PBYTE*      ppByteSecret,
    DWORD*      pdwSecretLen
    )
{
    DWORD               dwError = 0;
    PVDIR_ATTRIBUTE     pAttrSecret = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PBYTE               pLocalSecret = NULL;

    if ( IsNullOrEmptyString(pszUPN)    ||
         ppByteSecret == NULL           ||
         pdwSecretLen == NULL
        )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirdState() != VMDIRD_STATE_NORMAL)
    {
        /*
         * During local server promo, remote server rpc call
         * may cause the local server to crash when the backend was shutdown (swapped)
         */
        dwError = VMDIR_ERROR_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_KRB_UPN,
                    pszUPN,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        pAttrSecret = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_SRP_SECRET);
        if (!pAttrSecret)
        {
            dwError = VMDIR_ERROR_NO_SUCH_ATTRIBUTE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateAndCopyMemory(   pAttrSecret->vals[0].lberbv_val,
                                                pAttrSecret->vals[0].lberbv_len,
                                                (PVOID*)&pLocalSecret);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (entryArray.iSize == 0)
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *ppByteSecret = pLocalSecret;
    *pdwSecretLen = (DWORD) (pAttrSecret->vals[0].lberbv_len);

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY( pLocalSecret );

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirSRPGetIdentityData (%s) failed, (%u)", VDIR_SAFE_STRING(pszUPN), dwError);
    goto cleanup;
}

/*
 * Set SRP Identifier's secret on existing entry with Password set
 */
DWORD
VmDirSRPSetIdentityData(
    PCSTR       pszUPN,
    PCSTR       pszClearTextPassword
    )
{
    DWORD               dwError = 0;
    VDIR_OPERATION      op = {0};
    PSTR                pszLocalErrMsg = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ENTRY         pEntry = NULL;
    PVDIR_ATTRIBUTE     pAttrSecret = NULL;
    VDIR_BERVALUE       bvUPN = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       bvClearTextPassword = VDIR_BERVALUE_INIT;
    VDIR_BERVALUE       bervSecretBlob = VDIR_BERVALUE_INIT;


    if ( IsNullOrEmptyString(pszUPN)    ||
         IsNullOrEmptyString(pszClearTextPassword)
        )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    bvUPN.lberbv_val = (PSTR)pszUPN;
    bvUPN.lberbv_len = VmDirStringLenA(pszUPN);

    bvClearTextPassword.lberbv_val = (PSTR)pszClearTextPassword;
    bvClearTextPassword.lberbv_len = VmDirStringLenA(pszClearTextPassword);

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    "",
                    LDAP_SCOPE_SUBTREE,
                    ATTR_KRB_UPN,
                    pszUPN,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize == 1)
    {
        pAttrSecret = VmDirFindAttrByName(&(entryArray.pEntry[0]), ATTR_SRP_SECRET);
        if (pAttrSecret)
        {
            dwError = VMDIR_ERROR_ENTRY_ALREADY_EXIST;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }
    else
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = &(entryArray.pEntry[0]);

    dwError = VdirPasswordCheck(&bvClearTextPassword, pEntry);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSRPCreateSecret(&bvUPN, &bvClearTextPassword, &bervSecretBlob);
    BAIL_ON_VMDIR_ERROR(dwError);


    if (pEntry->allocType == ENTRY_STORAGE_FORMAT_PACK)
    {
        dwError = VmDirEntryUnpack( pEntry );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "VmDirSRPSetIdentityData: VmDirInitStackOperation failed: %u", dwError);

    op.pBEIF = VmDirBackendSelect(NULL);
    assert(op.pBEIF);

    op.reqDn.lberbv.bv_val = pEntry->dn.lberbv.bv_val;
    op.reqDn.lberbv.bv_len = pEntry->dn.lberbv.bv_len;
    op.request.modifyReq.dn.lberbv = op.reqDn.lberbv;

    dwError = VmDirAppendAMod( &op,
                               MOD_OP_ADD,
                               ATTR_SRP_SECRET,
                               ATTR_SRP_SECRET_LEN,
                               bervSecretBlob.lberbv_val,
                               bervSecretBlob.lberbv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry(&op);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeBervalContent(&bervSecretBlob);
    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeOperationContent(&op);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirSRPSetIdentityData (%s) failed, (%u)", VDIR_SAFE_STRING(pszUPN), dwError);
    goto cleanup;
}


/*
 * Set vmwPasswordNeverExpires (if it doesn't have a value) to TRUE
 * on the domain administrator's account.
 */
DWORD
VmDirSetAdministratorPasswordNeverExpires(
    VOID
    )
{
    DWORD               dwError = 0;
    PCSTR               pszDomainDn = NULL;
    const CHAR          szAdministrator[] = "cn=Administrator,cn=Users";
    const CHAR          szTrue[] = "TRUE";
    PSTR                pszAdministratorDn = NULL;
    VDIR_OPERATION      op = {0};
    PSTR                pszLocalErrMsg = NULL;
    VDIR_ENTRY_ARRAY    entryArray = {0};
    PVDIR_ENTRY         pEntry = NULL;
    VDIR_BERVALUE       bervBlob = VDIR_BERVALUE_INIT;

    pszDomainDn = gVmdirServerGlobals.systemDomainDN.lberbv.bv_val;
    if (pszDomainDn == NULL)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf(&pszAdministratorDn, "%s,%s", szAdministrator, pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSimpleEqualFilterInternalSearch(
                    pszDomainDn,
                    LDAP_SCOPE_SUBTREE,
                    ATTR_DN,
                    pszAdministratorDn,
                    &entryArray);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (entryArray.iSize != 1)
    {
        dwError = VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pEntry = &(entryArray.pEntry[0]);

    if (pEntry->allocType == ENTRY_STORAGE_FORMAT_PACK)
    {
        dwError = VmDirEntryUnpack( pEntry );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirInitStackOperation( &op, VDIR_OPERATION_TYPE_INTERNAL, LDAP_REQ_MODIFY, NULL);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "VmDirSetAdministratorPasswordNeverExpire: VmDirInitStackOperation failed: %u", dwError);

    op.pBEIF = VmDirBackendSelect(NULL);
    assert(op.pBEIF);

    op.reqDn.lberbv.bv_val = pEntry->dn.lberbv.bv_val;
    op.reqDn.lberbv.bv_len = pEntry->dn.lberbv.bv_len;
    op.request.modifyReq.dn.lberbv = op.reqDn.lberbv;

    bervBlob.lberbv.bv_val = (PSTR) szTrue;
    bervBlob.lberbv.bv_len = strlen(szTrue);
    dwError = VmDirAppendAMod( &op,
                               MOD_OP_REPLACE,
                               ATTR_PASSWORD_NEVER_EXPIRES,
                               ATTR_PASSWORD_NEVER_EXPIRES_LEN,
                               bervBlob.lberbv_val,
                               bervBlob.lberbv_len);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirInternalModifyEntry(&op);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VmDirFreeEntryArrayContent(&entryArray);
    VmDirFreeOperationContent(&op);
    VMDIR_SAFE_FREE_STRINGA(pszAdministratorDn);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirSetAdministratorPasswordNeverExpires failed, (%u)", dwError);
    goto cleanup;
}

static
DWORD
VmDirSrvGetCallerDomain(
    PVMDIR_SRV_ACCESS_TOKEN pAccessToken,
    PSTR*                   ppszDomain
    )
{
    DWORD dwError = 0;
    PSTR  pszCursor = NULL;
    PSTR  pszDomain = NULL;

    pszCursor = VmDirStringRChrA(
                    pAccessToken->pszUPN,
                    VMDIR_UPN_REALM_SEPARATOR);
    if (IsNullOrEmptyString(pszCursor) || IsNullOrEmptyString(pszCursor+1))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(pszCursor+1, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszDomain = pszDomain;

cleanup:

    return dwError;

error:

    *ppszDomain = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}
