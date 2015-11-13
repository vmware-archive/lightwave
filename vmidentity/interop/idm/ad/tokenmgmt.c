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
 *        tokenmgmt.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        Token Management
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#include "stdafx.h"
#include <sddl.h>


/*
 * Convert SID structure to a SID string.
 * Must copy from memory allocated by ConvertSidToStringSid()
 * as that memory must be released using LocalFree().
 */
static DWORD
_IDMSidToString(
    PSID pSid,
    PWSTR *ppwszSidString)
{
    DWORD dwError = 0;
    PWSTR pwszSidString = NULL;
    PWSTR pwszSidStringLocal = NULL;

    if (ConvertSidToStringSid(
            pSid,
            &pwszSidStringLocal))
    {
        dwError = IDMAllocateString(
                      pwszSidStringLocal,
                      &pwszSidString);
        BAIL_ON_ERROR(dwError);
    }
    *ppwszSidString = pwszSidString;

error:
    if (pwszSidStringLocal)
    {
        LocalFree(pwszSidStringLocal);
    }
    if (dwError)
    {
        IDM_SAFE_FREE_MEMORY(pwszSidString);
    }
    return dwError;
}


DWORD
IDMGetComputerName(
    PWSTR* ppszName                      /*    OUT */
    )
{
    DWORD dwError = 0;
    BOOL  bRet = FALSE;
    DWORD dwSize = 0;
    PWSTR pszName = NULL;

    if (!ppszName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    bRet = GetComputerNameEx(ComputerNameNetBIOS, NULL, &dwSize);
    if (!bRet)
    {
        dwError = GetLastError();
        if (dwError != ERROR_MORE_DATA)
        {
            BAIL_ON_ERROR(dwError);
        }
    }

    dwError = IDMAllocateMemory(dwSize * sizeof(WCHAR), (PVOID*)&pszName);
    BAIL_ON_ERROR(dwError);

    bRet = GetComputerNameEx(ComputerNameNetBIOS, pszName, &dwSize);
    if (!bRet)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    *ppszName = pszName;

cleanup:

    return dwError;

error:
    IDM_SAFE_FREE_MEMORY(pszName);

    goto cleanup;
}

DWORD
IDMGetUserInfo(
    HANDLE hClientToken,
    PIDM_USER_INFO * ppUserInfo
    )
{
    DWORD dwError = 0;
    PIDM_USER_INFO pUserInfo = NULL;

    if (!ppUserInfo)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMAllocateMemory(sizeof(IDM_USER_INFO), &pUserInfo);
    BAIL_ON_ERROR(dwError);

    dwError = IDMGetUserName(hClientToken,
                             &pUserInfo->pszUserName,
                             &pUserInfo->pszUserSid);
    BAIL_ON_ERROR(dwError);

    dwError = IDMGetGroupNames(
                    hClientToken,
                    &pUserInfo->ppszGroupNames,
                    &pUserInfo->ppszSids,
                    &pUserInfo->dwNumGroups
                    );
    BAIL_ON_ERROR(dwError);

    *ppUserInfo = pUserInfo;

cleanup:

    return dwError;

error:
    if (pUserInfo)
    {
        IDMFreeUserInfo(pUserInfo);
    }

    goto cleanup;
}

DWORD
IDMGetUserName(
    HANDLE hClientToken,
    PWSTR * ppszUserName,
    PWSTR * ppszUserSid
    )
{
    DWORD dwError = 0;
    LPBYTE pBuffer = NULL;
    DWORD dwNumBytes = 0;
    BOOL bRet = FALSE;
    PSID pSid = NULL;
    PWSTR pszUserName = NULL;
    PWSTR pszUserSid = NULL;
    PTOKEN_USER pTokenUser = NULL;
    DWORD dwBytesReturned = 0;
    PWSTR pszName = NULL;
    PWSTR pszSamAcctName = NULL;
    PWSTR pszDomain = NULL;
    PWSTR pszComputerName = NULL;
    PWSTR pszPreferredDomainName = NULL; // Do not free
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    WCHAR szSeparator[] = L"@";
    SIZE_T len = 0;
    PWSTR pwszObjectSid = NULL;

    if (!ppszUserName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    bRet = GetTokenInformation(
                    hClientToken,
                    TokenUser,
                    pBuffer,
                    dwNumBytes,
                    &dwBytesReturned);
    if (!bRet)
    {
        dwError = IDMAllocateMemory(dwBytesReturned, &pBuffer);
        BAIL_ON_ERROR(dwError);
    }

    bRet = GetTokenInformation(
                    hClientToken,
                    TokenUser,
                    pBuffer,
                    dwBytesReturned,
                    &dwBytesReturned);
    if (!bRet)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    pTokenUser = (TOKEN_USER *)pBuffer;

    pSid = pTokenUser->User.Sid;

    dwError = ConvertSidToNameAndDomain(pSid, &pszName, &pszDomain);
    BAIL_ON_ERROR(dwError);

    dwError = IDMGetComputerName(&pszComputerName);
    BAIL_ON_ERROR(dwError);

    if (!_wcsicmp(pszComputerName, pszDomain))
    {
        pszPreferredDomainName = pszDomain;
    }
    else
    {
        dwError = DsGetDcName(
                        NULL,      /* computer name */
                        pszDomain,
                        NULL,      /* domain guid */
                        NULL,      /* site */
                        DS_IS_FLAT_NAME | DS_RETURN_DNS_NAME,
                        &pDcInfo);
        BAIL_ON_ERROR(dwError);

        if (!ConvertSidToStringSid(pSid, &pwszObjectSid))
        {
            dwError = GetLastError();
            BAIL_ON_ERROR(dwError);
        }

        dwError = IDMGetSamAcctName(pwszObjectSid, &pszSamAcctName);
        if (dwError == 0)
        {
            // We believe that the pszAcctName will have the right case
            if (wcscmp(pszSamAcctName, pszName) != 0)
            {
                IDMFreeMemory(pszName);

                pszName = pszSamAcctName;
                pszSamAcctName = NULL;
            }
        }

        pszPreferredDomainName = pDcInfo->DomainName;
    }
    dwError = _IDMSidToString(
                  pSid,
                  &pszUserSid);
    BAIL_ON_ERROR(dwError);

    len = (wcslen(pszName) + wcslen(pszPreferredDomainName)) * sizeof(WCHAR) +
          sizeof(szSeparator);

    dwError = IDMAllocateMemory(len, (PVOID*)&pszUserName);
    BAIL_ON_ERROR(dwError);

    wsprintf(pszUserName, TEXT("%s%s%s"), pszName, &szSeparator[0], pszPreferredDomainName);

    *ppszUserName = pszUserName;
    *ppszUserSid = pszUserSid;
    pszUserSid = NULL;

cleanup:
    IDM_SAFE_FREE_MEMORY(pBuffer);
    IDM_SAFE_FREE_MEMORY(pszName);
    IDM_SAFE_FREE_MEMORY(pszSamAcctName);
    IDM_SAFE_FREE_MEMORY(pszDomain);
    IDM_SAFE_FREE_MEMORY(pszComputerName);
    if (pwszObjectSid)
    {
        LocalFree(pwszObjectSid);
    }

    if (pDcInfo)
    {
        NetApiBufferFree(pDcInfo);
    }

    return dwError;

error:
    IDM_SAFE_FREE_MEMORY(pszUserName);
    IDM_SAFE_FREE_MEMORY(pszUserSid);

    goto cleanup;
}

DWORD
IDMGetSamAcctName(
    PWSTR  pwszObjectSid,
    PWSTR* ppwszAcctName
    )
{
    DWORD dwError = 0;
    PWSTR pwszFilterTemplate = L"(&(objectclass=user)(objectsid=%s))";
    PWSTR pwszFilter = NULL;
    PWCHAR pwszAttrAcctName = L"sAMAccountName";
    PWSTR pwszAttrs[] = { pwszAttrAcctName, NULL };
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    DWORD dwPort = 3268;
    DWORD dwVersion = LDAP_VERSION3;
    LDAP* pLd = NULL;
    LDAPMessage* pMsg = NULL;
    DWORD dwNumEntries = 0;
    PWCHAR* pwszValues = NULL;
    PWSTR pwszAcctName = NULL;
    PWSTR pwszServer = NULL;

    dwError = DsGetDcName(
                    NULL,      /* computer name */
                    NULL,      /* domain name */
                    NULL,      /* domain guid */
                    NULL,      /* site */
                    DS_GC_SERVER_REQUIRED,
                    &pDcInfo);
    BAIL_ON_ERROR(dwError);

    if (pDcInfo->DomainControllerAddressType != DS_INET_ADDRESS)
    {
        dwError = ERROR_NOT_SUPPORTED;
        BAIL_ON_ERROR(dwError);
    }

    pwszServer = pDcInfo->DomainControllerAddress + 2; // skip "\\" prefix

    pLd = ldap_init(pwszServer, dwPort);
    if (!pLd)
    {
        dwError = LdapGetLastError();
        BAIL_ON_ERROR(dwError);
    }

    dwError = ldap_set_option(
                    pLd,
                    LDAP_OPT_PROTOCOL_VERSION,
                    (VOID*)&dwVersion);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_connect(pLd, NULL);
    BAIL_ON_ERROR(dwError);

    dwError = ldap_bind_s(pLd, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                    (wcslen(pwszFilterTemplate) + wcslen(pwszObjectSid) + 1) * sizeof(WCHAR),
                    (PVOID*)&pwszFilter);
    BAIL_ON_ERROR(dwError);

    wsprintf(pwszFilter, pwszFilterTemplate, pwszObjectSid);

    dwError = ldap_search_s(
                    pLd,
                    L"",
                    LDAP_SCOPE_SUBTREE,
                    pwszFilter,
                    pwszAttrs,
                    0,
                    &pMsg);
    BAIL_ON_ERROR(dwError);

    dwNumEntries = ldap_count_entries(pLd, pMsg);

    if (dwNumEntries == 0)
    {
        dwError = ERROR_NO_SUCH_USER;
        BAIL_ON_ERROR(dwError);
    }
    else if (dwNumEntries != 1)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    pwszValues = ldap_get_values(pLd, pMsg, pwszAttrAcctName);
    if (pwszValues)
    {
        DWORD dwNumValues = ldap_count_values(pwszValues);
        size_t len = 0;

        if (dwNumValues != 1)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_ERROR(dwError);
        }

        len = wcslen(pwszValues[0]) + 1;

        dwError = IDMAllocateMemory(len * sizeof(WCHAR), (PVOID*)&pwszAcctName);
        BAIL_ON_ERROR(dwError);

        wcscpy_s(pwszAcctName, len, pwszValues[0]);
    }
    else
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_ERROR(dwError);
    }

    *ppwszAcctName = pwszAcctName;

cleanup:

    if (pDcInfo)
    {
        NetApiBufferFree(pDcInfo);
    }
    if (pwszValues)
    {
        ldap_value_free(pwszValues);
    }
    if (pMsg)
    {
        ldap_msgfree(pMsg);
    }
    if (pLd)
    {
        ldap_unbind(pLd);
    }

    IDM_SAFE_FREE_MEMORY(pwszFilter);

    return dwError;

error:

    IDM_SAFE_FREE_MEMORY(pwszAcctName);

    goto cleanup;
}

