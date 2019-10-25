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

static
DWORD
_VmRegConfigWriteBuf(
    PVM_WRITE_KV            pWrite,
    PCSTR                   pszStr
    )
{
    DWORD   dwError = 0;
    DWORD   dwStrLen = VmStringLenA(pszStr);

    if (dwStrLen + pWrite->iBufLen >= pWrite->iBufSize)
    {
        dwError = VmReallocateMemoryWithInit(
                (PVOID)pWrite->pszBuf,
                pWrite->iBufSize,
                (PVOID*)&pWrite->pszBuf,
                pWrite->iBufSize * 2);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pWrite->iBufSize *= 2;
    }

    dwError = VmStringNCatA(
            pWrite->pszBuf,
            pWrite->iBufSize,
            pszStr,
            dwStrLen);
    BAIL_ON_VM_COMMON_ERROR(dwError);
    pWrite->iBufLen += dwStrLen;

    dwError = VmStringNCatA(
            pWrite->pszBuf,
            pWrite->iBufSize,
            "\n",
            1);

    BAIL_ON_VM_COMMON_ERROR(dwError);
    pWrite->iBufLen += 1;


error:
    return dwError;
}

DWORD
_OrgVmRegConfigWriteMultiSZBuf(
    PVM_WRITE_KV            pWrite,
    PSTR                    pszMultiSZStr
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwIdx = 0;
    DWORD   dwStrLen = VmStringLenA(pszMultiSZStr);
    PSTR    pszLocalStr = NULL;
    PSTR    pszStart = NULL;

    dwError = VmAllocateStringA(pszMultiSZStr, &pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmRegConfigWriteBuf(pWrite, VM_REGCONFIG_MULTISZ_STR);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0, pszStart = pszLocalStr; dwCnt<dwStrLen; dwCnt++)
    {
        if (pszLocalStr[dwCnt] == '\n')
        {
            CHAR    pszTmpStr[VM_SIZE_1024] = {0};

            pszLocalStr[dwCnt] = '\0';

            for (dwIdx=0; dwIdx < pWrite->pSubKeyList->dwCount; dwIdx++)
            {
                dwError = VmStringNCatA(
                        pszTmpStr,
                        VM_SIZE_1024,
                        VM_REGCONFIG_INDENTATION_STR,
                        VM_REGCONFIG_INDENTATION);
                BAIL_ON_VM_COMMON_ERROR(dwError);
            }

            dwError = VmStringNCatA(
                    pszTmpStr,
                    VM_SIZE_1024,
                    pszStart,
                    VmStringLenA(pszStart));
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = _VmRegConfigWriteBuf(pWrite, pszTmpStr);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            pszStart = pszLocalStr + dwCnt + 1;
        }
    }

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocalStr);
    return dwError;
}

