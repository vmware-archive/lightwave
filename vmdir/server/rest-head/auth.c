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
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pRestOp->pszAuth))
    {
        dwError = VmDirMLSetupAnonymousAccessInfo(&pRestOp->pConn->AccessInfo);
        BAIL_ON_VMDIR_ERROR(dwError);

        pRestOp->pConn->bIsAnonymousBind = TRUE;
        goto cleanup;
    }

    dwError = VmDirRESTAuthViaToken(pRestOp);
    if (dwError && dwError == VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED)
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
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
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
    // we want this error to be mapped to invalid credentials
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
 */
DWORD
VmDirRESTAuthViaToken(
    PVDIR_REST_OPERATION    pRestOp
    )
{
    DWORD   dwError = 0;
    PSTR    pszBindDN = NULL;
    PVDIR_REST_AUTH_TOKEN   pAuthToken = NULL;
    PVDIR_OPERATION pBindOp = NULL;

    if (!pRestOp || IsNullOrEmptyString(pRestOp->pszAuth))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // unset previously set error
    dwError = VmDirRESTResultUnsetError(pRestOp->pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTAuthTokenInit(&pAuthToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRESTAuthTokenParse(pAuthToken, pRestOp->pszAuth);
    BAIL_ON_VMDIR_ERROR(dwError);

    // TODO (PR 2004701): Validate the proof of possession
    //   1) get hok and signature from HTTP request header
    //   2) get public key from HOK
    //   3) validate signature with public key
    //if (pAuthToken->tokenType == VDIR_REST_AUTH_TOKEN_HOTK)
    //{
    //    BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNWILLING_TO_PERFORM);
    //}

    dwError = VmDirUPNToDN(pAuthToken->pszBindUPN, &pszBindDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirExternalOperationCreate(
            NULL, -1, LDAP_REQ_BIND, pRestOp->pConn, &pBindOp);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringToBervalContent(pszBindDN, &pBindOp->reqDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    pBindOp->request.bindReq.method = LDAP_AUTH_NONE;

    dwError = VmDirInternalBindEntry(pBindOp);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SET_REST_RESULT(pRestOp, pBindOp, dwError, NULL);
    VmDirFreeRESTAuthToken(pAuthToken);
    VMDIR_SAFE_FREE_STRINGA(pszBindDN);
    VmDirFreeOperation(pBindOp);
    return dwError;

error:
    goto cleanup;
}
