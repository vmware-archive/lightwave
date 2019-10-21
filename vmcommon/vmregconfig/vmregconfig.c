/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

/*
 * Initialize the regconfig context
 */
DWORD
VmRegConfigInit(
    VOID
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_CONTEXT pContext = NULL;

    dwError = VmRegConfigGetLWUserGroupId();
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateMemory(
            sizeof(VM_REGCONFIG_CONTEXT),
            (PVOID*)&pContext);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    _gpVmRegConfig = pContext;

cleanup:
    return dwError;

error:
    VM_COMMON_SAFE_FREE_MEMORY(pContext);
    goto cleanup;
}

/*
 * Free the config context
 */
VOID
VmRegConfigFree(
    VOID
    )
{
    if (_gpVmRegConfig)
    {
        VmRegConfigListEntryFree(_gpVmRegConfig->pListEntry);
        VM_COMMON_SAFE_FREE_MEMORY(_gpVmRegConfig);
    }
}

/*
 * add config file to context
 */
DWORD
VmRegConfigAddFile(
    PCSTR               pszFileName,
    BOOLEAN             bReadOnly
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;

    if (!pszFileName)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (!_gpVmRegConfig)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_STATE);
    }

    dwError = VmAllocateMemory(
            sizeof(VM_REGCONFIG_LIST_ENTRY),
            (PVOID*)&pEntry);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateMutex(&pEntry->pMutex);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringA(pszFileName, &pEntry->pszFileName);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringPrintf(
            &pEntry->pszLockFileName,
            "/tmp/%s.lock",
            VmStringRChrA(pszFileName, '/') + 1);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = stat(pEntry->pszFileName, &pEntry->fileStat);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pEntry->bReadOnly = bReadOnly;

    dwError = VmRegConfigReadFileInternal(pEntry);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    // TODO, this block should check for duplicate file
    if (_gpVmRegConfig->pListEntry)
    {
        pEntry->pNext = _gpVmRegConfig->pListEntry;
        _gpVmRegConfig->pListEntry = pEntry;
    }
    else
    {
        _gpVmRegConfig->pListEntry = pEntry;
    }

cleanup:
    return dwError;

error:
    VmRegConfigListEntryFree(pEntry);
    goto cleanup;
}

/*
 * delete config file in context
 */
DWORD
VmRegConfigDeleteFile(
    PCSTR               pszFileName
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;
    PVM_REGCONFIG_LIST_ENTRY    pPrevEntry = NULL;
    BOOLEAN                     bInLock = FALSE;

    if (!pszFileName)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (!_gpVmRegConfig)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_STATE);
    }

    for (pEntry = _gpVmRegConfig->pListEntry; pEntry; pPrevEntry = pEntry, pEntry = pEntry->pNext)
    {
        if (VmStringCompareA(pszFileName, pEntry->pszFileName, FALSE) == 0)
        {
            VM_LOCK_MUTEX(bInLock, pEntry->pMutex);

            if (pPrevEntry)
            {
                pPrevEntry->pNext = pEntry->pNext;
            }
            else
            {
                _gpVmRegConfig->pListEntry = pEntry->pNext;
            }

            break;
        }
    }

    if (!pEntry)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_NOT_FOUND);
    }

    VmRegConfigListEntryFree(pEntry);

cleanup:
    VM_UNLOCK_MUTEX(bInLock, pEntry->pMutex);

    return dwError;

error:
    goto cleanup;
}

/*
 * merge new into current config
 */
DWORD
VmRegConfigMergeFile(
    PCSTR              pszCurrentFileName,
    PCSTR              pszNewFileName
    );

/*
 * get key value
 */
DWORD
VmRegConfigGetKeyA(
    PCSTR               pszKeyName,
    PSTR                pszValue,     /* out */
    size_t*             piValueSize  /* in/out */
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;
    BOOLEAN                     bInLock = FALSE;

    if (!pszKeyName || !pszValue || !piValueSize)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (!_gpVmRegConfig)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_STATE);
    }

    for (pEntry = _gpVmRegConfig->pListEntry; pEntry; pEntry = pEntry->pNext)
    {
        if (VmStringStartsWithA(pszKeyName, pEntry->pszTopKey, FALSE))
        {
            VM_LOCK_MUTEX(bInLock, pEntry->pMutex);

            dwError = VmRegConfigForceReadInternal(pEntry);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = VmRegConfigGetKeyInternal(pEntry, pszKeyName, pszValue, piValueSize);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            break;
        }
    }

    if (!pEntry)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND);
    }

cleanup:
    VM_UNLOCK_MUTEX(bInLock, pEntry->pMutex);

    return dwError;

error:
    if (dwError == VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND)
    {
        dwError = LWREG_ERROR_NO_SUCH_KEY_OR_VALUE;
    }
    goto cleanup;
}

/*
 * get MultiSZ key value
 * 
 * out pszValue must be in this format "value1\0value2\0"
 */