static
DWORD
_VmRegConfigWriteMultiSZBuf(
    PVM_WRITE_KV            pWrite,
    PSTR                    pszMultiSZStr
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    PSTR    pszLocalStr = NULL;
    DWORD   dwLocalStrLen = 0;

    PVM_STRING_LIST pStrList = NULL;

    dwError = VmStringToTokenList(
            pszMultiSZStr,
            VM_REGCONFIG_NEWLINE_STR,
            &pStrList);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwLocalStrLen = VmStringLenA(pszMultiSZStr) + pStrList->dwCount + 1;
    dwError = VmAllocateMemory(dwLocalStrLen, (PVOID*)&pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    for (dwCnt=0; dwCnt < pStrList->dwCount; dwCnt++)
    {
        dwError = VmStringNCatA(
                pszLocalStr,
                dwLocalStrLen,
                pStrList->pStringList[dwCnt],
                VmStringLenA(pStrList->pStringList[dwCnt]));
        BAIL_ON_VM_COMMON_ERROR(dwError);

        if (dwCnt < pStrList->dwCount - 1)
        {
            dwError = VmStringNCatA(
                    pszLocalStr,
                    dwLocalStrLen,
                    "\\n",
                    2);
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }

    dwError = _VmRegConfigWriteBuf(pWrite, pszLocalStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:
    VM_COMMON_SAFE_FREE_MEMORY(pszLocalStr);
    return dwError;
}

static
DWORD
_VmRegConfigWriteMeta(
    PVM_WRITE_KV            pWrite
    )
{
    DWORD   dwError = 0;
    DWORD   dwIdx = 0;
    CHAR    pszStr[VM_SIZE_1024] = {0};
    DWORD   dwStrListCnt = pWrite->pSubKeyList->dwCount;
    PCSTR   pszLastStr = pWrite->pSubKeyList->pStringList[dwStrListCnt-1];
    DWORD   dwLastStrLen  = VmStringLenA(pszLastStr);

    if (pWrite->pSubKeyList->dwCount > 1 && pWrite->bPrefixNL)
    {
        dwError = VmStringNCatA(
                pszStr,
                VM_SIZE_1024,
                "\n",
                1);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    for (dwIdx=0; dwStrListCnt > 0 && dwIdx < dwStrListCnt-1; dwIdx++)
    {
        dwError = VmStringNCatA(
                pszStr,
                VM_SIZE_1024,
                VM_REGCONFIG_INDENTATION_STR,
                VM_REGCONFIG_INDENTATION);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    dwError = VmStringNCatA(
            pszStr,
            VM_SIZE_1024,
            pszLastStr,
            dwLastStrLen);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmStringNCatA(
            pszStr,
            VM_SIZE_1024,
            ":",
            1);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmRegConfigWriteBuf(pWrite, pszStr);
    BAIL_ON_VM_COMMON_ERROR(dwError);

error:
    return dwError;
}

static
DWORD
_VmRegConfigDQuoteValue(
    PVM_REGCONFIG_LIST_KV   pKV,
    PVM_WRITE_KV            pWrite
    )
{
    DWORD   dwError = 0;
    PSTR    pszDQuote = NULL;
    PSTR    pszStart = NULL;
    DWORD   dwLen = 0;
    DWORD   dwQuote = 0;
    DWORD   dwValueLen = VmStringLenA(pKV->pszValue);

    pszDQuote = VmStringChrA(pKV->pszValue, VM_REGCONFIG_KEY_DQUOTE);
    while (pszDQuote)
    {
        dwQuote++;
        pszDQuote = VmStringChrA(pszDQuote+1, VM_REGCONFIG_KEY_DQUOTE);
    }

    if (dwValueLen + dwQuote + 2 + 1 > pWrite->iQuoteBufSize)
    {
        dwError = VmReallocateMemoryWithInit(
                (PVOID)pWrite->pszValueQuoteBuf,
                pWrite->iQuoteBufSize,
                (PVOID*)&pWrite->pszValueQuoteBuf,
                pWrite->iQuoteBufSize * 2);
        BAIL_ON_VM_COMMON_ERROR(dwError);

        pWrite->iQuoteBufSize *= 2;
    }

    memset(pWrite->pszValueQuoteBuf, 0, pWrite->iQuoteBufSize);
    pWrite->pszValueQuoteBuf[0] = VM_REGCONFIG_KEY_DQUOTE;
    dwLen++;

    pszStart = pKV->pszValue;
    pszDQuote = VmStringChrA(pKV->pszValue, VM_REGCONFIG_KEY_DQUOTE);
    while (pszDQuote)
    {
        dwError = VmCopyMemory(
                pWrite->pszValueQuoteBuf + dwLen,
                pWrite->iQuoteBufSize - dwLen,
                pszStart,
                pszDQuote - pszStart);
        BAIL_ON_VM_COMMON_ERROR(dwError);
        dwLen += (pszDQuote - pszStart);

        pWrite->pszValueQuoteBuf[dwLen++] = VM_REGCONFIG_KEY_ESCAPE;

        pszStart = pszDQuote;
        pszDQuote = VmStringChrA(pszDQuote+1, VM_REGCONFIG_KEY_DQUOTE);
    }

    dwError = VmCopyMemory(
            pWrite->pszValueQuoteBuf + dwLen,
            pWrite->iQuoteBufSize - dwLen,
            pszStart,
            VmStringLenA(pszStart));
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwLen += VmStringLenA(pszStart);

    pWrite->pszValueQuoteBuf[dwLen] = VM_REGCONFIG_KEY_DQUOTE;

error:
    return dwError;
}

static
DWORD
_VmRegConfigWriteValue(
    PVM_REGCONFIG_LIST_KV   pKV,
    PVM_WRITE_KV            pWrite
    )
{
    DWORD   dwError = 0;
    PSTR    pszValue = pWrite->pszValueQuoteBuf;

    pWrite->pszBuf[pWrite->iBufLen-1] = ' ';  // replace '\n'

    if (VmStringChrA(pszValue, VM_REGCONFIG_NEWLINE))
    {
        dwError = _VmRegConfigWriteMultiSZBuf(pWrite, pszValue);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else
    {
        dwError = _VmRegConfigWriteBuf(pWrite, pszValue);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

error:
    return dwError;
}

static
DWORD
_VmRegConfigWriteKV(
    PVM_REGCONFIG_LIST_KV   pKV,
    PVM_WRITE_KV            pWrite
    )
{
    DWORD   dwError = 0;
    DWORD   dwCnt = 0;
    DWORD   dwStrCnt = 0;
    PVM_STRING_LIST pStrList = NULL;

    dwError = VmStringToTokenList(
            pKV->pszKey,
            VM_REGCONFIG_KEY_SEPARATOR_STR,
            &pStrList);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwStrCnt = pStrList->dwCount;

    // handle key output
    for (dwCnt = 0; dwCnt < dwStrCnt; dwCnt++)
    {
        pWrite->bPrefixNL = FALSE;

        if (dwCnt < pWrite->pSubKeyList->dwCount)
        {
            if (VmStringCompareA(
                    pStrList->pStringList[dwCnt],
                    pWrite->pSubKeyList->pStringList[dwCnt],
                    FALSE) != 0)
            {
                while (dwCnt < pWrite->pSubKeyList->dwCount)
                {
                    dwError = VmStringListRemoveLast(pWrite->pSubKeyList);
                    BAIL_ON_VM_COMMON_ERROR(dwError);
                }

                dwError = VmStringListAddStrClone(pStrList->pStringList[dwCnt], pWrite->pSubKeyList);
                BAIL_ON_VM_COMMON_ERROR(dwError);

                pWrite->bPrefixNL = TRUE;
                dwError = _VmRegConfigWriteMeta(pWrite);
                BAIL_ON_VM_COMMON_ERROR(dwError);
            }
        }
        else
        {
            dwError = VmStringListAddStrClone(pStrList->pStringList[dwCnt], pWrite->pSubKeyList);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            if (dwCnt < dwStrCnt -1)
            {
                pWrite->bPrefixNL = TRUE;
            }

            dwError = _VmRegConfigWriteMeta(pWrite);
            BAIL_ON_VM_COMMON_ERROR(dwError);
        }
    }

    // handle value output
    dwError = _VmRegConfigDQuoteValue(pKV, pWrite);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = _VmRegConfigWriteValue(pKV, pWrite);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmStringListRemoveLast(pWrite->pSubKeyList);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    VmStringListFree(pStrList);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmRegConfigWriteFileInternal(
    PVM_REGCONFIG_LIST_ENTRY   pEntry
    )
{
    DWORD           dwError = 0;
    PSTR            pszTmpFile = NULL;
    PVM_REGCONFIG_LIST_KV   pKV = NULL;
    VM_WRITE_KV     writeKV = {0};
    mode_t          priorMode = 0;
    uid_t           processUid = getuid();

    // allow config file read/write by lightwave user
    priorMode = umask(S_IWGRP|S_IRGRP|S_IWOTH|S_IROTH);

    writeKV.iBufSize = VM_SIZE_2048;
    dwError = VmAllocateMemory(writeKV.iBufSize, (PVOID)&writeKV.pszBuf);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    writeKV.iQuoteBufSize = VM_SIZE_2048;
    dwError = VmAllocateMemory(writeKV.iQuoteBufSize, (PVOID)&writeKV.pszValueQuoteBuf);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmStringListInitialize(&writeKV.pSubKeyList, 5);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringPrintf(&pszTmpFile, "%s.%lu", pEntry->pszFileName, (DWORD)getpid());
    BAIL_ON_VM_COMMON_ERROR(dwError);

    writeKV.fh = fopen(pszTmpFile, "w");
    if (!writeKV.fh)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

    for (pKV = pEntry->pListKV; pKV; pKV = pKV->pNext)
    {
        dwError = _VmRegConfigWriteKV(pKV, &writeKV);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (fwrite(writeKV.pszBuf, 1, writeKV.iBufLen, writeKV.fh) != writeKV.iBufLen)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

    fclose(writeKV.fh);
    writeKV.fh = NULL;

    if (processUid == 0)
    {
        dwError = VmSetLWOwnership(pszTmpFile);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (rename(pszTmpFile, pEntry->pszFileName) != 0)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

cleanup:
    VM_COMMON_SAFE_FREE_MEMORY(pszTmpFile);
    VM_COMMON_SAFE_FREE_MEMORY(writeKV.pszBuf);
    VM_COMMON_SAFE_FREE_MEMORY(writeKV.pszValueQuoteBuf);
    VmStringListFree(writeKV.pSubKeyList);
    if (writeKV.fh)
    {
        fclose(writeKV.fh);
    }
    umask(priorMode);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmRegConfigSetKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName,
    PCSTR                       pszValue,
    size_t                      iValueSize
    )
{
    DWORD   dwError = 0;
    PVM_REGCONFIG_LIST_KV   pKV = NULL;
    PVM_REGCONFIG_LIST_KV   pTargetKV = NULL;
    PVM_REGCONFIG_LIST_KV   pPrevKV = NULL;
    PVM_REGCONFIG_LIST_KV   pNewKV = NULL;
    PCSTR                   pszLocalKey = pszKeyName + VM_REGCONFIG_TOP_KEY_PATH_LEN;
    PSTR                    pszLocalKeyLastSep = VmStringRChrA(pszLocalKey, VM_REGCONFIG_KEY_SEPARATOR);

    for (pKV = pEntry->pListKV; pKV; pPrevKV = pKV, pKV = pKV->pNext)
    {
        PSTR    pszLastSep = NULL;

        if (VmStringCompareA(
                pKV->pszKey,
                pszLocalKey,
                FALSE) == 0)
        {
            VM_COMMON_SAFE_FREE_MEMORY(pKV->pszValue);

            dwError = VmAllocateMemory(iValueSize+1, (PVOID*)&pKV->pszValue);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = VmCopyMemory(pKV->pszValue, iValueSize+1, pszValue, iValueSize);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            pKV->iValueSize = iValueSize;

            goto done;
        }

        // find the position to insert this new KV into list
        pszLastSep = VmStringRChrA(pKV->pszKey, VM_REGCONFIG_KEY_SEPARATOR);
        if (pszLocalKeyLastSep - pszLocalKey == pszLastSep - pKV->pszKey)
        {
            pTargetKV = pKV;
        }
    }

    if (!pTargetKV)
    {
        pTargetKV = pPrevKV;
    }

    dwError = VmAllocateMemory(
            sizeof(VM_REGCONFIG_LIST_KV),
            (PVOID*)&pNewKV);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringA(pszLocalKey, &pNewKV->pszKey);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringA(pszValue, &pNewKV->pszValue);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pNewKV->iValueSize = iValueSize;

    pNewKV->pNext = pTargetKV->pNext;
    pTargetKV->pNext = pNewKV;
    pNewKV = NULL;

done:
    dwError = VmRegConfigWriteFileInternal(pEntry);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    VmRegConfigKVFree(pNewKV);
    goto cleanup;
}

DWORD
VmRegConfigDeleteKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName
    )
{
    DWORD   dwError = 0;
    PVM_REGCONFIG_LIST_KV   pKV = NULL;
    PVM_REGCONFIG_LIST_KV   pPrevKV = NULL;

    for (pKV = pEntry->pListKV; pKV; pPrevKV = pKV, pKV = pKV->pNext)
    {
        if (VmStringCompareA(
                pKV->pszKey,
                pszKeyName + VM_REGCONFIG_TOP_KEY_PATH_LEN,
                FALSE) == 0)
        {
            break;
        }
    }

    if (!pKV)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND);
    }

    if (!pPrevKV)
    {
        pEntry->pListKV = pKV->pNext;
    }
    else
    {
        pPrevKV->pNext = pKV->pNext;
    }

    VmRegConfigKVFree(pKV);

    dwError = VmRegConfigWriteFileInternal(pEntry);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmRegConfigLockFile(
    PCSTR   pszLockFileName,
    int*    pfd
    )
{
    DWORD   dwError = 0;
    int     fd = -1;

    if (!pszLockFileName || !pfd)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    if ((fd = open(pszLockFileName, O_CREAT, S_IRUSR| S_IWUSR)) == -1)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

    if (getuid() == 0)
    {   // root user, change lock file ownership to lightwave
        dwError = VmSetLWOwnership(pszLockFileName);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

    if (flock(fd, LOCK_EX) == -1)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

    *pfd = fd;

error:
    return dwError;
}
