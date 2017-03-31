/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
#include <vmca.h>

#ifdef _WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#ifdef _WIN32
#define STAT _stat
#define UNLINK _unlink
#define RENAME rename
#else
#define STAT stat
#define UNLINK unlink
#define RENAME rename
#endif

DWORD
VMCACreatePrivateKeyW(
    PWSTR pwszPassPhrase,
    size_t uiKeyLength,
    PVMCA_KEY* ppPrivateKey,
    PVMCA_KEY* ppPublicKey
)
// VMCAAllocatePrivateKey function creates private-public key pair  and retuns them to user
//
// Arguments :
//          pszPassPhrase   : Optional Pass Word to protect the Key
//          uiKeyLength     : Key Length - Valid values are between 1024 and 16384
//          ppPrivateKey    : PEM encoded Private Key String
//          ppPublicKey     : PEM encoded Public Key String.
//
// Returns :
//      Error Code(s)
//          ERROR_INVALID_PARAMETER
//
// Notes : This function makes some assumptions on the users
// behalf. One of them is that assumption on bit size. This is based on RSA's
// recommendation http://www.rsa.com/rsalabs/node.asp?id=2218 on
// Corporate Key lengths.
{
    DWORD dwError = 0;
    if ( (uiKeyLength < 1024 ) || (uiKeyLength > 16384) ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if( ppPrivateKey == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if( ppPublicKey == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;

        BAIL_ON_ERROR(dwError);
    }

    dwError =  VMCAAllocatePrivateKeyPrivate(
                    pwszPassPhrase,
                    uiKeyLength,
                    ppPrivateKey,
                    ppPublicKey);

    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
VMCACreatePrivateKeyA(
    PSTR pszPassPhrase,
    size_t uiKeyLength,
    PVMCA_KEY* ppPrivateKey,
    PVMCA_KEY* ppPublicKey
)
{
    PWSTR pwszPassPhrase = NULL;
    DWORD dwError = 0;

    if (pszPassPhrase != NULL)
    {
        dwError = VMCAAllocateStringWFromA(
                      pszPassPhrase,
                      &pwszPassPhrase);
        BAIL_ON_ERROR(dwError);
    }

    dwError = VMCACreatePrivateKeyW(
                  pwszPassPhrase,
                  uiKeyLength,
                  ppPrivateKey,
                  ppPublicKey);
    BAIL_ON_ERROR(dwError);

error:
    VMCA_SAFE_FREE_STRINGW(pwszPassPhrase);

    return dwError;
}

VOID
VMCAFreeKey(
    PVMCA_KEY pKey
)
// VMCAFreeKey frees the Key Objects allocated by the VMCAAllocatePrivateKey
//
// Arguments :
//      pKey : Points to the key that is to be freed.
// Returns :
//      Error Code
//
{
    if (pKey) {
        VMCA_SAFE_FREE_STRINGA(pKey);
    }
}

VOID
VMCAFreeCSR(
    PVMCA_CSR pCSR
)
// VMCAFreeCSR frees the Key Objects allocated by the VMCACreateSigningRequest
//
// Arguments :
//      pCSR : Points to the CSR that is to be freed.
// Returns :
//      Error Code
{
    if (pCSR) {
        VMCA_SAFE_FREE_STRINGA(pCSR);
    }
}

VOID
VMCAFreeCertificate(
    PVMCA_CERTIFICATE pCertificate
)
// VMCAFreeCertificate frees the Certificate Objects allocated by the VMCAGetSignedCertificate
//
// Arguments :
//      pCertficate : Points to the Certificate that is to be freed.
// Returns :
//      Error Code
//
{
    if (pCertificate) {
        VMCA_SAFE_FREE_STRINGA(pCertificate);
    }
}


DWORD
VMCAGetRegKeyString(PSTR pszRegKey, PSTR *ppszValue)
{
	#define MAX_REG_VALUE_SIZE 512
	DWORD dwError = 0;
#ifdef _WIN32
	HKEY hKey;
	DWORD dwBufferSize = MAX_REG_VALUE_SIZE;
	char szBuffer[MAX_REG_VALUE_SIZE] = {0};
	//HKEY_LOCAL_MACHINE\SOFTWARE\VMware, Inc.\VMware Certificate Services
	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\VMware, Inc.\\VMware Certificate Services",0, KEY_READ, &hKey);
	BAIL_ON_ERROR(dwError);
	dwError = RegQueryValueEx(hKey, pszRegKey, 0, NULL, (LPBYTE) szBuffer,&dwBufferSize);
	BAIL_ON_ERROR(dwError);
	dwError = VMCAAllocateStringA(szBuffer, ppszValue);
	BAIL_ON_ERROR(dwError);
error :
	RegCloseKey(hKey);
#endif //_WIN32
	return dwError;
}

DWORD
VMCAGetInstallDirectory(PSTR *ppszInstallDir)
{
	#define INSTALL_PATH "InstallPath"
	DWORD dwError = 0;
#ifndef _WIN32
	dwError = VMCAAllocateStringA(VMCA_INSTALL_DIR, ppszInstallDir);
#else
	dwError = VMCAGetRegKeyString(INSTALL_PATH, ppszInstallDir);
#endif
	return dwError;
}


DWORD
VMCAGetDataDirectory(PSTR *ppszDataDir)
{
	#define DATA_PATH "DataPath"
	DWORD dwError = 0;
#ifndef _WIN32
	dwError = VMCAAllocateStringA(VMCA_ROOT_CERT_DIR, ppszDataDir);
#else
	dwError = VMCAGetRegKeyString(DATA_PATH, ppszDataDir);
#endif
	return dwError;
}

DWORD
VMCAGetLogDirectory(PSTR *ppszLogDir)
{
#define LOG_DIR "LogsPath"
DWORD dwError = 0;
#ifndef _WIN32
	dwError = VMCAAllocateStringA(VMCA_LOG_DIR, ppszLogDir);
#else
   dwError = VMCAGetRegKeyString(LOG_DIR, ppszLogDir);
#endif
   return dwError;
}

DWORD
VMCAGetRootCertificateFilePath(PSTR *pszRootPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszRootPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,ROOT_CER);
   BAIL_ON_ERROR(dwError);

error:
   if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}

DWORD
VMCAGetPrivateKeyPath(PSTR *pszPrivPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszPrivPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,PRIVATE_KEY);
   BAIL_ON_ERROR(dwError);

error:
	if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}

DWORD
VMCAGetPrivateKeyPasswordPath(PSTR *pszPrivPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszPrivPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,PRIVATE_PASSWORD);
   BAIL_ON_ERROR(dwError);

error:
	if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}