DWORD _IdmGetGroupName(
    PLSA_TRANSLATED_NAME pTranslatedName,
    PLSA_TRUST_INFORMATION pDomainInformation,
    PWSTR * ppszDomainName
    )
{
    DWORD dwError = 0;
    DWORD groupFullNameSizeInChars = 0;
    PWSTR pwszGroupName = NULL;
    WCHAR  szSeparator[] = L"\\";

    if (!pTranslatedName || !pDomainInformation || !ppszDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    // MSDN: LSA_UNICODE_STRING
    //   - Length
    //       Specifies the length, in bytes, of the string pointed to by
    //       the Buffer member, not including the terminating null character, if any.
    //   - Buffer
    //       Pointer to a wide character string. Note that the strings returned by
    //       the various LSA functions might not be null-terminated.
    groupFullNameSizeInChars = (pTranslatedName->Name.Length + pDomainInformation->Name.Length)/sizeof(WCHAR) + 2; // separator + terminating NULL
    dwError = IDMAllocateMemory( groupFullNameSizeInChars*sizeof(WCHAR), (PVOID*)&(pwszGroupName) );
    BAIL_ON_ERROR(dwError);

    memcpy( pwszGroupName, pDomainInformation->Name.Buffer, pDomainInformation->Name.Length );
    memcpy( (pwszGroupName + (pDomainInformation->Name.Length/sizeof(WCHAR))),
            &szSeparator, sizeof(WCHAR) );
    memcpy( (pwszGroupName + (pDomainInformation->Name.Length/sizeof(WCHAR) + 1)),
             pTranslatedName->Name.Buffer, pTranslatedName->Name.Length );

    *ppszDomainName = pwszGroupName;
    pwszGroupName = NULL;

error:

    IDM_SAFE_FREE_MEMORY(pwszGroupName);

    return dwError;
}

DWORD
GetNamesFromSids(
    PSID * pSidStructsArray,
    DWORD sidCount,
    PWSTR ** ppGroupsArray,
    PWSTR ** ppSidsArray,
    PDWORD pdwCount)
{
    DWORD i = 0;
    DWORD j = 0;
    DWORD dwError = 0;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    // Used to call LsaLookupSids to lookup Names from SIDs
    LSA_HANDLE hPolicy = NULL;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes = {0}; // not used
    PLSA_REFERENCED_DOMAIN_LIST pDomains = NULL;
    PLSA_TRANSLATED_NAME pNames = NULL;

    // Used to fill in output groups and SID strings.
    DWORD iResolvedCount = 0;
    PWSTR * ppwszGroupNames = NULL;
    PWSTR * ppwszSidStrings = NULL;
    PWSTR pwszSid = NULL;
    PWSTR pwszGroup = NULL;

    // Used for batching the groups query
    DWORD dwMaxBatchSize = 1000;
    DWORD dwCurrentBatchSize = 0;
    DWORD dwLastBatchSize = 0;
    DWORD dwTotalBatchNumber = 0;
    DWORD dwCurrentBatchStartIndex = 0;

    ntStatus = LsaOpenPolicy( NULL, &ObjectAttributes, POLICY_LOOKUP_NAMES, &hPolicy);
    if (ntStatus != STATUS_SUCCESS)
    {
        dwError = LsaNtStatusToWinError(ntStatus);
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMAllocateMemory(
                  sizeof(*ppwszGroupNames) * sidCount,
                  (PVOID) &ppwszGroupNames);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                  sizeof(*ppwszSidStrings) * sidCount,
                  (PVOID) &ppwszSidStrings);
    BAIL_ON_ERROR(dwError);

    // Calculate the total batch number and last batch size
    dwLastBatchSize = sidCount % dwMaxBatchSize;
    dwTotalBatchNumber = sidCount / dwMaxBatchSize;
    if(dwLastBatchSize)
    {
        ++ dwTotalBatchNumber;
    }
    else
    {
        dwLastBatchSize = dwMaxBatchSize;
    }

    // Outer loop for all batches
    for (i=0, iResolvedCount=0; i < dwTotalBatchNumber; i++, dwError = 0)
    {
        dwCurrentBatchSize = dwMaxBatchSize;
        if(i == (dwTotalBatchNumber - 1)) // am I the last batch
        {
            dwCurrentBatchSize = dwLastBatchSize;
        }

        dwCurrentBatchStartIndex = i * dwMaxBatchSize;
        ntStatus = LsaLookupSids(
            hPolicy,
            dwCurrentBatchSize,
            &pSidStructsArray[dwCurrentBatchStartIndex],
            &pDomains,
            &pNames);

        // MSDN: LsaLookupSids
        //   If the function succeeds, the return value is one of the following NTSTATUS values.
        //    - STATUS_SOME_NOT_MAPPED
        //        Some of the SIDs could not be translated. This is an informational-level return value.
        //    - STATUS_SUCCESS
        //        All of the SIDs were found and successfully translated.
        //   If the function fails, the return value is an NTSTATUS code.
        if ( ( ntStatus != STATUS_SUCCESS ) &&
             ( ntStatus != STATUS_SOME_NOT_MAPPED ) )
        {
            dwError = LsaNtStatusToWinError(ntStatus);
            BAIL_ON_ERROR(dwError);
        }

        // Inner loop for current batch
        for (j = 0; j < dwCurrentBatchSize; j++)
        {
            //MSDN: LSA_TRANSLATED_NAME
            // Use
            //    A value from the SID_NAME_USE enumeration that identifies the type of SID.
            //    If Use has one of the following values, one or both of the Name or DomainIndex
            //    members of LSA_TRANSLATED_NAME is not valid. These members are valid if Use has any other value.
            //    - SidTypeDomain
            //       The DomainIndex member is valid, but the Name member is not valid and must be ignored.
            //
            //    - SidTypeInvalid
            //       Both DomainIndex and Name are not valid and must be ignored.
            //
            //    - SidTypeUnknown
            //       Both DomainIndex and Name are not valid and must be ignored.
            //
            //    - SidTypeWellKnownGroup
            //       The Name member is valid, but the DomainIndex member is not valid and must be ignored.
            if ( ( pNames[j].Use != SidTypeDomain ) &&
                 ( pNames[j].Use != SidTypeInvalid ) &&
                 ( pNames[j].Use != SidTypeUnknown ) &&
                 ( pNames[j].Use != SidTypeWellKnownGroup ) &&
                 ( pNames[j].DomainIndex >= 0 ) &&  // as precaution
                 ( ((ULONG)(pNames[j].DomainIndex)) < pDomains->Entries) )  // as precaution
            {
                dwError = _IDMSidToString(
                              pSidStructsArray[dwCurrentBatchStartIndex + j],
                              &pwszSid);
                BAIL_ON_ERROR(dwError);

                ppwszSidStrings[iResolvedCount] = pwszSid;
                pwszSid = NULL;

                // MSDN: LsaLookupSids
                //   The DomainIndex member of each entry in the Names array is the index of
                //   an entry in the Domains array returned in the ReferencedDomains parameter.
                dwError = _IdmGetGroupName(&(pNames[j]), &(pDomains->Domains[pNames[j].DomainIndex]), &pwszGroup);
                BAIL_ON_ERROR(dwError);

                ppwszGroupNames[iResolvedCount] = pwszGroup;
                pwszGroup = NULL;
                iResolvedCount ++;
            }
        }
        if ( pDomains != NULL )
        {
            LsaFreeMemory(pDomains);
            pDomains = NULL;
        }
        if ( pNames != NULL )
        {
            LsaFreeMemory(pNames);
            pNames = NULL;
        }
    }

    *ppGroupsArray = ppwszGroupNames;
    *ppSidsArray = ppwszSidStrings;
    *pdwCount = iResolvedCount;

cleanup:
    if ( pDomains != NULL )
    {
        LsaFreeMemory(pDomains);
        pDomains = NULL;
    }
    if ( pNames != NULL )
    {
        LsaFreeMemory(pNames);
        pNames = NULL;
    }
    if ( hPolicy != NULL )
    {
        LsaClose(hPolicy);
        hPolicy = NULL;
    }

    IDM_SAFE_FREE_MEMORY(pwszSid);
    IDM_SAFE_FREE_MEMORY(pwszGroup);
    return dwError;

error:
    if (dwError)
    {
         for (i=0; i<sidCount; i++)
         {
             IDM_SAFE_FREE_MEMORY(ppwszGroupNames[i]);
             IDM_SAFE_FREE_MEMORY(ppwszSidStrings[i]);
         }
         IDM_SAFE_FREE_MEMORY(ppwszGroupNames);
         IDM_SAFE_FREE_MEMORY(ppwszSidStrings);
    }
    goto cleanup;
}


DWORD
IDMGetGroupNames(
    HANDLE hClientToken,
    PWSTR ** ppGroupsArray,
    PWSTR ** ppSidsArray,
    DWORD * pdwCount
    )
{
    DWORD i = 0;
    DWORD dwError = 0;
    BOOL bRet = FALSE;

    // Used to get token groups from client token.
    LPBYTE pBuffer = NULL;
    DWORD dwNumBytes = 0;
    DWORD dwBytesReturned = 0;
    DWORD dwGroupCount = 0;
    PTOKEN_GROUPS pTokenGroups = NULL;

    // Used to find if the sid from token groups is one of known sids
    DWORD iUnMappedFromSidCache = 0;
    PWSTR pwszLookupSid = NULL;

    // Used to construct the Sid stuct array to call GetNamesFromSids.
    PSID pSid = NULL;
    PSID * pSidStructsArray = NULL;

    // Check inputs
    if (!ppGroupsArray || !ppSidsArray || !pdwCount)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    bRet = GetTokenInformation(
                    hClientToken,
                    TokenGroups,
                    pBuffer,
                    dwNumBytes,
                    &dwBytesReturned
                    );
    if (!bRet)
    {
        dwError = GetLastError();
        if (dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            BAIL_ON_ERROR(dwError);
        }

        dwError = IDMAllocateMemory(dwBytesReturned, &pBuffer);
        BAIL_ON_ERROR(dwError);
    }

    bRet = GetTokenInformation(
                    hClientToken,
                    TokenGroups,
                    pBuffer,
                    dwBytesReturned,
                    &dwBytesReturned);
    if (!bRet)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    pTokenGroups = (TOKEN_GROUPS *)pBuffer;
    dwGroupCount = pTokenGroups->GroupCount;

    if ( dwGroupCount > 0 )
    {
        dwError = IDMAllocateMemory(
                        sizeof(PSID)*dwGroupCount,
                        (PSID *)&pSidStructsArray
                        );
        BAIL_ON_ERROR(dwError);

        // filter out sids to ignore
        for (i = 0; i < dwGroupCount; i++)
        {
            pSid = pTokenGroups->Groups[i].Sid;
            dwError = FindSidCacheEntry(
                          IDMGetBuiltinSidCache(),
                          pSid,
                          &pwszLookupSid);
            if (dwError && !pwszLookupSid)
            {
                pSidStructsArray[iUnMappedFromSidCache] = pSid;
                iUnMappedFromSidCache++;
            }
            IDM_SAFE_FREE_MEMORY(pwszLookupSid);
        }

        if (iUnMappedFromSidCache > 0)
        {
            dwError = GetNamesFromSids(
                        pSidStructsArray,
                        iUnMappedFromSidCache,
                        ppGroupsArray,
                        ppSidsArray,
                        pdwCount);
            BAIL_ON_ERROR(dwError);
        }
        else
        {
            *ppGroupsArray = NULL;
            *ppSidsArray = NULL;
            *pdwCount = 0;
        }
    }

error:
    IDM_SAFE_FREE_MEMORY(pSidStructsArray);
    IDM_SAFE_FREE_MEMORY(pBuffer);
    IDM_SAFE_FREE_MEMORY(pwszLookupSid);
    return dwError;
}

DWORD
ConvertSidToName(
    PSID pSid,
    PWSTR *ppszFullName
    )
{
    DWORD  dwError = 0;
    PWSTR  pszName = NULL;
    PWSTR  pszDomain = NULL;
    PWSTR  pszFullName = NULL;
    SIZE_T lenFullName = 0;
    WCHAR  szSeparator[] = L"\\";

    if (!pSid || !ppszFullName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = ConvertSidToNameAndDomain(pSid, &pszName, &pszDomain);
    BAIL_ON_ERROR(dwError);

    lenFullName =   (wcslen(pszName) + wcslen(pszDomain)) * sizeof(WCHAR) +
                    sizeof(szSeparator);

    dwError = IDMAllocateMemory(lenFullName, (PVOID*)&pszFullName);
    BAIL_ON_ERROR(dwError);

    wsprintf(pszFullName, TEXT("%s%s%s"), pszDomain, &szSeparator[0], pszName);

    *ppszFullName = pszFullName;

cleanup:

    IDM_SAFE_FREE_MEMORY(pszName);
    IDM_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:
    IDM_SAFE_FREE_MEMORY(pszFullName);

    goto cleanup;
}

DWORD
ConvertSidToNameAndDomain(
    PSID    pSid,
    PWSTR* ppszName,
    PWSTR* ppszDomain
    )
{
    DWORD dwError = 0;
    PWSTR pszName = NULL;
    PWSTR pszDomain = NULL;
    BOOL bRet = FALSE;
    DWORD cchName = 0;
    DWORD cchDomain = 0;
    SID_NAME_USE eUse;

    if (!ppszName || !ppszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    bRet = LookupAccountSid(
                    NULL,
                    pSid,
                    NULL,
                    &cchName,
                    NULL,
                    &cchDomain,
                    &eUse);
    if (!bRet)
    {
        dwError = GetLastError();
        if (dwError != ERROR_INSUFFICIENT_BUFFER)
        {
            BAIL_ON_ERROR(dwError);
        }
    }

    dwError = IDMAllocateMemory(cchName * sizeof(WCHAR), (PVOID*)&pszName);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(cchDomain * sizeof(WCHAR), (PVOID*)&pszDomain);
    BAIL_ON_ERROR(dwError);

    bRet = LookupAccountSid(
                    NULL,
                    pSid,
                    pszName,
                    &cchName,
                    pszDomain,
                    &cchDomain,
                    &eUse);
    if (!bRet)
    {
        dwError = GetLastError();
        BAIL_ON_ERROR(dwError);
    }

    *ppszName = pszName;
    *ppszDomain = pszDomain;

cleanup:

    return dwError;

error:
    IDM_SAFE_FREE_MEMORY(pszName);
    IDM_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}

VOID
IDMFreeUserInfo(
    PIDM_USER_INFO pUserInfo
    )
{
    if (pUserInfo)
    {
        DWORD iGroup = 0;

        IDM_SAFE_FREE_MEMORY(pUserInfo->pszUserName);
        IDM_SAFE_FREE_MEMORY(pUserInfo->pszUserSid);
        if (pUserInfo->ppszGroupNames)
        {
            for (iGroup=0; iGroup < pUserInfo->dwNumGroups; iGroup++)
            {
                IDM_SAFE_FREE_MEMORY(pUserInfo->ppszGroupNames[iGroup]);
            }
            IDM_SAFE_FREE_MEMORY(pUserInfo->ppszGroupNames);
        }
        if (pUserInfo->ppszSids)
        {
            for (iGroup=0; iGroup < pUserInfo->dwNumGroups; iGroup++)
            {
                IDM_SAFE_FREE_MEMORY(pUserInfo->ppszSids[iGroup]);
            }
            IDM_SAFE_FREE_MEMORY(pUserInfo->ppszSids);
        }

        IDM_SAFE_FREE_MEMORY(pUserInfo);
    }
}
