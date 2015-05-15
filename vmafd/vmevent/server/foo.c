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
 * Module Name: ThinAppRepoService
 *
 * Filename: libmain.c
 *
 * Abstract:
 *
 * Thinapp Repository Server Utilities
 *
 */
#include "includes.h"

VOID
RepoFreePkgContainer(
    PREPO_PACKAGE_CONTAINER pContainer
    )
{
    if (pContainer->pPkgEntries)
    {
        RepoFreePkgEntryArray(pContainer->pPkgEntries, pContainer->dwCount);
    }

    RepoFreeMemory(pContainer);
}

VOID
RepoFreePkgEntryArray(
    PREPO_PACKAGE_ENTRY pPkgEntryArray,
    DWORD               dwCount
    )
{
    int iEntry = 0;

    for (; iEntry < dwCount; iEntry++ )
    {
        RepoFreePkgEntryContents(&pPkgEntryArray[iEntry]);
    }

    RepoFreeMemory(pPkgEntryArray);
}

VOID
RepoFreePkgEntryContents(
    PREPO_PACKAGE_ENTRY pPkgEntry
    )
{
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgName);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgDesc);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgGuid);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgPath);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgBinPath);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pszPkgBinFile);
    REPO_SAFE_FREE_MEMORY(pPkgEntry->pIconBlob);

    if (pPkgEntry->pPkgAppEntries != NULL)
    {
        RepoFreePkgAppEntryArray(pPkgEntry->pPkgAppEntries,
                                 pPkgEntry->dwAppCount);
    }

    REPO_SAFE_FREE_MEMORY(pPkgEntry->pPkgAppEntries);
}

VOID
RepoFreePkgAppEntryArray(
    PREPO_PKG_APP_ENTRY pPkgAppEntryArray,
    DWORD               dwCount
    )
{
    int iEntry = 0;

    for (; iEntry < dwCount; iEntry++)
    {
        RepoFreePkgAppEntryContents(&pPkgAppEntryArray[iEntry]);
    }
}

VOID
RepoFreePkgAppEntryContents(
    PREPO_PKG_APP_ENTRY pPKgAppEntry
    )
{
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppName);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppDesc);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppGuid);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppIconUrl);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppLaunchUrl);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppBinPath);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pszAppBinFile);
    REPO_SAFE_FREE_MEMORY(pPKgAppEntry->pIconBlob);
}
