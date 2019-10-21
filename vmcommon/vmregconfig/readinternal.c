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
_VmRefConfigAddKV(
    PVM_REGCONFIG_LIST_ENTRY   pEntry,
    PVM_CONSTRUCT_KV           pConstructor
    )
{
    DWORD   dwError = 0;
    PVM_REGCONFIG_LIST_KV   pKV = NULL;
    PVM_REGCONFIG_LIST_KV   pLastKV = NULL;

    dwError = VmAllocateMemory(
            sizeof(VM_REGCONFIG_LIST_KV),
            (PVOID*)&pKV);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    dwError = VmAllocateStringA(pConstructor->pszSubKey, &pKV->pszKey);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pKV->pszValue = pConstructor->pszValue;
    pKV->iValueSize = VmStringLenA(pKV->pszValue);
    pConstructor->pszValue = NULL;

    if (!pEntry->pListKV)
    {
        pEntry->pListKV = pKV;
    }
    else
    {
        for (pLastKV = pEntry->pListKV; pLastKV->pNext; pLastKV = pLastKV->pNext) {}
        pLastKV->pNext = pKV;
    }

cleanup:
    return dwError;

error:
    VmRegConfigKVFree(pKV);
    goto cleanup;
}

static
DWORD
_VmRefConfigAddSubKey(
    PCSTR                      pszScalar,
    PVM_CONSTRUCT_KV           pConstructor
    )
{
    DWORD   dwError = 0;
    DWORD   dwLen = 0;

    dwLen = VmStringLenA(pszScalar);

    pConstructor->pszSubKey[pConstructor->iSubKeySize++] = VM_REGCONFIG_KEY_SEPARATOR;
    dwError = VmStringNCatA(
            &pConstructor->pszSubKey[0],
            VM_SIZE_1024,
            pszScalar,
            dwLen);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    pConstructor->iSubKeySize += dwLen;

error:
    return dwError;
}

static
VOID
_VmRefConfigRemoveSubKey(
    PVM_CONSTRUCT_KV           pConstructor
    )
{
    int  iCnt = 0;

    for (iCnt = pConstructor->iSubKeySize -1; iCnt >= 0; iCnt--)
    {
        if (pConstructor->pszSubKey[iCnt] != VM_REGCONFIG_KEY_SEPARATOR)
        {
            pConstructor->pszSubKey[iCnt] = '\0';
        }
        else
        {
            pConstructor->pszSubKey[iCnt] = '\0';
            break;
        }
    }

    pConstructor->iSubKeySize = iCnt;
}

static
DWORD
_VmRefConfigDoKVStat(
    PVM_REGCONFIG_LIST_ENTRY   pEntry,
    PCSTR                      pszScalar,
    PVM_CONSTRUCT_KV           pConstructor
    )
{
    DWORD   dwError = 0;

    if (pConstructor->kvStat == VM_KV_STAT_SCALAR)
    {
        assert(pszScalar);

        if (!pConstructor->bHasKey)
        {
            if (VmStringChrA(pszScalar, VM_REGCONFIG_KEY_SEPARATOR))
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError,VM_COMMON_ERROR_REGCONFIG_BAD_KEY);
            }

            if (!pEntry->pszTopKey)
            {
                dwError = VmAllocateStringPrintf(
                        &pEntry->pszTopKey,
                        "%s\\%s\\",
                        VM_REGCONFIG_TOP_KEY_PATH, pszScalar);
                BAIL_ON_VM_COMMON_ERROR(dwError);
            }

            dwError = _VmRefConfigAddSubKey(pszScalar, pConstructor);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            pConstructor->bHasKey = TRUE;
        }
        else
        {
            dwError = VmAllocateStringA(pszScalar, &pConstructor->pszValue);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            dwError = _VmRefConfigAddKV(pEntry, pConstructor);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            _VmRefConfigRemoveSubKey(pConstructor);

            pConstructor->bHasKey = FALSE;
        }
    }
    else if (pConstructor->kvStat == VM_KV_STAT_ADD_SUBKEY)
    {
        pConstructor->bHasKey = FALSE;

    }
    else if (pConstructor->kvStat == VM_KV_STAT_REMOVE_SUBKEY)
    {
        _VmRefConfigRemoveSubKey(pConstructor);
    }

error:
    return dwError;
}

