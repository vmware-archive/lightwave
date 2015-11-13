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
 * Module Name:
 *
 *        sspi.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        SSPI
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#include "stdafx.h"

DWORD
IDMCreateAuthContext(
    LPWSTR pszPackageName,
    PIDM_AUTH_CONTEXT * ppAuthContext
    )
{
    PIDM_AUTH_CONTEXT pAuthContext = NULL;
    DWORD dwError = 0;
    TimeStamp Lifetime = {0};
    PSecPkgInfo pPackageInfo = NULL;
    SECURITY_STATUS secError = SEC_E_OK;

    if (!pszPackageName || !ppAuthContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    secError = QuerySecurityPackageInfo (pszPackageName, &pPackageInfo);
    BAIL_ON_SECURITY_ERROR(secError, dwError);

    dwError = IDMAllocateMemory(
                    sizeof(IDM_AUTH_CONTEXT),
                    (PVOID *)&pAuthContext);
    BAIL_ON_ERROR(dwError);

    pAuthContext->fNewConversation = TRUE;
    pAuthContext->dwState = AUTH_START_STATE;

    pAuthContext->dwMaxTokenSize = pPackageInfo->cbMaxToken;

    secError = AcquireCredentialsHandle (
                NULL,
                pszPackageName,
                SECPKG_CRED_INBOUND,
                NULL,
                NULL,
                NULL,
                NULL,
                &pAuthContext->hCredsHandle,
                &Lifetime);
    BAIL_ON_SECURITY_ERROR(secError, dwError);

    pAuthContext->phCredsHandle = &pAuthContext->hCredsHandle;

    *ppAuthContext = pAuthContext;

cleanup:

    if (pPackageInfo)
    {
        FreeContextBuffer(pPackageInfo);
    }

    return dwError;

error:

    if (pAuthContext)
    {
        IDMFreeAuthContext(pAuthContext);
    }
    goto cleanup;
}

DWORD
IDMAuthenticate2(
    PIDM_AUTH_CONTEXT pAuthContext,
    LPBYTE            pInputBuffer,
    DWORD             dwInputBufferSize,
    LPBYTE *          ppOutputBuffer,
    DWORD *           pdwOutputBufferSize,
    BOOL *            pfDone
    )
{
    DWORD dwError = 0;
    TimeStamp     Lifetime = {0};

    SecBufferDesc OutBuffDesc = {0};
    SecBuffer     OutSecBuff = {0};

    SecBufferDesc InBuffDesc = {0};
    SecBuffer     InSecBuff = {0};
    ULONG         Attribs = 0;
    DWORD  dwOutputBufferSize = 0;
    LPBYTE pOutputBuffer = NULL;
    SECURITY_STATUS secError = SEC_E_OK;

    if (!pAuthContext ||
        !pInputBuffer ||
        !ppOutputBuffer ||
        !pdwOutputBufferSize ||
        !pfDone)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pAuthContext->dwState == AUTH_START_COMPLETE)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    //----------------------------------------------------------------
    //  Prepare output buffers.

    dwError = IDMAllocateMemory(
                    pAuthContext->dwMaxTokenSize,
                    &pOutputBuffer);
    BAIL_ON_ERROR(dwError);

    dwOutputBufferSize = pAuthContext->dwMaxTokenSize;

    OutBuffDesc.ulVersion = 0;
    OutBuffDesc.cBuffers = 1;
    OutBuffDesc.pBuffers = &OutSecBuff;

    OutSecBuff.cbBuffer = dwOutputBufferSize;
    OutSecBuff.BufferType = SECBUFFER_TOKEN;
    OutSecBuff.pvBuffer = pOutputBuffer;

    //----------------------------------------------------------------
    //  Prepare input buffers.

    InBuffDesc.ulVersion = 0;
    InBuffDesc.cBuffers = 1;
    InBuffDesc.pBuffers = &InSecBuff;

    InSecBuff.cbBuffer = dwInputBufferSize;
    InSecBuff.BufferType = SECBUFFER_TOKEN;
    InSecBuff.pvBuffer = pInputBuffer;

    secError = AcceptSecurityContext (
                pAuthContext->phCredsHandle,
                pAuthContext->fNewConversation ? NULL : &pAuthContext->hCtxtHandle,
                &InBuffDesc,
                Attribs,
                SECURITY_NATIVE_DREP,
                &pAuthContext->hCtxtHandle,
                &OutBuffDesc,
                &Attribs,
                &Lifetime
                );
    BAIL_ON_SECURITY_ERROR(secError, dwError);

    pAuthContext->phCtxtHandle = &pAuthContext->hCtxtHandle;

    if (pAuthContext->fNewConversation)
    {
        pAuthContext->fNewConversation = FALSE;
    }

    //----------------------------------------------------------------
    //  Complete token if applicable.

    if ((SEC_I_COMPLETE_NEEDED == dwError)
        || (SEC_I_COMPLETE_AND_CONTINUE == dwError))
    {
        dwError = CompleteAuthToken (&pAuthContext->hCtxtHandle, &OutBuffDesc);
        BAIL_ON_ERROR(dwError);
    }

    dwOutputBufferSize = OutSecBuff.cbBuffer;

    *pfDone = !((SEC_I_CONTINUE_NEEDED == secError)
        || (SEC_I_COMPLETE_AND_CONTINUE == secError));

    pAuthContext->dwState = *pfDone ? AUTH_START_COMPLETE : AUTH_START_CONTINUE;

    *pdwOutputBufferSize = dwOutputBufferSize;
    *ppOutputBuffer = pOutputBuffer;

cleanup:

    return dwError;

error:
    IDM_SAFE_FREE_MEMORY(pOutputBuffer);

    goto cleanup;
}


DWORD
IDMGetUserInformationFromAuthContext(
    PIDM_AUTH_CONTEXT pAuthContext,
    PIDM_USER_INFO *  ppIdmUserInformation
    )

{
    DWORD dwError = 0;
    HANDLE hClientToken = NULL;
    CtxtHandle hSecCtxt = {0};
    PIDM_USER_INFO pUserInfo = NULL;
    SECURITY_STATUS secError = SEC_E_OK;

    if (!pAuthContext || !ppIdmUserInformation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (pAuthContext->dwState != AUTH_START_COMPLETE)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMInitializeSidCache();
    BAIL_ON_ERROR(dwError);

    hSecCtxt = pAuthContext->hCtxtHandle;

    secError =  QuerySecurityContextToken(&hSecCtxt, &hClientToken);
    BAIL_ON_SECURITY_ERROR(secError, dwError);

    dwError = IDMGetUserInfo(hClientToken, &pUserInfo);
    BAIL_ON_ERROR(dwError);

    *ppIdmUserInformation = pUserInfo;

error:
    if (hClientToken)
    {
        CloseHandle(hClientToken);
    }

    return dwError;
}

VOID
IDMFreeAuthContext(
    PIDM_AUTH_CONTEXT pAuthContext
    )
{
    if (pAuthContext)
    {
        if (pAuthContext->phCredsHandle)
        {
            FreeCredentialsHandle(&pAuthContext->hCredsHandle);
            pAuthContext->phCredsHandle = NULL;
        }
        if (pAuthContext->phCtxtHandle)
        {
            DeleteSecurityContext(&pAuthContext->hCtxtHandle);
            pAuthContext->phCtxtHandle = NULL;
        }

        IDM_SAFE_FREE_MEMORY(pAuthContext);

        pAuthContext = NULL;
    }
}

