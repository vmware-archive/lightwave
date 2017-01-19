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

    dwError = VmAfdOpenFilePath(pszFileName, "w", &fp, 0);
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

static
DWORD
_VmAfdFileCopyBackup(
    PSTR pszFileName,
    PSTR pszBackupFileName)
{
    DWORD dwError = 0;
    DWORD iRead = 0;
    DWORD iWrite = 0;
    FILE *pInFile = NULL;
    FILE *pOutFile = NULL;
    PSTR pszBuf = NULL;

    dwError = _VmAfdCreateFile(pszBackupFileName,
                               &pOutFile);
    BAIL_ON_VMAFD_ERROR(dwError);

    pInFile = fopen(pszFileName, "r");
    if (!pInFile)
    {
        dwError = VMAFD_ERRNO;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* Allocate a 1K buffer for copying file */
    dwError = VmAfdAllocateMemory(sizeof(*pszBuf) * VMAFD_FILE_COPY_BUFSZ,
                                  (PVOID*)&pszBuf);
    BAIL_ON_VMAFD_ERROR(dwError);

    while (!feof(pInFile))
    {
        iRead = fread(pszBuf, sizeof(*pszBuf), VMAFD_FILE_COPY_BUFSZ, pInFile);
        if (iRead == -1)
        {
            dwError = ERROR_READ_FAULT;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (iRead > 0)
        {
            iWrite = fwrite(pszBuf, sizeof(*pszBuf), iRead, pOutFile);
            if (iWrite == -1 || iWrite != iRead)
            {
                dwError = ERROR_WRITE_FAULT;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
    }

error:
    if (pInFile)
    {
        fclose(pInFile);
    }
    if (pOutFile)
    {
        fclose(pOutFile);
    }
    VMAFD_SAFE_FREE_MEMORY(pszBuf);

    return dwError;
}

/*
 * Make copy backup of the configuration file. Two files are created:
 * 1) file.conf.orig
 * 2) file.conf.bak
 *
 * 1) is a one-time original copy of the file.conf, and is never overwritten
 * 2) is a copy of the current file.conf, which is changed each time
 */

DWORD
VmAfdBackupCopyKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig)
{
    DWORD dwError = 0;
    PSTR pszFileName = NULL;
    PSTR pszBackupFileName = NULL;
    PSTR pszBackupOrigFileName = NULL;
    BOOLEAN bFound = FALSE;
    BOOLEAN bBackupFound = FALSE;
    BOOLEAN bBackupOrigFound = FALSE;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszFileName, dwError);

    pszFileName = pKrbConfig->pszFileName;


    dwError = VmAfdFileExists(pszFileName, &bFound);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (bFound)
    {
        dwError = VmAfdAllocateStringPrintf(
                          &pszBackupOrigFileName,
                          "%s.orig",
                          pszFileName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdFileExists(pszBackupOrigFileName, &bBackupOrigFound);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (!bBackupOrigFound)
        {
            /*
             * 1) Create an original copy of the krb5.conf file.
             */
            dwError = _VmAfdFileCopyBackup(pszFileName, pszBackupOrigFileName);
            BAIL_ON_VMAFD_ERROR(dwError);
        }

        dwError = VmAfdAllocateStringPrintf(
                          &pszBackupFileName,
                          "%s.bak",
                          pszFileName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdFileExists(pszBackupFileName, &bBackupFound);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (bBackupFound)
        {
            /*
             * 2) Create a .bak backup copy file the current state of krb5.conf
             */
            remove(pszBackupFileName);
        }
        dwError = _VmAfdFileCopyBackup(pszFileName, pszBackupFileName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszBackupFileName);
    VMAFD_SAFE_FREE_STRINGA(pszBackupOrigFileName);

    return dwError;

error:

    goto cleanup;
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

#ifdef USE_DEFAULT_KRB5_PATHS
/*
 * Merge vmdir Kerberos entries into the default krb5.conf file
 */
DWORD
VmAfdWriteKrbConfig(
    PVMAFD_KRB_CONFIG pKrbConfig)
{
    DWORD dwError = 0;
    DWORD i = 0;
    BOOLEAN bKrbConfFound = FALSE;
    BOOLEAN bDefaultRealmFound = TRUE;
    BOOLEAN bDefaultKeytabFound = TRUE;
    FILE *pFile = NULL;
    int retval = 0;
    const char *section[4] = {0};
    profile_t profile = {0};
    PSTR pszRetString = NULL;
    PSTR *ppszRetRealm = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszFileName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pKrbConfig->pszDefaultRealm, dwError);

    retval = profile_init_path(pKrbConfig->pszFileName, &profile);
    if (retval)
    {
        if (retval == ENOENT)
        {
            bKrbConfFound = FALSE;
            dwError = _VmAfdCreateFile(pKrbConfig->pszFileName, &pFile);
            BAIL_ON_VMAFD_ERROR(dwError);
            fclose(pFile);
            retval = profile_init_path(pKrbConfig->pszFileName, &profile);
            if (retval)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR(dwError);
            }
        }
        else
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    else
    {
        bKrbConfFound = TRUE;
    }

    if (!bKrbConfFound)
    {
        bDefaultRealmFound = FALSE;
        bDefaultKeytabFound = FALSE;
    }
    else
    {
        /*
         * Only add [libdefaults]/default_realm and default_keytab_name
         * when these values do not already exist.
         */
        retval = profile_get_string(profile, "libdefaults", "default_realm", NULL, NULL, &pszRetString);
        if (retval == 0 && (!pszRetString || !pszRetString[0]))
        {
            bDefaultRealmFound = FALSE;
        }
        retval = profile_get_string(profile, "libdefaults", "default_keytab_name", NULL, NULL, &pszRetString);
        if (retval == 0 && (!pszRetString || !pszRetString[0]))
        {
            bDefaultKeytabFound = FALSE;
        }

    }

    if (!bDefaultRealmFound)
    {
        section[0] = "libdefaults";
        section[1] = "default_realm";
        section[2] = NULL;
        retval = profile_add_relation(profile,
                                      section,
                                      pKrbConfig->pszDefaultRealm);
        if (retval)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    if (!bDefaultKeytabFound)
    {
        section[0] = "libdefaults";
        section[1] = "default_keytab_name";
        section[2] = NULL;
        retval = profile_add_relation(profile,
                                      section,
                                      pKrbConfig->pszDefaultKeytabName);
        if (retval)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    /*
     * Following code writes an entry of the following format:
     * [realms]
     *     REALM.COM = {
     *         kdc = value1
     *         kdc = valueN
     *     }
     * First, remove any existing relation for the same REALM,
     * in case this realm was previously joined. Rejoining must
     * remove previous cruft, otherwise the wrong KDC will be referenced.
     */
    section[0] = "realms";
    section[1] = pKrbConfig->pszDefaultRealm;
    section[2] = "kdc";
    section[3] = NULL;
    profile_clear_relation(profile, section);

    for (i=0; i<pKrbConfig->iNumKdcServers; i++)
    {
        retval = profile_add_relation(profile,
                                      section,
                                      pKrbConfig->pszKdcServer[i]);
        if (retval)
        {
            dwError = ERROR_INVALID_PARAMETER;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

error:
    if (profile)
    {
        profile_flush(profile);
        profile_release(profile);
    }
    if (ppszRetRealm)
    {
        profile_free_list(ppszRetRealm);
    }

    return dwError;
}

#else

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

#endif
