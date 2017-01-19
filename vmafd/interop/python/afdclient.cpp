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
#include <vmafdclient.h>
#include <vecsclient.h>
#include <iostream>
#include "afdclient.h"
#include <vecs_error.h>

void THROW_IF_NEEDED(unsigned int dwError)
{
    if (dwError)
    {
      std::cout << "Error Code : " << dwError << std::endl;
      throw new vmafd_exception(dwError);
    }
}

PSTR
VMAFDErrorCodeToName(int code)
{
    return (PSTR) UNKNOWN_STRING;
}

//
// Client Functions
//
client::client(std::string CertServerName) : ServerName(CertServerName)
{

}

client::client() : ServerName("localhost")
{

}

std::string client::GetStatus()
{
    DWORD dwError = 0;
    VMAFD_STATUS status = VMAFD_STATUS_UNKNOWN;
    std::string result;

    dwError = VmAfdGetStatusA(ServerName.c_str(), &status);
    BAIL_ON_ERROR(dwError);

    if (status == VMAFD_STATUS_UNKNOWN) {
        result.assign("Unknown");
    }

    if (status == VMAFD_STATUS_INITIALIZING) {
        result.assign("Initializing");
    }

    if (status == VMAFD_STATUS_RUNNING) {
        result.assign("Running");
    }

error:
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetDomainName()
{
    PSTR pszDomainName = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetDomainNameA(ServerName.c_str(), &pszDomainName);
    BAIL_ON_ERROR(dwError);

    result.assign(pszDomainName);

error:
    if (pszDomainName != NULL)
    {
        VmAfdFreeString(pszDomainName);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetDomainName(std::string domainName)
{
    DWORD dwError = 0;

    dwError = VmAfdSetDomainNameA(ServerName.c_str(),
                                  domainName.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

std::string client::GetDomainState()
{
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetDomainStateA(ServerName.c_str(), &domainState);
    BAIL_ON_ERROR(dwError);

    switch (domainState)
    {
      case VMAFD_DOMAIN_STATE_NONE:
        result.assign("None");
        break;
      case VMAFD_DOMAIN_STATE_CONTROLLER:
        result.assign("Controller");
        break;
      case VMAFD_DOMAIN_STATE_CLIENT:
        result.assign("Client");
        break;
      default:
        result.assign("Unknown");
        break;
    }

error:
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetLDU()
{
    PSTR pszDomainName = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetLDUA(ServerName.c_str(), &pszDomainName);
    BAIL_ON_ERROR(dwError);

    result.assign(pszDomainName);

error:
    if (pszDomainName != NULL) {
        VmAfdFreeString(pszDomainName);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetLDU(std::string LDUName)
{
    DWORD dwError = 0;

    dwError = VmAfdSetLDUA(ServerName.c_str(), LDUName.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

void client::SetRHTTPProxyPort(unsigned int port)
{
    DWORD dwError = 0;

    dwError = VmAfdSetRHTTPProxyPortA(ServerName.c_str(), port);
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

void client::SetDCPort(unsigned int port)
{
    DWORD dwError = 0;

    dwError = VmAfdSetDCPortA(ServerName.c_str(), port);
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

std::string client::GetCMLocation()
{
    PSTR pszDomainName = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetCMLocationA(ServerName.c_str(), &pszDomainName);
    BAIL_ON_ERROR(dwError);

    result.assign(pszDomainName);

error:
    if (pszDomainName != NULL)
    {
        VmAfdFreeString(pszDomainName);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetLSLocation()
{
    PSTR pszLSLocation= NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetLSLocationA(ServerName.c_str(), &pszLSLocation);
    BAIL_ON_ERROR(dwError);

    result.assign(pszLSLocation);

error:
    if (pszLSLocation != NULL)
    {
        VmAfdFreeString(pszLSLocation);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetDCName()
{
    PSTR pszDomainName = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetDCNameA(ServerName.c_str(), &pszDomainName);
    BAIL_ON_ERROR(dwError);

    result.assign(pszDomainName);

error:
    if (pszDomainName != NULL)
    {
        VmAfdFreeString(pszDomainName);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetDCName(std::string MachineName)
{
    DWORD dwError = 0;

    dwError = VmAfdSetDCNameA(ServerName.c_str(),
                    MachineName.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

std::string client::GetPNID()
{
    PSTR pszPNID = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetPNIDA(ServerName.c_str(), &pszPNID);
    BAIL_ON_ERROR(dwError);

    result.assign(pszPNID);

error:
    if (pszPNID != NULL)
    {
        VmAfdFreeString(pszPNID);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetPNID(std::string PNID)
{
    DWORD dwError = 0;

    dwError = VmAfdSetPNIDA(ServerName.c_str(),
                            PNID.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

std::string client::GetCAPath()
{
    PSTR pszCAPath = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetCAPathA(ServerName.c_str(), &pszCAPath);
    BAIL_ON_ERROR(dwError);

    result.assign(pszCAPath);

error:
    if (pszCAPath != NULL)
    {
        VmAfdFreeString(pszCAPath);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetCAPath(std::string path)
{
    DWORD dwError = 0;

    dwError = VmAfdSetCAPathA(ServerName.c_str(),
                            path.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

std::string client::GetMachineName()
{
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetMachineAccountInfoA(ServerName.c_str(),
                         &pszAccount,&pszPassword);

    BAIL_ON_ERROR(dwError);

    result.assign(pszAccount);

error:
    if (pszAccount != NULL)
    {
        VmAfdFreeString(pszAccount);
        VmAfdFreeString(pszPassword);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetMachinePassword()
{
    PSTR pszAccount = NULL;
    PSTR pszPassword = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetMachineAccountInfoA(ServerName.c_str(),
                         &pszAccount,&pszPassword);

    BAIL_ON_ERROR(dwError);

    result.assign(pszPassword);

error:
    if (pszAccount != NULL)
    {
        VmAfdFreeString(pszAccount);
        VmAfdFreeString(pszPassword);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetMachineCert()
{
    PSTR pszPrivateKey = NULL;
    PSTR pszCertificate = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetSSLCertificate(ServerName.c_str(),
                         &pszCertificate,&pszPrivateKey);
    BAIL_ON_ERROR(dwError);

    result.assign(pszCertificate);

error:
    if (pszPrivateKey != NULL){
        VmAfdFreeString(pszPrivateKey);
    }
    if(pszCertificate != NULL){
        VmAfdFreeString(pszCertificate);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetMachinePrivateKey()
{
    PSTR pszPrivateKey = NULL;
    PSTR pszCertificate = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetSSLCertificate(ServerName.c_str(),
                         &pszCertificate,&pszPrivateKey);

    BAIL_ON_ERROR(dwError);

    result.assign(pszPrivateKey);

error:
    if (pszPrivateKey != NULL) {
        VmAfdFreeString(pszPrivateKey);
    }
    if(pszCertificate != NULL){
        VmAfdFreeString(pszCertificate);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

DWORD
ReadFileContentsToString(std::string FileName, PSTR *ppszData)
{
    DWORD dwError = 0;
    FILE * fp = NULL;
    size_t stDataSize = 0;
    PSTR pszFileData = NULL;
    DWORD dwReadSize =0;
    fflush(NULL); // This prevents writes in Python code which lingers in Memory
#ifdef _WIN32
#define stat _stat
#endif
    struct stat filedata = { 0 };
    dwError = stat(FileName.c_str(), &filedata);
    BAIL_ON_ERROR(dwError);

    stDataSize = filedata.st_size;
    dwError = VmAfdAllocateMemory(stDataSize + 1,(PVOID*) &pszFileData);
    BAIL_ON_ERROR(dwError);

    dwError = VmAfdOpenFilePath(FileName.c_str(), "r", &fp, 0);
    BAIL_ON_ERROR(dwError);

    dwReadSize = fread(pszFileData, 1, stDataSize, fp);
    if (dwReadSize != stDataSize)
    {
        dwError = VECS_GENERIC_FILE_IO;
        BAIL_ON_ERROR(dwError);
    }

    *ppszData = pszFileData;

cleanup:
    if (fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    VMAFD_SAFE_FREE_MEMORY(pszFileData);
    goto cleanup;
}

void client::SetMachineCertWithString(
    std::string PEMEncodedCertificate,
    std::string PEMEncodedPrivateKey)
{
    DWORD dwError = 0;

    if (PEMEncodedCertificate.length() <= 0)
    {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    if (PEMEncodedPrivateKey.length() <= 0)
    {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    dwError = VmAfdSetSSLCertificate(ServerName.c_str(),
        (PSTR) PEMEncodedCertificate.c_str(),
        (PSTR) PEMEncodedPrivateKey.c_str());
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::AddCert(
    opaque Store,
    int iEntryType,
    std::string Alias,
    std::string Cert,
    std::string PrivateKey,
    std::string Password,
    bool bAutoRefresh)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PSTR pszPassword = NULL;
    PSTR pszPrivateKey = NULL;
    CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;

    pszPrivateKey = (PSTR)PrivateKey.c_str();
    if (pszPrivateKey && pszPrivateKey[0] == '\0') {
        pszPrivateKey = NULL;
    }

    pszPassword = (PSTR)Password.c_str();
    if (pszPassword && pszPassword[0] == '\0') {
        pszPassword = NULL;
    }

    switch (iEntryType)
    {
      case 1:
        entryType = CERT_ENTRY_TYPE_PRIVATE_KEY;
        break;
      case 2:
        entryType = CERT_ENTRY_TYPE_SECRET_KEY;
        break;
      case 3:
        entryType = CERT_ENTRY_TYPE_TRUSTED_CERT;
        break;
      case 0:
      default:
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    pStore = (PVECS_STORE)Store;

    dwError =  VecsAddEntryA(
                pStore,
                entryType,
                (PSTR) Alias.c_str(),
                (PSTR) Cert.c_str(),
                pszPrivateKey,
                pszPassword,
                bAutoRefresh);
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::AddTrustedRoot(
    std::string Cert)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    dwError = VecsCreateCertStoreA(
                  (PSTR) ServerName.c_str(),
                  TRUSTED_ROOTS_STORE_NAME,
                  NULL,
                  &pStore);
    if (dwError == ERROR_ALREADY_EXISTS)
    {
        dwError = VecsOpenCertStoreA(
                      (PSTR) ServerName.c_str(),
                      TRUSTED_ROOTS_STORE_NAME,
                      NULL,
                      &pStore);
    }
    BAIL_ON_ERROR(dwError);

    dwError =  VecsAddEntryA(
                pStore,
                CERT_ENTRY_TYPE_TRUSTED_CERT,
                NULL,
                (PSTR) Cert.c_str(),
                NULL,
                NULL,
                0);
    if (dwError == ERROR_ALREADY_EXISTS) 
    {
        dwError = 0;
    }
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pStore)
    {
        VecsCloseCertStore(pStore);
    }
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::SetMachineCert(
    std::string CertificateFile,
    std::string PrivateKeyFile)
{
    DWORD dwError = 0;
    PSTR pszCertificate = NULL;
    PSTR pszPrivateKey = NULL;

    if (CertificateFile.length() <= 0)
    {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    if(PrivateKeyFile.length() <= 0)
    {
        dwError = VECS_GENERIC_FILE_IO;
        BAIL_ON_ERROR(dwError);
    }

    dwError = ReadFileContentsToString(CertificateFile.c_str(),&pszCertificate);
    BAIL_ON_ERROR(dwError);

    dwError = ReadFileContentsToString(PrivateKeyFile.c_str(), &pszPrivateKey);
    BAIL_ON_ERROR(dwError);

    dwError = VmAfdSetSSLCertificate(
                  ServerName.c_str(),
                  pszCertificate,
                  pszPrivateKey);
    BAIL_ON_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszPrivateKey);
    VMAFD_SAFE_FREE_MEMORY(pszCertificate);
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

std::string client::GetSiteGUID()
{
    PSTR pszSiteGUID = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetSiteGUIDA(
                  ServerName.c_str(),
                  &pszSiteGUID);

    BAIL_ON_ERROR(dwError);

    result.assign(pszSiteGUID);

error:
    if (pszSiteGUID != NULL)
    {
        VmAfdFreeString(pszSiteGUID);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetSiteName()
{
    PSTR pszSiteName = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetSiteNameA(
                  ServerName.c_str(),
                  &pszSiteName);

    BAIL_ON_ERROR(dwError);

    result.assign(pszSiteName);

error:
    if (pszSiteName != NULL)
    {
        VmAfdFreeString(pszSiteName);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

std::string client::GetMachineID()
{
    PSTR pszMachineID = NULL;
    DWORD dwError = 0;
    std::string result;

    dwError = VmAfdGetMachineIDA(
                  ServerName.c_str(),
                  &pszMachineID);

    BAIL_ON_ERROR(dwError);

    result.assign(pszMachineID);

error:
    if (pszMachineID != NULL)
    {
        VmAfdFreeString(pszMachineID);
    }
    THROW_IF_NEEDED(dwError);
    return result;
}

void client::SetMachineID(std::string id)
{
    DWORD dwError = 0;

    dwError = VmAfdSetMachineIDA(
                  ServerName.c_str(),
                  id.c_str());
    BAIL_ON_ERROR(dwError);

error:
    THROW_IF_NEEDED(dwError);
}

void client::JoinDomain(
    std::string DomainName,
    std::string OU,
    std::string UserName,
    std::string Password)
{
    DWORD dwError = 0;
    // dwError = VmAfdJoinVmDirA(
    //         NULL,
    //         DomainName.c_Str(),
    //         OU.c_str(),
    //         UserName.c_Str(),
    //         Password.c_Str()
    // );

    BAIL_ON_ERROR(dwError);

error:
     THROW_IF_NEEDED(dwError);
     return;
}

opaque client::CreateCertStore(
    std::string StoreName,
    std::string Password)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    opaque result;
    PSTR pszPassword = NULL;

    pszPassword = (PSTR)Password.c_str();
    if (pszPassword && pszPassword[0] == '\0') {
        pszPassword = NULL;
    }

    dwError = VecsCreateCertStoreA(
                  ServerName.c_str(),
                  (PSTR)StoreName.c_str(),
                  pszPassword,
                  &pStore);
    BAIL_ON_ERROR(dwError);

    result = (opaque)pStore;

cleanup:
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

opaque client::OpenCertStore(
    std::string StoreName,
    std::string Password)
{
    DWORD dwError = 0;
    opaque result;
    PVECS_STORE pStore = NULL;
    PSTR pszPassword = NULL;

    pszPassword = (PSTR)Password.c_str();
    if (pszPassword && pszPassword[0] == '\0') {
        pszPassword = NULL;
    }

    dwError = VecsOpenCertStoreA(
                  ServerName.c_str(),
                  StoreName.c_str(),
                  pszPassword,
                  &pStore);
    BAIL_ON_ERROR(dwError);

    result = (opaque)pStore;

cleanup:
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::CloseCertStore(
    opaque Store)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    pStore = (PVECS_STORE) Store;

    dwError = VecsCloseCertStore(pStore);
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

std::string client::GetCertByAlias(
    opaque Store,
    std::string Alias)
{
    DWORD dwError = 0;
    PSTR pszAlias = NULL;
    PSTR pszCertificate = NULL;
    PVECS_STORE pStore = NULL;
    std::string result;

    if (Alias.length() <= 0)
    {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    pStore = (PVECS_STORE)Store;

    dwError = VecsGetCertificateByAliasA(
                  pStore,
                  (PSTR)Alias.c_str(),
                  &pszCertificate);
    BAIL_ON_ERROR(dwError);

    result.assign(pszCertificate);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszCertificate);
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

std::string client::GetPrivateKeyByAlias(
    opaque Store,
    std::string Alias,
    std::string Password)
{
    DWORD dwError = 0;
    PSTR pszAlias = NULL;
    PSTR pszPrivateKey = NULL;
    PVECS_STORE pStore = NULL;
    std::string result;

    if (Alias.length() <= 0)
    {
        dwError = VECS_NO_CERT_FOUND;
        BAIL_ON_ERROR(dwError);
    }

    pStore = (PVECS_STORE)Store;

    dwError = VecsGetKeyByAliasA(
                  pStore,
                  (PSTR)Alias.c_str(),
                  (PSTR)Password.c_str(),
                  &pszPrivateKey);
    BAIL_ON_ERROR(dwError);

    result.assign(pszPrivateKey);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pszPrivateKey);
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

int client::GetEntryCount(
    opaque Store)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    DWORD dwEntryCount = 0;
    int result = 0;

    pStore = (PVECS_STORE)Store;

    dwError = VecsGetEntryCount(
                  pStore,
                  &dwEntryCount);
    BAIL_ON_ERROR(dwError);

    result = (int)dwEntryCount;

cleanup:
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

opaque client::BeginEnumAliases(
    opaque Store,
    int EntryCount)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;
    opaque result;

    pStore = (PVECS_STORE)Store;

    dwError = VecsBeginEnumEntries(
                  pStore,
                  EntryCount,
                  ENTRY_INFO_LEVEL_1,
                  &pEnumContext);
    BAIL_ON_ERROR(dwError);

    result = (opaque)pEnumContext;

cleanup:
    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

bpl::list client::EnumAliases(
    opaque EnumContext)
{
    DWORD dwError = 0;
    bpl::list result;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    PSTR pAlias = NULL;
    PVECS_CERT_ENTRY_A pEntries = NULL;

    pEnumContext = (PVECS_ENUM_CONTEXT)EnumContext;

    dwError = VecsEnumEntriesA(
                  pEnumContext,
                  &pEntries,
                  &dwCount);
    if (dwError == 0 && dwCount > 0) {
        for (dwIndex=0; dwIndex<dwCount; dwIndex++)
        {
            pAlias = pEntries[dwIndex].pszAlias;
            if (pAlias)
            {
                std::string st(pAlias);
                result.append(st);
            }
        }
    }

    if (dwError == ERROR_NO_MORE_ITEMS)
    {
        dwError = 0;
    }
    BAIL_ON_ERROR(dwError);

cleanup:

    if (pEntries)
    {
        VecsFreeCertEntryArrayA(pEntries, dwCount);
    }

    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::EndEnumAliases(
    opaque EnumContext)
{
    DWORD dwError = 0;
    PVECS_ENUM_CONTEXT pEnumContext = NULL;

    pEnumContext = (PVECS_ENUM_CONTEXT)EnumContext;

    dwError = VecsEndEnumEntries(pEnumContext);
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::DeleteCert(
    opaque Store,
    std::string Alias)
{
    DWORD dwError = 0;
    PVECS_STORE pStore = NULL;

    pStore = (PVECS_STORE)Store;

    dwError = VecsDeleteEntryA(
                  pStore,
                  Alias.c_str());
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::DeleteCertStore(
    std::string StoreName)
{
    DWORD dwError = 0;

    dwError = VecsDeleteCertStoreA(
                  ServerName.c_str(),
                  StoreName.c_str());
    BAIL_ON_ERROR(dwError);

cleanup:
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::EnableClientAffinity()
{
    DWORD dwError = 0;

    //This code would eventually move out of here
    //when other APIs become remotable

    PVMAFD_SERVER pServer = NULL;

    dwError = VmAfdOpenServerA(NULL,NULL,NULL,&pServer);
    BAIL_ON_ERROR(dwError);

    dwError = CdcEnableClientAffinity(pServer);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }
    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

void client::DisableClientAffinity()
{
    DWORD dwError = 0;

    //This code would eventually move out of here
    //when other APIs become remotable

    PVMAFD_SERVER pServer = NULL;

    dwError = VmAfdOpenServerA(NULL,NULL,NULL,&pServer);
    BAIL_ON_ERROR(dwError);

    dwError = CdcDisableClientAffinity(pServer);
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    return;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

std::string client::GetAffinitizedDC(std::string DomainName, bool bForceRefresh)
{
    PSTR pszAffinitizedDC = NULL;
    DWORD dwError = 0;
    std::string result;
    PCDC_DC_INFO_A pDomainControllerInfo = NULL;
    //This code would eventually move out of here
    //when other APIs become remotable
    PVMAFD_SERVER pServer = NULL;

    dwError = VmAfdOpenServerA(NULL,NULL,NULL,&pServer);
    BAIL_ON_ERROR(dwError);

    dwError = CdcGetDCNameA(
                         pServer,
                         (PSTR)DomainName.c_str(),
                         NULL,
                         NULL,
                         bForceRefresh,
                         &pDomainControllerInfo);
    BAIL_ON_ERROR(dwError);

    result.assign(pDomainControllerInfo->pszDCName);

cleanup:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    if (pDomainControllerInfo)
    {
        CdcFreeDomainControllerInfoA(pDomainControllerInfo);
    }

    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

bpl::list client::EnumDCEntries()
{
    DWORD dwError = 0;
    bpl::list result;
    DWORD dwCount = 0;
    DWORD dwIndex = 0;
    PSTR pszDCEntry = NULL;
    PSTR *ppszDCEntries = NULL;
    //This code would eventually move out of here
    //when other APIs become remotable
    PVMAFD_SERVER pServer = NULL;

    dwError = VmAfdOpenServerA(NULL,NULL,NULL,&pServer);
    BAIL_ON_ERROR(dwError);

    dwError = CdcEnumDCEntriesA(
                            pServer,
                            &ppszDCEntries,
                            &dwCount);
    if (dwError == 0 && dwCount > 0)
    {
        for (dwIndex=0; dwIndex<dwCount; dwIndex++)
        {
            if (ppszDCEntries[dwIndex])
            {
                std::string st(ppszDCEntries[dwIndex]);
                result.append(st);
            }
        }
    }
    BAIL_ON_ERROR(dwError);

cleanup:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    if (ppszDCEntries)
    {
        CdcFreeStringArrayA(ppszDCEntries, dwCount);
    }

    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}

std::string client::GetCdcState()
{
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;
    DWORD dwError = 0;
    std::string result;
    //This code would eventually move out of here
    //when other APIs become remotable
    PVMAFD_SERVER pServer = NULL;

    dwError = VmAfdOpenServerA(NULL,NULL,NULL,&pServer);
    BAIL_ON_ERROR(dwError);


    dwError = CdcGetCurrentState(pServer, &cdcState);
    BAIL_ON_ERROR(dwError);

    switch (cdcState)
    {
      case CDC_DC_STATE_NO_DC_LIST:
        result.assign("NO_DC_LIST");
        break;
      case CDC_DC_STATE_SITE_AFFINITIZED:
        result.assign("SITE_AFFINITIZED");
        break;
      case CDC_DC_STATE_OFF_SITE:
        result.assign("OFF_SITE");
        break;
      case CDC_DC_STATE_NO_DCS_ALIVE:
        result.assign("NO_DCS_ALIVE");
        break;
      case CDC_DC_STATE_LEGACY:
        result.assign("DISABLED");
        break;
      default:
        result.assign("UNKNOWN");
        break;
    }

cleanup:
    if (pServer)
    {
        VmAfdCloseServer(pServer);
    }

    return result;

error:
    THROW_IF_NEEDED(dwError);
    goto cleanup;
}
