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
 *        prototypes.h
 *
 * Abstract:
 *
 *        Identity Manager - Active Directory Integration
 *
 *        Function prototypes
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

// sidcache.c

DWORD
IDMInitializeSidCache(
    VOID
    );

DWORD
IDMDestroySidCache(
    VOID
    );

DWORD
InitializeSidCache(
    PSID_CACHE pSidCache
    );

DWORD
DestroySidCache(
    PSID_CACHE  pSidCache
    );

DWORD
AddSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID       pSid,
    PWSTR     pszName
    );

DWORD
FindSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID       pSid,
    PWSTR*    ppszName
    );

DWORD
FindSidCacheEntryWithLock(
    PSID_CACHE pSidCache,
    PSID       pSid,
    PWSTR*    ppszName,
    PSID_ENTRY* ppSidEntry // Optional, NULL is OK
    );

DWORD
DeleteSidCacheEntry(
    PSID_CACHE pSidCache,
    PSID       pSid
    );

DWORD
CreateSidCacheEntry(
    PSID        pSid,
    PWSTR      pszName,
    PSID_ENTRY* ppSidEntry
    );

VOID
FreeSidEntry(
    PSID_ENTRY pSidEntry
    );

PSID_CACHE
IDMGetBuiltinSidCache(
    VOID
    );

// tokenmgmt.c

DWORD
IDMGetUserName(
    HANDLE  hClientToken,
    PWSTR* ppszUserName,
    PWSTR * ppszUserSid
    );

DWORD
ConvertSidToName(
    PSID    pSid,
    PWSTR* ppszFullName
    );

DWORD
ConvertSidToNameAndDomain(
    PSID    pSid,
    PWSTR* ppszName,
    PWSTR* ppszDomain
    );

DWORD
IDMGetSamAcctName(
    PWSTR  pwszObjectSid,
	PWSTR* ppwszAcctName
	);

DWORD
IDMGetGroupNames(
    HANDLE   hClientToken,
    PWSTR** ppStringArray,
    PWSTR** ppSidsArray,
    DWORD*   pdwCount
    );

DWORD
IDMGetUserInfo(
    HANDLE          hClientToken,
    PIDM_USER_INFO* ppUserInfo
    );

VOID
IDMFreeUserInfo(
    PIDM_USER_INFO pUserInfo
    );

