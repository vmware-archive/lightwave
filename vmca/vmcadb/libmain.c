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
 * Module Name: VMware Certificate Server
 *
 * Filename: externs.h
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Library Entry points
 *
 */

#include "includes.h"

int _forceCRTManifestCUR = 0;

DWORD
VmcaDbInitialize(
    PCSTR pszDbPath
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszDbPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMCA_ERROR(dwError);
    }

    dwError = VmcaDbDatabaseInitialize(pszDbPath);
    BAIL_ON_VMCA_ERROR(dwError);

    dwError = VMCAAllocateStringA(pszDbPath, &gVmcaDbGlobals.pszDbPath);
    BAIL_ON_VMCA_ERROR(dwError);

error:

    return dwError;
}

VOID
VmcaDbShutdown(
    VOID
    )
{
    BOOLEAN bInLock = FALSE;

    VMCA_LOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);

    if (gVmcaDbGlobals.pDbContextList)
    {
        VmcaDbFreeContext(gVmcaDbGlobals.pDbContextList);

        gVmcaDbGlobals.pDbContextList = NULL;
        gVmcaDbGlobals.dwNumCachedContexts = 0;
    }

    VMCA_SAFE_FREE_MEMORY(gVmcaDbGlobals.pszDbPath);
    gVmcaDbGlobals.pszDbPath = NULL;

    VMCA_UNLOCK_MUTEX(bInLock, &gVmcaDbGlobals.mutex);

}

DWORD
VmcaDbReset(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VmcaDbResetAppDatabase();

    return dwError;
}

VOID
VmcaDbFreeCertEntry(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    VmcaDbFreeCertEntryContents(pCertEntry);

    VMCAFreeMemory(pCertEntry);
}

VOID
VmcaDbFreeCertEntryArray(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntryArray,
    DWORD                      dwCount
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwCount; iEntry++)
    {
        VmcaDbFreeCertEntryContents(&pCertEntryArray[iEntry]);
    }

    VMCAFreeMemory(pCertEntryArray);
}

VOID
VmcaDbFreeCertEntryContents(
    PVMCA_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszCommonName);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszAltNames);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszOrgName);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszOrgUnitName);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszIssuerName);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszCountryName);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszSerial);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pCertBlob);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszTimeValidFrom);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pwszTimeValidTo);
    VMCA_SAFE_FREE_MEMORY(pCertEntry->pCertBlob);
}