static
DWORD
_VmRefConfigConstructKV(
    PVM_REGCONFIG_LIST_ENTRY   pEntry,
    VM_YAML_PARSE_STAT_TYPE    yamlStat,
    PCSTR                      pszScalar,
    PVM_CONSTRUCT_KV           pConstructor
    )
{
    DWORD   dwError = 0;

    if (yamlStat == VM_YAML_PARSE_STAT_SCALAR)
    {
        pConstructor->kvStat = VM_KV_STAT_SCALAR;
        dwError = _VmRefConfigDoKVStat(pEntry, pszScalar, pConstructor);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else if (yamlStat == VM_YAML_PARSE_STAT_START_MAPPING)
    {
        pConstructor->kvStat = VM_KV_STAT_ADD_SUBKEY;
        dwError = _VmRefConfigDoKVStat(pEntry, NULL, pConstructor);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }
    else if (yamlStat == VM_YAML_PARSE_STAT_END_MAPPING)
    {
        pConstructor->kvStat = VM_KV_STAT_REMOVE_SUBKEY;
        dwError = _VmRefConfigDoKVStat(pEntry, NULL, pConstructor);
        BAIL_ON_VM_COMMON_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmRegConfigReadFileInternal(
    PVM_REGCONFIG_LIST_ENTRY   pEntry
    )
{
    DWORD           dwError = 0;
    FILE*           fh = NULL;
    yaml_parser_t   yamlParser = {0};
    yaml_event_t    yamlEvent = {0};

    PVM_CONSTRUCT_KV pConstructor = NULL;

    dwError = VmAllocateMemory(
            sizeof(VM_CONSTRUCT_KV),
            (PVOID*)&pConstructor);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if ((fh = fopen(pEntry->pszFileName, "r")) == NULL)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_FILE_IO);
    }

    if (yaml_parser_initialize(&yamlParser) != 1)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_YAML_PARSE_INIT);
    }

    yaml_parser_set_input_file(&yamlParser, fh);

    do {

      if (yaml_parser_parse(&yamlParser, &yamlEvent) != 1)
      {
          BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_YAML_PARSE);
      }

      switch(yamlEvent.type)
      {
          case YAML_NO_EVENT:
          case YAML_STREAM_START_EVENT:
          case YAML_STREAM_END_EVENT:
          case YAML_DOCUMENT_START_EVENT:
          case YAML_DOCUMENT_END_EVENT:
          case YAML_SEQUENCE_START_EVENT:
          case YAML_SEQUENCE_END_EVENT:
          case YAML_ALIAS_EVENT:
              break;

          case YAML_MAPPING_START_EVENT:
              dwError = _VmRefConfigConstructKV(
                      pEntry,
                      VM_YAML_PARSE_STAT_START_MAPPING,
                      NULL,
                      pConstructor);
              BAIL_ON_VM_COMMON_ERROR(dwError);
              break;

          case YAML_MAPPING_END_EVENT:
              dwError = _VmRefConfigConstructKV(
                      pEntry,
                      VM_YAML_PARSE_STAT_END_MAPPING,
                      NULL,
                      pConstructor);
              BAIL_ON_VM_COMMON_ERROR(dwError);
              break;

          case YAML_SCALAR_EVENT:
              dwError = _VmRefConfigConstructKV(
                      pEntry,
                      VM_YAML_PARSE_STAT_SCALAR,
                      (PCSTR)yamlEvent.data.scalar.value,
                      pConstructor);
              BAIL_ON_VM_COMMON_ERROR(dwError);
              break;
      }

      if (yamlEvent.type != YAML_STREAM_END_EVENT)
      {
          yaml_event_delete(&yamlEvent);
      }

    } while(yamlEvent.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&yamlEvent);

cleanup:
    VmRegConfigConstructorKVFree(pConstructor);
    yaml_parser_delete(&yamlParser);

    if (fh)
    {
        fclose(fh);
    }

    return dwError;

error:
    goto cleanup;
}

DWORD
VmRegConfigForceReadInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry
    )
{
    DWORD                   dwError = 0;
    struct stat             statbuf = {0};
    PVM_REGCONFIG_LIST_KV   pCurrListKV = NULL;

    if (!pEntry)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);
    }

    dwError = stat(pEntry->pszFileName, &statbuf);
    BAIL_ON_VM_COMMON_ERROR(dwError);

    if (pEntry->fileStat.st_mtim.tv_sec  == statbuf.st_mtim.tv_sec   &&
        pEntry->fileStat.st_mtim.tv_nsec == statbuf.st_mtim.tv_nsec)
    {
        goto cleanup;
    }

    pCurrListKV = pEntry->pListKV;
    pEntry->pListKV = NULL;

    pEntry->fileStat.st_mtim.tv_sec  = statbuf.st_mtim.tv_sec;
    pEntry->fileStat.st_mtim.tv_nsec = statbuf.st_mtim.tv_nsec;

    dwError = VmRegConfigReadFileInternal(pEntry);
    BAIL_ON_VM_COMMON_ERROR(dwError);

cleanup:
    VmRegConfigListKVFree(pCurrListKV);

    return dwError;

error:
    pEntry->pListKV = pCurrListKV;
    pCurrListKV = NULL;
    goto cleanup;
}

DWORD
VmRegConfigGetKeyInternal(
    PVM_REGCONFIG_LIST_ENTRY    pEntry,
    PCSTR                       pszKeyName,
    PSTR                        pszValue,
    size_t*                     piValueSize
    )
{
    DWORD   dwError = 0;
    size_t  iBufSize = *piValueSize;
    PVM_REGCONFIG_LIST_KV   pKV = NULL;

    pKV = pEntry->pListKV;

    while (pKV)
    {
        if (VmStringCompareA(
                pKV->pszKey,
                pszKeyName + VM_REGCONFIG_TOP_KEY_PATH_LEN,
                FALSE) == 0)
        {
            if (pKV->iValueSize > iBufSize - 1)
            {
                BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_VALUE_TOO_BIG);
            }

            memset(pszValue, 0, iBufSize);
            dwError = VmCopyMemory(pszValue, iBufSize, pKV->pszValue, pKV->iValueSize);
            BAIL_ON_VM_COMMON_ERROR(dwError);

            *piValueSize = pKV->iValueSize;
            break;
        }

        pKV = pKV->pNext;
    }

    if (!pKV)
    {
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND);
    }

error:
    return dwError;
}