DWORD
VmRegConfigGetMultiSZKeyA(
    PCSTR               pszKeyName,
    PSTR                pszValue,     /* out */
    size_t*             piValueSize   /* in/out */
    )
{
    DWORD  dwError = 0;
    DWORD  dwCnt = 0;
    size_t iSize = 0;

    if (piValueSize)
    {
        iSize = *piValueSize;
    }

    dwError = VmRegConfigGetKeyA( pszKeyName, pszValue, piValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; pszValue[dwCnt] != '\0'; dwCnt++)
    {
        if (pszValue[dwCnt] == '\n')
        {
            pszValue[dwCnt] = '\0';
        }
    }

    if (pszValue[dwCnt-1] != '\0')
    {
        if (dwCnt+1 >= iSize)
	{
            BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_VALUE_TOO_BIG);
	}

        *piValueSize = dwCnt+1;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * set MultiSZ key value
 *
 * pszValue must be in this format "value1\0value2\0"
 */
DWORD
VmRegConfigSetMultiSZKeyA(
    PCSTR               pszKeyName,
    PCSTR               pszValue,
    size_t              iValueSize
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszLocalStr = NULL;

    dwError = VmAllocateMemory(iValueSize+2, (PVOID*)&pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmCopyMemory(
            pszLocalStr,
            iValueSize+2,
            pszValue,
            iValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; dwCnt < iValueSize; dwCnt++)
    {
        if (pszLocalStr[dwCnt] == '\0')
        {
            pszLocalStr[dwCnt] = '\n';
        }
    }

    if (pszLocalStr[iValueSize-1] != '\n')
    {   // make sure every SZ ends with newline
        pszLocalStr[iValueSize] = '\n';
        iValueSize++;
    }

    dwError = VmRegConfigSetKeyA(pszKeyName, pszLocalStr, iValueSize);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocalStr);
    return dwError;

error:
    goto cleanup;
}

/*
 * set or create key value
 */
DWORD
VmRegConfigSetKeyA(
    PCSTR               pszKeyName,
    PCSTR               pszValue,
    size_t              iValueSize
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;
    BOOLEAN                     bInLock = FALSE;
    int                         fd = -1;

    if (!pszKeyName || !pszValue || iValueSize == 0)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (!_gpVmRegConfig)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_STATE);
    }

    for (pEntry = _gpVmRegConfig->pListEntry; pEntry; pEntry = pEntry->pNext)
    {
        if (VmStringStartsWithA(pszKeyName, pEntry->pszTopKey, FALSE))
        {
            VM_LOCK_MUTEX(bInLock, pEntry->pMutex);

            if (pEntry->bReadOnly)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_OPERATION_NOT_PERMITTED);
            }

            if ((fd = open(pEntry->pszLockFileName, O_CREAT, S_IRUSR| S_IWUSR)) == -1)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
            }

            if (flock(fd, LOCK_EX) == -1)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
            }

            dwError = VmRegConfigForceReadInternal(pEntry);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = VmRegConfigSetKeyInternal(pEntry, pszKeyName, pszValue, iValueSize);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            break;
        }
    }

    if (!pEntry)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND);
    }

cleanup:
    VM_UNLOCK_MUTEX(bInLock, pEntry->pMutex);
    if (fd != -1)
    {
        flock(fd, LOCK_UN);
        close(fd);
    }

    return dwError;

error:
    if (dwError == VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND)
    {
        dwError = LWREG_ERROR_NO_SUCH_KEY_OR_VALUE;
    }
    goto cleanup;
}

/*
 * delete key
 */
DWORD
VmRegConfigDeleteKeyA(
    PCSTR               pszKeyName
    )
{
    DWORD dwError = 0;
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;
    BOOLEAN                     bInLock = FALSE;
    int                         fd = -1;

    if (!pszKeyName)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if (!_gpVmRegConfig)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_STATE);
    }

    for  (pEntry = _gpVmRegConfig->pListEntry; pEntry; pEntry = pEntry->pNext)
    {
        if (VmStringStartsWithA(pszKeyName, pEntry->pszTopKey, FALSE))
        {
            VM_LOCK_MUTEX(bInLock, pEntry->pMutex);

            if (pEntry->bReadOnly)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_OPERATION_NOT_PERMITTED);
            }

            if ((fd = open(pEntry->pszLockFileName, O_CREAT, S_IRUSR| S_IWUSR)) == -1)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
            }

            if (flock(fd, LOCK_EX) == -1)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
            }

            dwError = VmRegConfigForceReadInternal(pEntry);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = VmRegConfigDeleteKeyInternal(pEntry, pszKeyName);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            break;
        }
    }

    if (!pEntry)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND);
    }

cleanup:
    VM_UNLOCK_MUTEX(bInLock,pEntry->pMutex);
    if (fd != -1)
    {
        flock(fd, LOCK_UN);
        close(fd);
    }

    return dwError;

error:
    if (dwError == VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND)
    {
        dwError = LWREG_ERROR_NO_SUCH_KEY_OR_VALUE;
    }
    goto cleanup;
}
