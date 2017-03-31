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
 * Module Name: vdcpromo
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcpromo main module entry point
 *
 */

#include "includes.h"

#ifdef _WIN32
static
DWORD
VmAfdConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PSTR*    ppszValue      /*    OUT */
    );
#endif

int main(int argc, char* argv[])
{
    DWORD   dwError = 0;
    int    retCode = 0;
    PSTR   pszDomain = NULL;
    PSTR   pszDomainName = NULL;
    PSTR   pszUserName = NULL;
    PSTR   pszPassword = NULL;
    PSTR   pszPwdFile = NULL;
    PSTR   pszSiteName = NULL;
    PSTR   pszPartnerHostName = NULL;
    PSTR   pszLotusServerName = NULL;
    PSTR   pszPasswordBuf = NULL;
    FILE * fpPwdFile = NULL;
    PSTR   pszPath = NULL;
    PSTR   pszLogPathName = NULL;
    size_t dPwdLen = 0;
    BOOL   bLogInitialized = FALSE;
    PCSTR  pszErrorMsg = NULL;
    PSTR   pszErrorDesc = NULL;
    PCSTR  pszLocalhost = "localhost";
    PSTR   pszPnid = NULL;
    DNS_INIT_FLAG dnsInitFlag = DNS_NONE;
#ifdef _WIN32
    WSADATA wsaData = { 0 };
    BOOLEAN bWsaStartup = FALSE;
#endif

#ifndef _WIN32
    setlocale(LC_ALL, "");
#else
    dwError = WSAStartup(MAKEWORD(2, 2), &wsaData);
    BAIL_ON_VMAFD_ERROR(dwError);
    bWsaStartup = TRUE;
#endif

    dwError = VmAfCfgInit();
    BAIL_ON_VMAFD_ERROR(dwError);

#ifndef _WIN32
    dwError = VmAfdAllocateStringA(VMAFD_LOG_PATH, &pszLogPathName );
    BAIL_ON_VMAFD_ERROR(dwError);
#else
    dwError = VmAfdConfigGetString(VMAFD_SOFTWARE_KEY_PATH, VMAFD_LOGPATH_KEY_VALUE, &pszLogPathName);
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfdAllocateStringAVsnprintf( &pszPath, "%s%s", pszLogPathName, "vdcpromo.log"  );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdLogInitialize(pszPath, 0, 0);
    BAIL_ON_VMAFD_ERROR(dwError);
    bLogInitialized = TRUE;

    dwError = VmAfdParseArgs(argc,
                             argv,
                             &pszDomain,
                             &pszUserName,
                             &pszPassword,
                             &pszSiteName,
                             &pszPartnerHostName,
                             &pszLotusServerName,
                             &pszPwdFile,
                             &dnsInitFlag);
    if (dwError)
    {
        ShowUsage();
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszUserName))
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Username parameter is not valid");
    }
    else
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Username: \"%s\"", pszUserName);
    }

    if (!pszPwdFile && IsNullOrEmptyString(pszPassword))
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Password parameter is not valid");
    }

    if (IsNullOrEmptyString(pszDomain))
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Domain parameter is not valid");
    }
    else
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Domain: \"%s\"", pszDomain);
    }

    dwError = VmAfdAllocateMemory(VMAFD_MAX_PWD_LEN+1, (PVOID *)&pszPasswordBuf);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszPassword == NULL && pszPwdFile != NULL)
    {
        dwError = VmAfdOpenFilePath(pszPwdFile, "rb", &fpPwdFile, 0);
        if (dwError != ERROR_SUCCESS)
        {
           printf("vdcpromo: cannot open password file %s (%u)\n", pszPwdFile, dwError);
           dwError = ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN;
           BAIL_ON_VMAFD_ERROR(dwError);
        }
        if ( (dPwdLen = fread(pszPasswordBuf, 1, VMAFD_MAX_PWD_LEN, fpPwdFile)) == 0)
        {
           dwError = ERROR_LOCAL_PASSWORDFILE_CANNOT_READ;
           printf("vdcpromo: Could not read password file\n");
           BAIL_ON_VMAFD_ERROR(dwError);
        }
        if (*pszPasswordBuf == '\0')
        {
           VmAfdLog(VMAFD_DEBUG_ANY, "password is empty");
           dwError = ERROR_LOCAL_PASSWORD_EMPTY;
           BAIL_ON_VMAFD_ERROR(dwError);
        }
    }
    else if (pszPassword != NULL && pszPwdFile == NULL)
    {
       dwError = VmAfdStringCpyA(pszPasswordBuf, VMAFD_MAX_PWD_LEN, pszPassword);
       BAIL_ON_VMAFD_ERROR(dwError);
    } else //no password nor password-file, read password from stdin
    {
       VmAfdReadString("password: ", pszPasswordBuf, VMAFD_MAX_PWD_LEN+1, FALSE);
    }

    if (IsNullOrEmptyString(pszDomain))
    {
        dwError = VmAfdGetDomainNameA(NULL, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        pszDomain = pszDomainName;
    }

    if (dnsInitFlag == DNS_INIT)
    {
        dwError = VmAfdConfigureDNSA(pszUserName, pszPasswordBuf);
        if (dwError)
        {
            fprintf(stderr, "Warning: failed to initialize DNS. Error [%d]\n", dwError);
        }
        else
        {
            fprintf(stdout, "Successfully initialized DNS.\n");
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        goto cleanup;
    }

    printf("Initializing Directory server instance ... \n");
    fflush(stdout);

    dwError = VmAfdPromoteVmDirA(
                        pszLotusServerName ? pszLotusServerName : pszLocalhost,
                        pszDomain,
                        pszUserName,
                        pszPasswordBuf,
                        pszSiteName,
                        pszPartnerHostName);
    BAIL_ON_VMAFD_ERROR(dwError);

    printf("Directory host instance created successfully\n");

cleanup:

    VmAfCfgShutdown();

    if (bLogInitialized)
    {
        VmAfdLogTerminate();
    }

#ifdef _WIN32
    if (bWsaStartup != FALSE)
    {
        WSACleanup();
    }
#endif
    VMAFD_SAFE_FREE_MEMORY(pszPnid);
    VMAFD_SAFE_FREE_MEMORY(pszPasswordBuf);
    VMAFD_SAFE_FREE_MEMORY(pszPath);
    VMAFD_SAFE_FREE_MEMORY(pszLogPathName);
    VMAFD_SAFE_FREE_MEMORY(pszErrorDesc);
    VMAFD_SAFE_FREE_MEMORY(pszDomainName);
    return retCode;

error:

    switch (dwError)
    {
        case ERROR_LOCAL_OPTION_UNKNOWN:
            retCode = 2;
            pszErrorMsg = "An unknown option was present on the command line.";
            break;
        case ERROR_LOCAL_OPTION_INVALID:
            retCode = 3;
            pszErrorMsg = "The options present on the command line are not valid.";
            break;
        case ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN:
            retCode = 4;
            pszErrorMsg = "Could not open password file.\nVerify the path is correct.";
            break;
        case ERROR_LOCAL_PASSWORDFILE_CANNOT_READ:
            retCode = 5;
            pszErrorMsg = "Problem reading password file.\nVerify contents of password file.";
            break;
        case ERROR_LOCAL_PASSWORD_EMPTY:
            retCode = 6;
            pszErrorMsg = "Invalid password; password cannot be empty.";
            break;
        case ERROR_CANNOT_CONNECT_VMAFD:
            retCode = 20;
            pszErrorMsg = "Could not connect to the local service VMware AFD.\nVerify VMware AFD is running.";
            break;
        case VMDIR_ERROR_CANNOT_CONNECT_VMDIR:
            retCode = 21;
            pszErrorMsg = "Could not connect to the local service VMware Directory Service.\nVerify VMware Directory Service is running.";
            break;
        case ERROR_INVALID_CONFIGURATION:
            retCode = 22;
            pszErrorMsg = "Configuration is not correct.\nFirst boot scripts need to be executed.";
            break;
        case VMDIR_ERROR_SERVER_DOWN:
            retCode = 23;
            pszErrorMsg = "Could not connect to VMware Directory Service via LDAP.\nVerify VMware Directory Service is running on the appropriate system and is reachable from this host.";
            break;
        case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
            retCode = 24;
            pszErrorMsg = "Authentication to VMware Directory Service failed.\nVerify the username and password.";
            break;
        case ERROR_ACCESS_DENIED:
            retCode = 25;
            pszErrorMsg = "Authorization failed.\nVerify account has proper administrative privileges.";
            break;
        case VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE:
            retCode = 26;
            pszErrorMsg = "Could not join to the remote service VMWare Directory Service.\nThe remote schema is incompatible with the local schema.";
            break;
        case VMDIR_ERROR_INVALID_FUNC_LVL:
            retCode = 27;
            pszErrorMsg = "Could not join to the remote service VMWare Directory Service.\nThe local service cannot support the domain functional level";
            break;
        default:
            retCode = 1;
    }

    if (bLogInitialized)
    {
        if (pszErrorMsg == NULL)
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Vdcpromo failed. Error[%d]", dwError);
        }
        else
        {
            VmAfdLog(VMAFD_DEBUG_ANY, "Vdcpromo failed. Error[%d]\n%s", dwError, pszErrorMsg);
        }
    }

    if (pszErrorMsg == NULL)
    {
        if (!VmAfdGetErrorString(dwError, &pszErrorDesc))
        {
            printf("Vdcpromo failed. Error %d: %s\n", dwError, pszErrorDesc);
        }
        else
        {
            printf("Vdcpromo failed. Error %d.\n", dwError);
        }
    }
    else
    {
        printf("Vdcpromo failed. Error[%d]\n%s\n", dwError, pszErrorMsg);
    }

    goto cleanup;
}

#ifdef _WIN32

static
DWORD
VmAfdConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PSTR*    ppszValue      /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PSTR  pszValue = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppszValue, dwError);

    dwError = VmAfConfigOpenConnection(&pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenRootKey(
                    pConnection,
                    "HKEY_LOCAL_MACHINE",
                    0,
                    KEY_READ,
                    &pRootKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigOpenKey(
                    pConnection,
                    pRootKey,
                    pszSubKey,
                    0,
                    KEY_READ,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigReadStringValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszValue = pszValue;

cleanup:

    if (pParamsKey)
    {
        VmAfConfigCloseKey(pParamsKey);
    }
    if (pRootKey)
    {
        VmAfConfigCloseKey(pRootKey);
    }
    if (pConnection)
    {
        VmAfConfigCloseConnection(pConnection);
    }

    return dwError;

error:

    if (ppszValue)
    {
        *ppszValue = NULL;
    }

    goto cleanup;
}

#endif

