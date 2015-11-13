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
CdcAllocateDCInfoAfromW(
    PCDC_DC_INFO_W pDomainControllerInfoW,
    PCDC_DC_INFO_A *ppDomainControllerInfoA
    );

static
DWORD
CdcValidateState(
    DWORD dwHAState,
    PCDC_DC_STATE pCdcState
    );

/*
 * @brief Enables client affinity
 *
 * @param[in]  pServer Host server struct
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnableClientAffinity(
        PVMAFD_SERVER pServer
        )
{
    DWORD dwError = 0;

    if (!pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = CdcLocalEnableClientAffinity();
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * @brief Disable client affinity feature
 *
 * @param[in]  pServer Host server struct
 *
 * @return Returns 0 for success
 */
DWORD
CdcDisableClientAffinity(
        PVMAFD_SERVER pServer
        )
{
    DWORD dwError = 0;

    if (!pServer)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = CdcLocalDisableClientAffinity();
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * @brief Returns the affinitized DC for the node
 *
 * @param[in]           pServer Host server struct
 * @param[in,optional]  pszDomainName Domain Name
 * @param[in,optional]  pDomainGuid
 * @param[in,optional]  pszSiteName Site Name
 * @param[in,optional]  dwFlags Flags to filter the result
 * @param[out]          ppDomainControllerInfo Pointer to a struct to hold DC info
 *
 * @return Returns 0 for success
 */
DWORD
CdcGetDCNameA(
        PVMAFD_SERVER  pServer,
        PCSTR          pszDomainName,
        GUID_A         pDomainGuid,
        PCSTR          pszSiteName,
        DWORD          dwFlags,
        PCDC_DC_INFO_A *ppDomainControllerInfo
        )
{
    DWORD dwError = 0;
    PWSTR pwszDomainName = NULL;
    GUID_W pwDomainGuid = NULL;
    PWSTR pwszSiteName = NULL;
    PCDC_DC_INFO_A pDomainControllerInfo = NULL;
    PCDC_DC_INFO_W pwDomainControllerInfo = NULL;

    if (!pServer || !ppDomainControllerInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if(!IsNullOrEmptyString(pszDomainName))
    {
        dwError = VmAfdAllocateStringWFromA(
                                 pszDomainName,
                                 &pwszDomainName
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszSiteName))
    {
        dwError = VmAfdAllocateStringWFromA(
                                pszSiteName,
                                &pwszSiteName
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pDomainGuid))
    {
        dwError = VmAfdAllocateStringWFromA(
                                pDomainGuid,
                                &pwDomainGuid
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcGetDCNameW(
                      pServer,
                      pwszDomainName,
                      pwDomainGuid,
                      pwszSiteName,
                      dwFlags,
                      &pwDomainControllerInfo
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcAllocateDCInfoAfromW(
                            pwDomainControllerInfo,
                            &pDomainControllerInfo
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppDomainControllerInfo = pDomainControllerInfo;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwDomainGuid);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    if (pwDomainControllerInfo)
    {
        CdcFreeDomainControllerInfoW(pwDomainControllerInfo);
    }

    return dwError;
error:
    if (ppDomainControllerInfo)
    {
        *ppDomainControllerInfo = NULL;
    }
    if (pDomainControllerInfo)
    {
       CdcFreeDomainControllerInfoA(pDomainControllerInfo);
    }

    goto cleanup;
}

/*
 * @brief Returns the affinitized DC for the node
 *
 * @param[in]           pServer Host server struct
 * @param[in,optional]  pwszDomainName Domain Name
 * @param[in,optional]  pDomainGuid
 * @param[in,optional]  pwszSiteName Site Name
 * @param[in,optional]  dwFlags Flags to filter the result
 * @param[out]          ppDomainControllerInfo Pointer to a struct to hold DC info
 *
 * @return Returns 0 for success
 */

DWORD
CdcGetDCNameW(
        PVMAFD_SERVER  pServer,
        PCWSTR         pszDomainName,
        GUID_W         pDomainGuid,
        PCWSTR         pszSiteName,
        DWORD          dwFlags,
        PCDC_DC_INFO_W *ppDomainControllerInfo
        )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pDomainControllerInfo = NULL;

    if (!pServer || !ppDomainControllerInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = CdcLocalGetDCNameW(
                                pszDomainName,
                                pDomainGuid,
                                pszSiteName,
                                dwFlags,
                                &pDomainControllerInfo
                                );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_CALL_NOT_IMPLEMENTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppDomainControllerInfo = pDomainControllerInfo;

cleanup:

    return dwError;
error:

    if (ppDomainControllerInfo)
    {
        *ppDomainControllerInfo = NULL;
    }

    if (pDomainControllerInfo)
    {
        CdcFreeDomainControllerInfoW(pDomainControllerInfo);
    }

    goto cleanup;
}

/*
 * @brief Lists entries in client side cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          ppszDCEntries String array to hold DC entries
 * @param[out]          pdwCount Returns the count of entries
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnumDCEntriesA(
        PVMAFD_SERVER pServer,
        PSTR          **ppszDCEntries,
        PDWORD        pdwCount
        )
{
    DWORD dwError = 0;
    PWSTR *pwszDCEntries = NULL;
    PSTR *pszDCEntries = NULL;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;

    if (!ppszDCEntries ||
        !pdwCount ||
        !pServer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = CdcEnumDCEntriesW(
                      pServer,
                      &pwszDCEntries,
                      &dwCount
                      );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (dwCount)
    {
      dwError = VmAfdAllocateMemory (
                  sizeof (PSTR) * dwCount,
                  (PVOID *)&pszDCEntries
                  );
      BAIL_ON_VMAFD_ERROR (dwError);

      for (; dwIndex <dwCount; dwIndex++)
      {
          dwError = VmAfdAllocateStringAFromW (
                          pwszDCEntries[dwIndex],
                          &(pszDCEntries[dwIndex])
                          );
          BAIL_ON_VMAFD_ERROR (dwError);
      }
    }

    *ppszDCEntries = pszDCEntries;
    *pdwCount = dwCount;
cleanup:

    CdcFreeStringArrayW(pwszDCEntries, dwCount);

    return dwError;
error:

    if (ppszDCEntries)
    {
        *ppszDCEntries = NULL;
    }
    CdcFreeStringArrayA(pszDCEntries, dwCount);

    goto cleanup;
}


/*
 * @brief Lists entries in client side cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          ppszDCEntries String array to hold DC entries
 * @param[out]          pdwCount Returns the count of entries
 *
 * @return Returns 0 for success
 */
DWORD
CdcEnumDCEntriesW(
        PVMAFD_SERVER pServer,
        PWSTR         **ppszDCEntries,
        PDWORD        pdwCount
        )
{
    DWORD dwError = 0;
    PWSTR *pszDCEntries = NULL;
    DWORD dwCount = 0;

    if (!ppszDCEntries ||
        !pdwCount ||
        !pServer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pServer->hBinding)
    {
        dwError = CdcLocalEnumDCEntries(
                                    &pszDCEntries,
                                    &dwCount
                                    );
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppszDCEntries = pszDCEntries;
    *pdwCount = dwCount;

cleanup:

    return dwError;
error:

    if (ppszDCEntries)
    {
        *ppszDCEntries = NULL;
    }
    if (pdwCount)
    {
        *pdwCount = 0;
    }
    if (pszDCEntries)
    {
        CdcFreeStringArrayW(pszDCEntries, dwCount);
    }

    goto cleanup;
}

/*
 * @brief Gets the current state of the client domain controller cache
 *
 * @param[in]           pServer Host server struct
 * @param[out]          pcdcState Returns the state of cdc
 *
 * @return Returns 0 for success
 */
DWORD
CdcGetCurrentState(
        PVMAFD_SERVER pServer,
        PCDC_DC_STATE pcdcState
        )
{
    DWORD dwError = 0;
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;
    DWORD dwState = 0;

    if (!pServer || !pcdcState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (!pServer->hBinding)
    {
        dwError = CdcLocalGetCurrentState(&dwState);
    }
    else
    {
        dwError = ERROR_CALL_NOT_IMPLEMENTED;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = CdcValidateState(dwState, &cdcState);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pcdcState = cdcState;

cleanup:

    return dwError;
error:

    if (pcdcState)
    {
        *pcdcState = CDC_DC_STATE_UNDEFINED;
    }
    goto cleanup;
}

/*
 * @brief Frees an array of strings
 *
 * @param[in] ppszStringArray Array of strings
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeStringArrayA(
    PSTR *pszStringArray,
    DWORD dwCount
    )
{
    if (pszStringArray && dwCount)
    {
        VmAfdFreeStringArrayCountA (
              pszStringArray,
              dwCount
            );
    }
}


/*
 * @brief Frees an array of strings
 *
 * @param[in] pwszStringArray Array of strings
 * @param[in] dwCount Count of entries
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeStringArrayW(
    PWSTR *pwszStringArray,
    DWORD dwCount
    )
{
    if (pwszStringArray && dwCount)
    {
        VmAfdFreeStringArrayW (
            pwszStringArray,
            dwCount
            );
    }
}

/*
 * @brief Frees Domain Controller Info structure
 *
 * @param[in] pDomainControllerInfoA
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeDomainControllerInfoA(
    PCDC_DC_INFO_A pDomainControllerInfoA
    )
{
    if (pDomainControllerInfoA)
    {
        VmAfdFreeDomainControllerInfoA(pDomainControllerInfoA);
    }
}

/*
 * @brief Frees Domain Controller Info structure
 *
 * @param[in] pDomainControllerInfoW
 *
 * @return Returns 0 for success
 */
VOID
CdcFreeDomainControllerInfoW(
    PCDC_DC_INFO_W pDomainControllerInfoW
    )
{
    if (pDomainControllerInfoW)
    {
        VmAfdFreeDomainControllerInfoW(pDomainControllerInfoW);
    }
}

static
DWORD
CdcAllocateDCInfoAfromW(
    PCDC_DC_INFO_W pDomainControllerInfoW,
    PCDC_DC_INFO_A *ppDomainControllerInfoA
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_A pDomainControllerInfoA = NULL;

    if (!pDomainControllerInfoW ||
        !ppDomainControllerInfoA)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateMemory(
                            sizeof(CDC_DC_INFO_A),
                            (PVOID *)&pDomainControllerInfoA
                            );
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!IsNullOrEmptyString(pDomainControllerInfoW->pszDCName))
    {
        dwError = VmAfdAllocateStringAFromW(
                                  pDomainControllerInfoW->pszDCName,
                                  &pDomainControllerInfoA->pszDCName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pDomainControllerInfoW->pszDCAddress))
    {
        dwError = VmAfdAllocateStringAFromW(
                                  pDomainControllerInfoW->pszDCAddress,
                                  &pDomainControllerInfoA->pszDCAddress
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pDomainControllerInfoW->pszDomainName))
    {
        dwError = VmAfdAllocateStringAFromW(
                                  pDomainControllerInfoW->pszDomainName,
                                  &pDomainControllerInfoA->pszDomainName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pDomainControllerInfoW->pszDcSiteName))
    {
        dwError = VmAfdAllocateStringAFromW(
                                  pDomainControllerInfoW->pszDcSiteName,
                                  &pDomainControllerInfoA->pszDcSiteName
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    pDomainControllerInfoA->DcAddressType =
                            pDomainControllerInfoW->DcAddressType;

    *ppDomainControllerInfoA = pDomainControllerInfoA;

cleanup:

    return dwError;

error:

    if (ppDomainControllerInfoA)
    {
        *ppDomainControllerInfoA = NULL;
    }

    if (pDomainControllerInfoA)
    {
        CdcFreeDomainControllerInfoA(pDomainControllerInfoA);
    }

    goto cleanup;
}

static
DWORD
CdcValidateState(
    DWORD dwHAState,
    PCDC_DC_STATE pCdcState
    )
{
    DWORD dwError = 0;
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;

    if (!pCdcState)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    switch(dwHAState)
    {
        case CDC_DC_STATE_NO_DC_LIST:
          cdcState = CDC_DC_STATE_NO_DC_LIST;
          break;

        case CDC_DC_STATE_SITE_AFFINITIZED:
          cdcState = CDC_DC_STATE_SITE_AFFINITIZED;
          break;

        case CDC_DC_STATE_OFF_SITE:
          cdcState = CDC_DC_STATE_OFF_SITE;
          break;

        case CDC_DC_STATE_NO_DCS_ALIVE:
          cdcState = CDC_DC_STATE_NO_DCS_ALIVE;
          break;

        case CDC_DC_STATE_DISABLED:
          cdcState = CDC_DC_STATE_DISABLED;
          break;

        default:
          dwError = ERROR_INVALID_PARAMETER;
          break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    *pCdcState = cdcState;

cleanup:

    return dwError;
error:

    if (pCdcState)
    {
        *pCdcState = CDC_DC_STATE_UNDEFINED;
    }
    goto cleanup;
}
