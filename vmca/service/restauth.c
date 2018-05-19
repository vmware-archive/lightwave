/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ~@~\License~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ~@~\AS IS~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

static
DWORD
VMCAGetAccessTokenFromParameter(
    PSTR pszAccessTokenParameter,
    PVMCA_AUTHORIZATION_PARAM* ppAuthorization
    );

static
DWORD
VMCAFindRestAuthMethod(
    VMCA_AUTHORIZATION_TYPE authType,
    PDWORD pdwIdx
    );

DWORD
VMCARESTGetAccessToken(
    VMCA_HTTP_REQ_OBJ*  pVMCARequest,
    PVMCA_ACCESS_TOKEN* ppAccessToken
    )
{
    DWORD dwError = 0;
    DWORD dwIdx = 0;
    PVMCA_AUTHORIZATION_PARAM pAuthorization = NULL;
    PVMCA_ACCESS_TOKEN pAccessToken = NULL;

    if (!pVMCARequest || !ppAccessToken)
    {
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAGetAccessTokenFromParameter(pVMCARequest->pszAuthorization, &pAuthorization);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAFindRestAuthMethod(pAuthorization->tokenType, &dwIdx);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = gVMCAAccessTokenMethods[dwIdx].pfnVerify(pAuthorization, pVMCARequest, ppAccessToken);
    BAIL_ON_VMCA_ERROR(dwError);

    *ppAccessToken = pAccessToken;

cleanup:
    if (pAuthorization)
    {
        VMCAFreeAuthorizationParam(pAuthorization);
    }
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    if (ppAccessToken)
    {
        *ppAccessToken = NULL;
    }
    if (pAccessToken)
    {
        VMCAFreeAccessToken(pAccessToken);
    }
    dwError = (dwError == EACCES) ? EACCES : ERROR_ACCESS_DENIED;
    goto cleanup;
}

VOID
VMCAFreeAuthorizationParam(
      PVMCA_AUTHORIZATION_PARAM pAuthorization
      )
{
    if (pAuthorization)
    {
        VMCA_SAFE_FREE_STRINGA(pAuthorization->pszAuthorizationToken);
        VMCA_SAFE_FREE_MEMORY(pAuthorization);
    }
}

VOID
VMCAFreeAccessToken(
      PVMCA_ACCESS_TOKEN pAccessToken
      )
{
    DWORD dwError = 0;
    if (pAccessToken)
    {
        DWORD dwIdx = 0;
        dwError = VMCAFindRestAuthMethod(pAccessToken->tokenType, &dwIdx);
        if (!dwError)
        {
            gVMCAAccessTokenMethods[dwIdx].pfnFree(pAccessToken);
        }
    }
}

static
DWORD
VMCAGetAccessTokenFromParameter(
    PSTR pszAccessTokenParameter,
    PVMCA_AUTHORIZATION_PARAM* ppAuthorization
    )
{
    DWORD dwError = 0;
    PVMCA_AUTHORIZATION_PARAM pAuthorization = NULL;
    PSTR pszNextToken = NULL;
    PSTR pszTokenType = NULL;

    if (IsNullOrEmptyString(pszAccessTokenParameter) ||
        !ppAuthorization
       )
    {
        dwError = EACCES;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateMemory(
                            sizeof(VMCA_ACCESS_TOKEN),
                            (PVOID*)&pAuthorization
                            );
    BAIL_ON_VMCA_ERROR(dwError);

    pszTokenType = VMCAStringTokA(
                              pszAccessTokenParameter,
                              " ",
                              &pszNextToken
                              );

    if (pszTokenType)
    {
        if (!VMCAStringCompareA(pszTokenType, "Bearer", FALSE))
        {
            pAuthorization->tokenType = VMCA_AUTHORIZATION_TYPE_BEARER_TOKEN;
        }
        else if (!VMCAStringCompareA(pszTokenType, "Negotiate", FALSE))
        {
            pAuthorization->tokenType = VMCA_AUTHORIZATION_TOKEN_TYPE_KRB;
        }
        else if (!VMCAStringCompareA(pszTokenType, "hot-pk", FALSE) ||
                 !VMCAStringCompareA(pszTokenType, "hotk-pk", FALSE))
        {
            pAuthorization->tokenType = VMCA_AUTHORIZATION_TYPE_HOTK_TOKEN;
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMCA_ERROR(dwError);
        }
    }
    else
    {
        dwError = ERROR_INVALID_PARAMETER; 
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VMCAAllocateStringA(
                           pszNextToken,
                           &pAuthorization->pszAuthorizationToken
                           );
    BAIL_ON_VMCA_ERROR(dwError);

    *ppAuthorization = pAuthorization;
cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    if (ppAuthorization)
    {
        *ppAuthorization = NULL;
    }
    if (pAuthorization)
    {
        VMCA_SAFE_FREE_STRINGA(pAuthorization->pszAuthorizationToken);
        VMCA_SAFE_FREE_MEMORY(pAuthorization);
    }
    goto cleanup;
}

static
DWORD
VMCAFindRestAuthMethod(
    VMCA_AUTHORIZATION_TYPE authType,
    PDWORD pdwIdx
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    BOOL bFound = FALSE;

    if (!pdwIdx)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    for (;dwIndex < VMCA_AUTHORIZATION_TOKEN_TYPE_MAX; ++dwIndex)
    {
        if (gVMCAAccessTokenMethods[dwIndex].type == authType)
        {
            bFound = TRUE;
            break;
        }
    }

    if (bFound)
    {
        *pdwIdx = dwIndex;
    }
    else
    {
        dwError = ERROR_INVALID_FUNCTION;
        BAIL_ON_VMCA_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMCA_LOG_ERROR("%s failed with error (%d)", __FUNCTION__, dwError);
    if (pdwIdx)
    {
        *pdwIdx = 0;
    }
    goto cleanup;
}