DWORD
VMCAGetCertsDBPath(PSTR *pszPrivPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszPrivPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,CERTS_DB);
   BAIL_ON_ERROR(dwError);

error:
	if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}

DWORD
VMCAGetCRLNamePath(PSTR *pszPrivPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszPrivPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,VMCA_CRL_NAME);
   BAIL_ON_ERROR(dwError);

error:
	if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}

DWORD
VMCAGetTempCRLNamePath(PSTR *pszPrivPath)
{
   PSTR pszDataPath = NULL;
   DWORD dwError = 0;
   dwError = VMCAGetDataDirectory(&pszDataPath);
   BAIL_ON_ERROR(dwError);
   dwError = VMCAAllocateStringPrintfA(pszPrivPath,"%s%s%s",pszDataPath,VMCA_PATH_SEPARATOR,VMCA_CRL_TMP_NAME);
   BAIL_ON_ERROR(dwError);

error:
	if(pszDataPath  != NULL){
		VMCAFreeStringA(pszDataPath);
   }
   return dwError;
}


static DWORD
VMCABackupFiles(
    PCSTR pszBaseFilePath
    )
{
    DWORD dwError = ERROR_SUCCESS;
    const DWORD MAX_NUMBER_OF_BAKUPS = 32;
    struct STAT st = { 0 };
    DWORD dwCounter;

    PSTR pszDestFile = NULL;
    PSTR pszSourceFile = NULL;

    for (dwCounter = MAX_NUMBER_OF_BAKUPS; dwCounter > 0; --dwCounter)
    {
        dwError = VMCAAllocateStringPrintfA(
                              &pszDestFile,
                              "%s.%d",
                              pszBaseFilePath,
                              dwCounter);
        BAIL_ON_ERROR(dwError);

        if(STAT(pszDestFile, &st) == ERROR_SUCCESS ) 
        {
            UNLINK(pszDestFile);
        }

        dwError = VMCAAllocateStringPrintfA(
                              &pszSourceFile,
                              "%s.%d",
                              pszBaseFilePath,
                              dwCounter - 1);
        BAIL_ON_ERROR(dwError);

        VMCACopyFile(pszSourceFile, pszDestFile);

        VMCA_SAFE_FREE_STRINGA(pszDestFile);
        VMCA_SAFE_FREE_STRINGA(pszSourceFile);

        pszDestFile = NULL;
        pszSourceFile = NULL;
    }

    VMCA_SAFE_FREE_STRINGA(pszDestFile);

    dwError = VMCAAllocateStringPrintfA(
                          &pszDestFile,
                          "%s.%d",
                          pszBaseFilePath,
                          0);
    BAIL_ON_ERROR(dwError);

    VMCACopyFile(pszBaseFilePath, pszDestFile);

error:
    VMCA_SAFE_FREE_STRINGA(pszDestFile);
    VMCA_SAFE_FREE_STRINGA(pszSourceFile);

    return dwError;
}


