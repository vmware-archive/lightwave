/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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
VmDirRESTAuth(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;

    if (!pRestOp)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (IsNullOrEmptyString(pRestOp->pszAuth))
    {
        dwError = VmDirMLSetupAnonymousAccessInfo(&pRestOp->pConn->AccessInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        pRestOp->pConn->bIsAnonymousBind = TRUE;
        goto cleanup;
    }

    dwError = VmDirRESTAuthViaToken(pRestOp);
    if (dwError && pRestOp->authMthd == VDIR_REST_AUTH_METHOD_UNDEF)
    {
        dwError = VmDirRESTAuthViaBasic(pRestOp);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, error (%d)",
            __FUNCTION__,
            dwError);

    goto cleanup;
}

/*
 * Note: In lwraft, basic auth is for lwraft accounts only
 */
DWORD
VmDirRESTAuthViaBasic(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;
    int     sts = 0;
    int     len = 0;
    PSTR    pszBasic = NULL;
    PSTR    pszData = NULL;
    PSTR    pszDecode = NULL;
    PSTR    pszBindDN = NULL;
    PSTR    pszPasswd = NULL;
    PVDIR_OPERATION pBindOp = NULL;

    if (!pRestOp || IsNullOrEmptyString(pRestOp->pszAuth))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // unset previously set error
    dwError = VmDirRESTResultUnsetError(pRestOp->pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszBasic = strstr(pRestOp->pszAuth, "Basic ");
    if (IsNullOrEmptyString(pszBasic))
    {
        dwError = VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    pRestOp->authMthd = VDIR_REST_AUTH_METHOD_BASIC;

    pszData = pszBasic + strlen("Basic ");

    dwError = VmDirAllocateMemory(VmDirStringLenA(pszData) + 1, (PVOID*)&pszDecode);
    BAIL_ON_VMDIR_ERROR(dwError);

    sts = sasl_decode64(pszData, strlen(pszData), pszDecode, strlen(pszData), &len);
    if (sts != SASL_OK)
    {
        dwError = VMDIR_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pszPasswd = strchr(pszDecode, ':');
    if (IsNullOrEmptyString(pszPasswd))
    {
        dwError = VMDIR_ERROR_AUTH_BAD_DATA;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    *pszPasswd = '\0';
    pszPasswd++;

    dwError = VmDirUPNToDN(pszDecode, &pszBindDN);
    // we want this to map to invalid credentials error
    if (dwError == VMDIR_ERROR_ENTRY_NOT_FOUND)
    {
        dwError = VMDIR_ERROR_AUTH_BAD_DATA;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_BIND, pRestOp->pConn, &pBindOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszBindDN, &pBindOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszPasswd, &pBindOp->request.bindReq.cred);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBindOp->request.bindReq.method = LDAP_AUTH_SIMPLE;

    dwError = VmDirInternalBindEntry(pBindOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pBindOp, dwError, NULL);
    VMDIR_SECURE_FREE_STRINGA(pszDecode);
    VMDIR_SAFE_FREE_STRINGA(pszBindDN);
    VmDirFreeOperation(pBindOp);
    return dwError;

error:
    goto cleanup;
}

/*
 * Do Authentication based on received Token
 *
 * Note: In lwraft, token auth is for lightwave accounts only
 */
DWORD
VmDirRESTAuthViaToken(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;
    DWORD   i = 0;
    ULONG   ulBufLen = 0;
    PVDIR_REST_AUTH_TOKEN   pAuthToken = NULL;
    PACCESS_TOKEN           pAccessToken = NULL;
    PTOKEN_USER             pUser  = NULL;
    PTOKEN_GROUPS           pGroups = NULL;
    PSTR                    pszUserSid = NULL;
    PSID                    pBuiltInAdminsGroupSid = NULL;
    PLW_MAP_SECURITY_CONTEXT    pMapSecurityContext = NULL;
    VDIR_BERVALUE   berval = VDIR_BERVALUE_INIT;

    if (!pRestOp || IsNullOrEmptyString(pRestOp->pszAuth))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    // unset previously set error
    dwError = VmDirRESTResultUnsetError(pRestOp->pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTAuthTokenInit(&pAuthToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTAuthTokenParse(pAuthToken, pRestOp->pszAuth);
    BAIL_ON_VMDIR_ERROR(dwError);

    pRestOp->authMthd = VDIR_REST_AUTH_METHOD_TOKEN;

    dwError = VmDirRESTAuthTokenValidate(pAuthToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO (PR 2004701): Validate the proof of possession
    //   1) get hok and signature from HTTP request header
    //   2) get public key from HOK
    //   3) validate signature with public key
    //if (pAuthToken->tokenType == VDIR_REST_AUTH_TOKEN_HOTK)
    //{
    //    BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED);
    //}

    // retrieve security information of the UPN
    dwError = LwMapSecurityCreateContext(&pMapSecurityContext);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = LwMapSecurityCreateAccessTokenFromCStringUsername(
            pMapSecurityContext, &pAccessToken, pAuthToken->pszBindUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get user sid
    dwError = VmDirQueryAccessTokenInformation(
            pAccessToken, TokenUser, NULL, 0, &ulBufLen);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

    dwError = VmDirAllocateMemory(ulBufLen, (PVOID*)&pUser);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueryAccessTokenInformation(
            pAccessToken, TokenUser, pUser, ulBufLen, &ulBufLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateCStringFromSid(&pszUserSid, pUser->User.Sid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // grant full access to members of admin group of joined lightwave domain
    dwError = VmDirQueryAccessTokenInformation(
            pAccessToken, TokenGroups, NULL, 0, &ulBufLen);
    BAIL_ON_VMDIR_ERROR(dwError != ERROR_INSUFFICIENT_BUFFER);

    dwError = VmDirAllocateMemory(ulBufLen, (PVOID*)&pGroups);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirQueryAccessTokenInformation(
            pAccessToken, TokenGroups, pGroups, ulBufLen, &ulBufLen);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTCacheGetBuiltInAdminsGroupSid(
            gpVdirRestCache, &pBuiltInAdminsGroupSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (i = 0; i < pGroups->GroupCount; i++)
    {
        if (RtlEqualSid(pGroups->Groups[i].Sid, pBuiltInAdminsGroupSid))
        {
            VMDIR_LOG_VERBOSE(
                    VMDIR_LOG_MASK_ALL,
                    "Admin group access rights given to user: %s",
                    VDIR_SAFE_STRING(pAuthToken->pszBindUPN));

            RtlReleaseAccessToken(&pAccessToken);
            pAccessToken = NULL;

            dwError = VmDirSrvCreateAccessTokenForAdmin(&pAccessToken);
            BAIL_ON_VMDIR_ERROR(dwError);
            break;
        }
    }

    // populate connection access info
    pRestOp->pConn->AccessInfo.pszBindedObjectSid = pszUserSid;
    pszUserSid = NULL;

    pRestOp->pConn->AccessInfo.pAccessToken = pAccessToken;
    pAccessToken = NULL;

    // Note: copied from anonymous bind
    // Set these flags so that the worker routines don't try to look up our
    // info (since we don't have a real user to search against). Since we're
    // anonymous we know we're not in any of these groups.
    pRestOp->pConn->AccessInfo.accessRoleBitmap =
            VDIR_ACCESS_DCGROUP_MEMBER_VALID_INFO |
            VDIR_ACCESS_DCCLIENT_GROUP_MEMBER_VALID_INFO |
            VDIR_ACCESS_ADMIN_MEMBER_VALID_INFO;

    // build imaginary binded DN in format of "cn=<UPN>,<DOMAIN_DN>"
    dwError = VmDirAllocateStringPrintf(
            &berval.lberbv.bv_val,
            "cn=%s,%s",
            pAuthToken->pszBindUPN,
            gVmdirServerGlobals.systemDomainDN.lberbv.bv_val);
    BAIL_ON_VMDIR_ERROR(dwError);
    berval.bOwnBvVal = TRUE;

    dwError = VmDirNormalizeDNWrapper(&berval);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            berval.lberbv.bv_val, &pRestOp->pConn->AccessInfo.pszBindedDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
            berval.bvnorm_val, &pRestOp->pConn->AccessInfo.pszNormBindedDn);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, NULL, dwError, NULL);
    LwMapSecurityFreeContext(&pMapSecurityContext);
    RtlReleaseAccessToken(&pAccessToken);
    VmDirFreeRESTAuthToken(pAuthToken);
    VmDirFreeBervalContent(&berval);
    VMDIR_SAFE_FREE_MEMORY(pBuiltInAdminsGroupSid);
    VMDIR_SAFE_FREE_MEMORY(pszUserSid);
    VMDIR_SAFE_FREE_MEMORY(pGroups);
    VMDIR_SAFE_FREE_MEMORY(pUser);
    return dwError;

error:
    goto cleanup;
}
