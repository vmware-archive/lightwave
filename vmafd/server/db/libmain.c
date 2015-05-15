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

DWORD
VecsDbInitialize(
    PCSTR pszDbPath
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszDbPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VECS_ERROR(dwError);
    }

    dwError = VecsDbDatabaseInitialize(pszDbPath);
    BAIL_ON_VECS_ERROR(dwError);

    dwError = VmAfdAllocateStringA(pszDbPath, &gVecsDbGlobals.pszDbPath);
    BAIL_ON_VECS_ERROR(dwError);

error:

    return dwError;
}

VOID
VecsDbShutdown(
    VOID
    )
{
    VECS_DB_LOCK_MUTEX(&gVecsDbGlobals.mutex);

    if (gVecsDbGlobals.pDbContextList)
    {
        VecsDbFreeContext(gVecsDbGlobals.pDbContextList);

        gVecsDbGlobals.pDbContextList = NULL;
        gVecsDbGlobals.dwNumCachedContexts = 0;
    }

    VMAFD_SAFE_FREE_MEMORY(gVecsDbGlobals.pszDbPath);
    gVecsDbGlobals.pszDbPath = NULL;


    VECS_DB_UNLOCK_MUTEX(&gVecsDbGlobals.mutex);

}

DWORD
VecsDbReset(
    VOID
    )
{
    DWORD dwError = 0;

    dwError = VecsDbResetAppDatabase();

    return dwError;
}

VOID
VecsDbFreeCertEntry(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    VecsDbFreeCertEntryContents(pCertEntry);

    VMAFD_SAFE_FREE_MEMORY(pCertEntry);
}

VOID
VecsDbFreeCertEntryArray(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntryArray,
    DWORD                      dwCount
    )
{
    DWORD iEntry = 0;

    for (; iEntry < dwCount; iEntry++)
    {
        VecsDbFreeCertEntryContents(&pCertEntryArray[iEntry]);
    }

    VMAFD_SAFE_FREE_MEMORY(pCertEntryArray);
}

VOID
VecsDbFreeCertEntryContents(
    PVECS_DB_CERTIFICATE_ENTRY pCertEntry
    )
{
    VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszAlias);
    VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszSerial);
    VMAFD_SAFE_FREE_MEMORY(pCertEntry->pCertBlob);
    VMAFD_SAFE_FREE_MEMORY(pCertEntry->pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pCertEntry->pPrivateKey);
}


