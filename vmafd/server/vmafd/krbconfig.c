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

#ifndef _WIN32

static
DWORD
_VmAfdCreateFile(
    PCSTR pszFileName,
    FILE **pFile)
{
    DWORD dwError = 0;
    FILE *fp = NULL;
    int fd = 0;

    fd = open(pszFileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
    {
        dwError = ERROR_OPERATION_NOT_PERMITTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    fp = fdopen(fd, "w");
    if (!fp)
    {
        dwError = ERROR_FILE_NOT_FOUND;
        close(fd);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *pFile = fp;

error:

    return dwError;
}

#else

static
DWORD
_VmAfdCreateFile(
    PCSTR pszFileName,
    FILE **pFile)
{
    DWORD dwError = 0;
    FILE *fp = NULL;

    dwError = VmAfdOpenFilePath(pszFileName, "w", &fp);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pFile = fp;

error:

    return dwError;
}

#endif

DWORD
VmAfdInitKrbConfig(
    PCSTR pszFileName,
    PVMAFD_KRB_CONFIG *ppKrbConfig)
{
    DWORD dwError = 0;
    PVMAFD_KRB_CONFIG pKrbConfig = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszFileName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppKrbConfig, dwError);

    dwError = VmAfdAllocateMemory(sizeof(*pKrbConfig),
                                  (PVOID*)&pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringA(
                      pszFileName,
                      &pKrbConfig->pszFileName);
    BAIL_ON_VMAFD_ERROR(dwError);

    pKrbConfig->pszDefaultRealm = NULL;
    pKrbConfig->pszDefaultKeytabName = NULL;
    pKrbConfig->iNumKdcServers = 0;

    *ppKrbConfig = pKrbConfig;

error:

    if (dwError)
    {
        if (pKrbConfig)
        {
            VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszFileName);
        }
        VMAFD_SAFE_FREE_MEMORY(pKrbConfig);
    }

    return dwError;
}

DWORD
VmAfdDestroyKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig)
{
    DWORD dwError = 0;
    DWORD i = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);

    VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszFileName);
    VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszDefaultRealm);
    VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszDefaultKeytabName);

    for (i=0; i<pKrbConfig->iNumKdcServers; i++)
    {
        VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszKdcServer[i]);
    }

    VMAFD_SAFE_FREE_MEMORY(pKrbConfig);

error:

    return dwError;
}

DWORD
VmAfdBackupKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig)
{
    DWORD dwError = 0;
    PSTR pszFileName = NULL;
    PSTR pszBackupFileName = NULL;
    int iRetVal = 0;
    BOOLEAN bFound = FALSE;
    BOOLEAN bBackupFound = FALSE;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszFileName, dwError);

    pszFileName = pKrbConfig->pszFileName;

    dwError = VmAfdFileExists(pszFileName, &bFound);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bFound)
    {
        dwError = VmAfdAllocateStringPrintf(
                          &pszBackupFileName,
                          "%s.bak",
                          pszFileName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdFileExists(pszBackupFileName, &bBackupFound);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!bBackupFound)
        {
            iRetVal = rename(pszFileName, pszBackupFileName);
            if (iRetVal == -1)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszBackupFileName);

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfdAddKdcKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszKdc)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszKdc, dwError);

    if (pKrbConfig->iNumKdcServers >= VMAFD_MAX_KDC_SERVERS)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(
                      pszKdc,
                      &pKrbConfig->pszKdcServer[pKrbConfig->iNumKdcServers]);
    BAIL_ON_VMAFD_ERROR(dwError);

    pKrbConfig->iNumKdcServers++;

error:

    return dwError;
}

DWORD
VmAfdSetDefaultRealmKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszDefaultRealm)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszDefaultRealm, dwError);

    VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszDefaultRealm);

    dwError = VmAfdAllocateStringA(
                      pszDefaultRealm,
                      &pKrbConfig->pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdSetDefaultKeytabNameKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig,
    PCSTR pszDefaultKeytabName)
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszDefaultKeytabName, dwError);

    VMAFD_SAFE_FREE_STRINGA(pKrbConfig->pszDefaultKeytabName);

    dwError = VmAfdAllocateStringA(
                      pszDefaultKeytabName,
                      &pKrbConfig->pszDefaultKeytabName);
    BAIL_ON_VMAFD_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmAfdWriteKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig)
{
    DWORD dwError = 0;
    DWORD i = 0;
    FILE *fp = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszFileName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszDefaultRealm, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszDefaultKeytabName, dwError);

    dwError = _VmAfdCreateFile(pKrbConfig->pszFileName, &fp);
    BAIL_ON_VMAFD_ERROR(dwError);

    fprintf(fp, "[libdefaults]\n");
    fprintf(fp, "\tdefault_realm = %s\n",
                pKrbConfig->pszDefaultRealm);
    fprintf(fp, "\tdefault_keytab_name = %s\n",
                pKrbConfig->pszDefaultKeytabName);
    fprintf(fp, "\n");
    fprintf(fp, "[realms]\n");
    fprintf(fp, "\t%s = {\n", pKrbConfig->pszDefaultRealm);
    for (i=0; i<pKrbConfig->iNumKdcServers; i++)
    {
        fprintf(fp, "\t\tkdc = %s\n",
                    pKrbConfig->pszKdcServer[i]);
    }
    fprintf(fp, "\t}\n");

error:

    if (fp)
    {
        fclose(fp);
    }

    return dwError;
}