DWORD
VMCABackupRootCAFiles(
    PCSTR pszRootCACertFile,
    PCSTR pszRootCAPrivateKeyFile,
    PCSTR pszRootCAPasswordFile
    )
{
    DWORD dwError = ERROR_SUCCESS;

    dwError = VMCABackupFiles(pszRootCACertFile);
    BAIL_ON_ERROR(dwError);
    dwError = VMCABackupFiles(pszRootCAPrivateKeyFile);
    BAIL_ON_ERROR(dwError);
    dwError = VMCABackupFiles(pszRootCAPasswordFile);
    BAIL_ON_ERROR(dwError);

error:
    return dwError;
}

DWORD
VMCAAccountDnToUpn(
    PSTR dn,
    PSTR *retUpn)
{
/*
 * Convert:  cn=adam-sles11.ssolabs2.com,ou=Domain Controllers,dc=VSPHERE,dc=LOCAL
 *      to:  adam-sles11.ssolabs2.com@VSPHERELOCAL
 */
    DWORD dwError = 0;

    PSTR ptr = NULL;
    PSTR end = NULL;
    PSTR upn = NULL;
    PSTR fmtupn = NULL;
    PSTR sep = ".";
    DWORD len = (DWORD) strlen(dn);

    dwError = VMCAAllocateMemory(
                  sizeof(CHAR) * (len + 2),
                  (PVOID) &upn);
    if (dwError)
    {
        goto error;
    }
    fmtupn = upn;

    /*
 * TBD: Note: this code assumes DN is all lower case.
     * Handle "cn=" portion of UPN
     */
    ptr = strstr(dn, "cn=");
    if (ptr)
    {
        ptr += 3; /* Skip over cn= */
        end = strstr(ptr, ",ou=");
        if (!end)
        {
            end = strstr(ptr, ",dc=");
        }
        if (end)
        {
            fmtupn += snprintf(fmtupn, len, "%.*s@", (int) (end-ptr), ptr);
        }
    }

    ptr = strstr(ptr, "dc=");
    while (ptr)
    {
        ptr += 3;
        if (*ptr)
        {
            end = strstr(ptr, ",dc=");
            if (!end)
            {
                end = ptr + strlen(ptr);
                sep = "";
            }
            fmtupn += snprintf(fmtupn, len, "%.*s%s", (int) (end-ptr), ptr, sep);
        }
        ptr = strstr(ptr, "dc=");
    }
    *retUpn = upn;
    upn = NULL;

error:
    if (dwError)
    {
        if (upn)
        {
            free(upn);
        }
    }
    return dwError;
}
