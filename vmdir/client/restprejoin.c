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

static
DWORD
_VmDirJsonResultMachineAccountElementsCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    pMachineInfo = (PVMDIR_MACHINE_INFO_A)pUserData;

    if (!VmDirStringCompareA("computer_dn", pszKey, TRUE))
    {
        dwError = VmDirAllocateStringA(
                      pValue->value.pszValue,
                      &pMachineInfo->pszComputerDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!VmDirStringCompareA("sitename", pszKey, TRUE))
    {
        dwError = VmDirAllocateStringA(
                      pValue->value.pszValue,
                      &pMachineInfo->pszSiteName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (!VmDirStringCompareA("machine_guid", pszKey, TRUE))
    {
        dwError = VmDirAllocateStringA(
                      pValue->value.pszValue,
                      &pMachineInfo->pszMachineGUID);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirJsonResultMachineAccountCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    /* top level key must be "machine_account" */
    if (VmStringCompareA("machine_account", pszKey, TRUE) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Error parsing join result json. Top level key: '%s'. Expected 'machine_account'",
            pszKey);
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    /* iterate elements of machine_account object */
    dwError = VmJsonResultIterateObjectAt(
                  pValue->value.pObject,
                  pUserData,
                  _VmDirJsonResultMachineAccountElementsCB);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirJsonResultTopLevelJoinObjectCB(
    PVOID pUserData,
    PCSTR pszKey,
    PVM_JSON_RESULT_VALUE pValue
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    if (!pUserData || IsNullOrEmptyString(pszKey) || !pValue)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    pMachineInfo = (PVMDIR_MACHINE_INFO_A)pUserData;

    /* top level key must be "join" */
    if (VmStringCompareA("join", pszKey, TRUE) != 0)
    {
        VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Error parsing join result json. Top level key: '%s'. Expected 'join'",
            pszKey);
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    /* iterate machine_account object */
    dwError = VmJsonResultIterateObjectAt(
                  pValue->value.pObject,
                  pMachineInfo,
                  _VmDirJsonResultMachineAccountCB);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "Error: [%s : %d]", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_VmDirGetHttpClientResultMachineInfo(
    PCSTR pszResult,
    PVMDIR_MACHINE_INFO_A *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVM_JSON_RESULT pJsonResult = NULL;
    PVM_JSON_POSITION pPosition = NULL;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    dwError = VmJsonResultInit(&pJsonResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmJsonResultLoadString(pszResult, pJsonResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmJsonResultGetRootPosition(pJsonResult, &pPosition);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                  sizeof(VMDIR_MACHINE_INFO_A),
                  (PVOID *)&pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    /*
     * this is the structure of json we look to parse.
     * {"join":
     *     {"machine_account":
     *          {"computer_dn":"cn=lightwave-server,ou=blah,ou=Computers,dc=post,dc=test",
     *           "sitename":"Default-first-site",
     *           "machine_guid":"7c575148-1158-4f1b-9609-59521297d9d1"
     *          }
     *      }
     *  }
    */
    dwError = VmJsonResultIterateObjectAt(
                  pPosition,
                  pMachineInfo,
                 _VmDirJsonResultTopLevelJoinObjectCB);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:
    VmJsonResultFreeHandle(pJsonResult);
    return dwError;

error:
    VmDirFreeMachineInfoA(pMachineInfo);
    goto cleanup;
}

/*
 * TODO: Return pKrbInfo from this function as well
 * not immediately required for prejoins
*/
static
DWORD
_VmDirRestClientPrejoinAtomic(
    PCSTR pszServerName,
    PCSTR pszToken,
    PCSTR pszCAPath,
    PCSTR pszMachineName,
    PCSTR pszOrgUnit,
    PVMDIR_MACHINE_INFO_A *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PSTR pszParamString = NULL;
    PSTR pszUrl = NULL;
    PSTR pszOrgUnitParam = NULL;
    PVM_HTTP_CLIENT pHttpClient = NULL;
    PCSTR pszResult = NULL;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    dwError = VmHttpClientInit(&pHttpClient, pszCAPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!IsNullOrEmptyString(pszOrgUnit))
    {
        dwError = VmHttpClientSetQueryParam(
                      pHttpClient,
                      VMDIR_REST_JOINATOMIC_ORG_UNIT,
                      pszOrgUnit);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_REST_JOINATOMIC_MACHINE_ACCOUNT_NAME,
                  pszMachineName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetQueryParam(
                  pHttpClient,
                  VMDIR_REST_JOINATOMIC_PREJOINED,
                  "true");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmFormatUrl(
                  "https",
                  pszServerName,
                  VMDIR_REST_API_HTTPS_PORT,
                  VMDIR_REST_API_BASE"/"VMDIR_REST_API_JOINATOMIC_CMD,
                  NULL, /* query params will be set in VmHttpClientPerform */
                  &pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientSetToken(pHttpClient,
                                   VMHTTP_TOKEN_TYPE_BEARER,
                                   pszToken);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientPerform(pHttpClient, VMHTTP_METHOD_POST, pszUrl);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmHttpClientGetResult(pHttpClient, &pszResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetHttpClientResultMachineInfo(pszResult, &pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:
    VmHttpClientFreeHandle(pHttpClient);
    VMDIR_SAFE_FREE_MEMORY(pszUrl);
    VMDIR_SAFE_FREE_MEMORY(pszOrgUnitParam);
    VMDIR_SAFE_FREE_MEMORY(pszParamString);
    return dwError;

error:
    VmDirFreeMachineInfoA(pMachineInfo);
    goto cleanup;
}

/*
 * Do prejoin commands via rest path
*/
DWORD
VmDirRestClientPreJoinAtomic(
    PCSTR                   pszServerName,
    PCSTR                   pszToken,
    PCSTR                   pszCAPath,
    PCSTR                   pszMachineName,
    PCSTR                   pszPassword,
    PCSTR                   pszOrgUnit,
    PVMDIR_MACHINE_INFO_A   *ppMachineInfo
    )
{
    DWORD dwError = 0;
    PVMDIR_MACHINE_INFO_A pMachineInfo = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszToken) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszCAPath) ||
        IsNullOrEmptyString(pszMachineName) ||
        !ppMachineInfo
       )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = _VmDirRestClientPrejoinAtomic(
                  pszServerName,
                  pszToken,
                  pszCAPath,
                  pszMachineName,
                  pszOrgUnit,
                  &pMachineInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConfigSetDCAccountInfo(
                  pszMachineName,
                  pMachineInfo->pszComputerDN,
                  pszPassword,
                  VmDirStringLenA(pszPassword),
                  pMachineInfo->pszMachineGUID);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppMachineInfo = pMachineInfo;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s error (%u)", __FUNCTION__, dwError);

    if (ppMachineInfo)
    {
        *ppMachineInfo = NULL;
    }
    VmDirFreeMachineInfoA(pMachineInfo);
    goto cleanup;
}
