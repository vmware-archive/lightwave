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

    dwError = VmGetLWUserGroupId();
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
 * merge filenamefrom content into filenameinto
 *
 * New keys will be added.
 * Existing keys will NOT be overridden.
 */
DWORD
VmRegConfigMergeFile(
    PCSTR              pszFileNameFrom,
    PCSTR              pszFileNameInto
    )
{
    DWORD   dwError = 0;
    PSTR    pszTopKey = NULL;
    CHAR    keyBuf[VM_SIZE_512] = {0};
    CHAR    valueBuf[VM_SIZE_4096] = {0};
    size_t  valueBufSize = sizeof(valueBuf);
    PVM_REGCONFIG_LIST_ENTRY    pEntry = NULL;
    PVM_REGCONFIG_LIST_KV       pKV = NULL;

    if (!pszFileNameFrom || !pszFileNameInto)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = VmRegConfigAddFile(pszFileNameFrom, TRUE);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmRegConfigAddFile(pszFileNameInto, FALSE);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (pEntry = _gpVmRegConfig->pListEntry; pEntry; pEntry = pEntry->pNext)
    {
        if (!pszTopKey)
        {
            pszTopKey = pEntry->pszTopKey;
        }
        else
        {
            if (VmStringCompareA(pszTopKey, pEntry->pszTopKey, FALSE) != 0)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_INVALID_MERGE);
            }
        }

        if (VmStringCompareA(pszFileNameFrom, pEntry->pszFileName, FALSE) == 0)
        {
            pKV = pEntry->pListKV;
            break;
        }
    }

    for (; pKV; pKV = pKV->pNext)
    {
        dwError = VmStringPrintFA(
                keyBuf, VM_SIZE_512,
                "%s%s", VM_REGCONFIG_TOP_KEY_PATH, pKV->pszKey);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        valueBufSize = sizeof(valueBuf);
        dwError = VmRegConfigGetKeyA(keyBuf, valueBuf, &valueBufSize);
        if (dwError == LWREG_ERROR_NO_SUCH_KEY_OR_VALUE)
        {
            dwError = VmRegConfigSetKeyA(keyBuf, pKV->pszValue, VmStringLenA(pKV->pszValue));
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
}

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

            dwError = VmRegConfigLockFile(pEntry->pszLockFileName, &fd);
            BAIL_ON_VM_COMMON_ERROR(dwError);

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

            dwError = VmRegConfigLockFile(pEntry->pszLockFileName, &fd);
            BAIL_ON_VM_COMMON_ERROR(dwError);

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
