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

/*
 * read one schema element definition from file and normalize its definition.
 */
static
DWORD
_VmDirReadOneDefFromFile(
    FILE*              fp,
    PVMDIR_STRING_LIST pStrList
    )
{
    DWORD   dwError = 0;
    size_t  iSize = VMDIR_SIZE_9216, iLen = 0;
    CHAR    pDescBuf[VMDIR_SIZE_9216+1] = {0};
    CHAR    pbuf[VMDIR_SIZE_4096] = {0};
    PCSTR   pPrefix = "( ";
    size_t  iPrefixLen = VmDirStringLenA(pPrefix);
    PSTR    pOut = NULL;

    dwError = VmDirStringNCatA(
            pDescBuf+iLen, iSize-iLen, pPrefix, iPrefixLen);
    BAIL_ON_VMDIR_ERROR(dwError);
    iLen += iPrefixLen;

    while (fgets(pbuf, sizeof(pbuf), fp) != NULL)
    {
        size_t len = VmDirStringLenA(pbuf) - 1;
        if (pbuf[len] == '\n')
        {
            pbuf[len] = '\0';
        }

        if ( pbuf[0] == '#')
        {
            continue;
        }

        if ( pbuf[0] == ' ')
        {
            dwError = VmDirStringNCatA(
                    pDescBuf+iLen, iSize-iLen, pbuf, VmDirStringLenA(pbuf));
            BAIL_ON_VMDIR_ERROR(dwError);
            iLen += VmDirStringLenA(pbuf);
        }
        else
        {
            VmdDirNormalizeString(pDescBuf);
            dwError = VmDirAllocateStringA(pDescBuf, &pOut);
            BAIL_ON_VMDIR_ERROR(dwError);

            dwError = VmDirStringListAdd(pStrList, pOut);
            BAIL_ON_VMDIR_ERROR(dwError);
            pOut = NULL;
            break;
        }
    }

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pOut);
    goto cleanup;
}

DWORD
VmDirGetDefaultSchemaFile(
    PSTR*   ppszSchemaFile
    )
{
    DWORD   dwError = 0;
    PSTR    pszSchemaFile = NULL;
#ifdef _WIN32
    PSTR    pszCfgPath = NULL;
#else
    PCSTR   pszLinuxFile = LWRAFT_CONFIG_DIR "/lwraftschema.ldif";
#endif

    if ( ppszSchemaFile==NULL)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

#ifdef _WIN32
    dwError = VmDirGetCfgPath(&pszCfgPath);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pszSchemaFile,"%s\\lwraftschema.ldif", pszCfgPath);
    BAIL_ON_VMDIR_ERROR(dwError);
#else
    dwError = VmDirAllocateStringA(pszLinuxFile, &pszSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);
#endif

    *ppszSchemaFile = pszSchemaFile;

cleanup:
    return dwError;
error:
#ifdef _WIN32
    VMDIR_SAFE_FREE_MEMORY(pszCfgPath);
#endif
    VMDIR_SAFE_FREE_MEMORY(pszSchemaFile);
    goto cleanup;
}

/*
 * Read schema definition from file
 * We only care for
 *  attributetypes
 *  objectclasses
 *  ditcontentrules
 */
DWORD
VmDirReadSchemaFile(
    PCSTR               pszSchemaFilePath,
    PVMDIR_STRING_LIST* ppAtStrList,
    PVMDIR_STRING_LIST* ppOcStrList,
    PVMDIR_STRING_LIST* ppCrStrList
    )
{
    DWORD dwError = 0;
    CHAR  pbuf[1024] = {0};
    FILE* fp = NULL;

    PVMDIR_STRING_LIST  pAtStrList = NULL;
    PVMDIR_STRING_LIST  pOcStrList = NULL;
    PVMDIR_STRING_LIST  pCrStrList = NULL;

    if (!pszSchemaFilePath || !ppAtStrList || !ppOcStrList || !ppCrStrList)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirStringListInitialize(&pAtStrList, 2048);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pOcStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringListInitialize(&pCrStrList, 512);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef _WIN32
    fp = fopen(pszSchemaFilePath, "r");
#else
    if (fopen_s(&fp, pszSchemaFilePath, "r") != 0)
    {
        fp = NULL;
    }
#endif
    if (NULL == fp)
    {
        dwError = errno;
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "Open schema file (%s) failed. Error (%d)",
                pszSchemaFilePath, dwError);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    while (fgets(pbuf, sizeof(pbuf), fp) != NULL)
    {
        PSTR pszTag = NULL;

        if ((pbuf[0] == '\n')    ||
            (pbuf[0] == '#')     ||
            (pbuf[0] != ' ' && (pszTag = VmDirStringChrA(pbuf, ':')) == NULL))
        {
            continue;
        }

        if (IS_ATTRIBUTETYPES_TAG(pbuf))
        {
            dwError = _VmDirReadOneDefFromFile(fp, pAtStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_OBJECTCLASSES_TAG(pbuf))
        {
            dwError = _VmDirReadOneDefFromFile(fp, pOcStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (IS_CONTENTRULES_TAG(pbuf))
        {
            dwError = _VmDirReadOneDefFromFile(fp, pCrStrList);
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else
        {
            continue;
        }
    }

    *ppAtStrList = pAtStrList;
    *ppOcStrList = pOcStrList;
    *ppCrStrList = pCrStrList;

cleanup:
    if (fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    VmDirStringListFree(pAtStrList);
    VmDirStringListFree(pOcStrList);
    VmDirStringListFree(pCrStrList);
    goto cleanup;
}
