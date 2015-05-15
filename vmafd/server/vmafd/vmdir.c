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

static
DWORD
VmAfSrvDirOpenConnection(
    PCWSTR pwszDCName,
    PCWSTR pwszDomain,
    PCWSTR pwszAccount,
    PCWSTR pwszPassword,
    PVMDIR_CONNECTION*ppConnection
    );

static
DWORD
_CreateKrbConfig(
    PCSTR pszDefaultRealm,
    PCSTR pszConfigFileName,
    PCSTR pszDefaultKeytabName,
    PCSTR pszKdcServer1,
    PCSTR pszKdcServer2)
{
    DWORD dwError = 0;
    DWORD dwError2 = 0;
    PVMAFD_KRB_CONFIG pKrbConfig = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pszConfigFileName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszDefaultRealm, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszDefaultKeytabName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pszKdcServer1, dwError);

    dwError = VmAfdInitKrbConfig(pszConfigFileName, &pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetDefaultRealmKrbConfig(pKrbConfig, pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdSetDefaultKeytabNameKrbConfig(pKrbConfig, pszDefaultKeytabName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAddKdcKrbConfig(pKrbConfig, pszKdcServer1);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pszKdcServer2)
    {
        dwError = VmAfdAddKdcKrbConfig(pKrbConfig, pszKdcServer2);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdBackupKrbConfig(pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdWriteKrbConfig(pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    dwError2 = VmAfdDestroyKrbConfig(pKrbConfig);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to create Kerberos configuration. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetSiteGUID(
    PWSTR*   ppwszGUID        /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszGUID = NULL;
    PWSTR pwszGUID = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszGUID, dwError);

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvDirOpenConnection(
                    pwszDCName,
                    pwszDomain,
                    pwszAccount,
                    pwszPassword,
                    &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirGetSiteGuid(pConnection, &pszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszGUID, &pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszGUID = pwszGUID;
                
cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    if (pszGUID)
    {
        VmDirFreeMemory(pszGUID);
    }

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    if (ppwszGUID)
    {
        *ppwszGUID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszGUID);

    goto cleanup;
}

DWORD
VmAfSrvPromoteVmDir(
    PWSTR    pwszLotusServerName, /* IN              */
    PWSTR    pwszDomainName,      /* IN     OPTIONAL */
    PWSTR    pwszUserName,        /* IN              */
    PWSTR    pwszPassword,        /* IN              */
    PWSTR    pwszSiteName,        /* IN     OPTIONAL */
    PWSTR    pwszPartnerHostName  /* IN     OPTIONAL */
    )
{
    DWORD dwError = 0;
    PSTR pszLotusServerName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszSiteName = NULL;
    PSTR pszPartnerHostName = NULL;
    PSTR pszDefaultRealm = NULL;
    PWSTR pwszCurDomainName = NULL;
    BOOLEAN bFirstInstance = TRUE;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszLotusServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_CANNOT_PROMOTE_VMDIR_ALREADY_PROMOTED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    bFirstInstance = pwszDomainName && !pwszPartnerHostName;

    dwError = VmAfdAllocateStringAFromW(pwszLotusServerName, &pszLotusServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszDomainName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszDomainName, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszSiteName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszSiteName, &pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszPartnerHostName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszPartnerHostName, &pszPartnerHostName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (bFirstInstance)
    {
        dwError = VmAfdAllocateStringA(pszDomainName, &pszDefaultRealm);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdUpperCaseStringA(pszDefaultRealm);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = _CreateKrbConfig(
                          pszDefaultRealm,
                          gVmafdGlobals.pszKrb5Config,
                          gVmafdGlobals.pszKrb5Keytab,
                          "localhost",
                          NULL);
        if (dwError)
        {
            dwError = ERROR_CANNOT_CREATE_KERBEROS_CONFIGURATION;
        }
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmDirSetupHostInstance(
                          pszDomainName,
                          pszLotusServerName,
                          pszUserName,
                          pszPassword,
                          pszSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfSrvGetDomainName(&pwszCurDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszCurDomainName, &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(pszDomainName, &pszDefaultRealm);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdUpperCaseStringA(pszDefaultRealm);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = _CreateKrbConfig(
                          pszDefaultRealm,
                          gVmafdGlobals.pszKrb5Config,
                          gVmafdGlobals.pszKrb5Keytab,
                          "localhost",
                          pszPartnerHostName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmDirJoin(
                          pszLotusServerName,
                          pszUserName,
                          pszPassword,
                          pszSiteName,
                          pszPartnerHostName,
                          VMAFD_FIRST_REPL_CYCLE_MODE_COPY_DB);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmDirGetDomainName(
                          pszPartnerHostName,
                          &pszDomainName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvSetDomainNameA(pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CONTROLLER);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszLotusServerName);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszSiteName);
    VMAFD_SAFE_FREE_STRINGA(pszPartnerHostName);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);
    VMAFD_SAFE_FREE_MEMORY(pwszCurDomainName);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvDemoteVmDir(
    PWSTR    pwszServerName,    /* IN              */
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_CANNOT_DEMOTE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pwszServerName, &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirDemote(pszUserName, pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* TODO: remove KrbConfig entries */

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvJoinVmDir(
    PWSTR    pwszServerName,     /* IN            */
    PWSTR    pwszUserName,       /* IN            */
    PWSTR    pwszPassword,       /* IN            */
    PWSTR    pwszMachineName,    /* IN            */
    PWSTR    pwszDomainName,     /* IN            */
    PWSTR    pwszOrgUnit         /* IN   OPTIONAL */
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszMachineName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszOrgUnit = NULL;
    PSTR pszDefaultRealm = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszMachineName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);
    if (domainState != VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = ERROR_CANNOT_JOIN_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pwszServerName, &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszMachineName, &pszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszOrgUnit)
    {
        dwError = VmAfdAllocateStringAFromW(pwszOrgUnit, &pszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszDomainName, &pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdUpperCaseStringA(pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirClientJoin(
                      pszServerName,
                      pszUserName,
                      pszPassword,
                      pszMachineName,
                      pszOrgUnit);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _CreateKrbConfig(
                      pszDefaultRealm,
                      gVmafdGlobals.pszKrb5Config,
                      gVmafdGlobals.pszKrb5Keytab,
                      pszServerName,
                      NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainName(pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CLIENT);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfSrvJoinVmDir: joined Vmdir.");

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszOrgUnit);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "VmAfSrvJoinVmDir: Failed to join Vmdir. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvLeaveVmDir(
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PWSTR pwszServerName = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = ERROR_CANNOT_LEAVE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdAllocateStringAFromW(pwszServerName, &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszUserName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszPassword)
    {
        dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Machine credentials will be used if the user name and password are NULL.
    dwError = VmDirClientLeave(pszServerName, pszUserName, pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

    /*
     * TODO: remove krb5.conf entries, machine account, etc.
     */

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "Failed to leave Vmdir. Error(%u)",
                              dwError);

    goto cleanup;
}

DWORD
VmAfSrvForceReplication(
    PWSTR pwszServerName         /* IN               */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);

    //dwError = VmDirReplNow(pwszServerName);
    //BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvDirGetMachineId(
    VMAFD_DOMAIN_STATE domainState, /* IN              */
    PWSTR*             ppwszGUID    /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PSTR  pszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszGUID = NULL;
    PWSTR pwszGUID = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszGUID, dwError);

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvDirOpenConnection(
                    pwszDCName,
                    pwszDomain,
                    pwszAccount,
                    pwszPassword,
                    &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszAccount, &pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (domainState)
    {
        case VMAFD_DOMAIN_STATE_CONTROLLER:

            dwError = VmDirGetServerID(pConnection, pszAccount, &pszGUID);

            break;

        case VMAFD_DOMAIN_STATE_CLIENT:

            dwError = VmDirGetComputerID(pConnection, pszAccount, &pszGUID);

            break;

        case VMAFD_DOMAIN_STATE_NONE:

            dwError = VECS_NOT_JOINED;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszGUID, &pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszGUID = pwszGUID;

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    if (pszGUID)
    {
        VmDirFreeMemory(pszGUID);
    }

    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    if (ppwszGUID)
    {
        *ppwszGUID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszGUID);

    goto cleanup;
}

DWORD
VmAfSrvDirSetMachineId(
    VMAFD_DOMAIN_STATE domainState,
    PCWSTR   pwszGUID        /* IN */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PSTR  pszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszGUID = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pwszGUID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvDirOpenConnection(
                    pwszDCName,
                    pwszDomain,
                    pwszAccount,
                    pwszPassword,
                    &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszAccount, &pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszGUID, &pszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (domainState)
    {
        case VMAFD_DOMAIN_STATE_CONTROLLER:

            dwError = VmDirSetServerID(pConnection, pszAccount, pszGUID);

            break;

        case VMAFD_DOMAIN_STATE_CLIENT:

            dwError = VmDirSetComputerID(pConnection, pszAccount, pszGUID);

            break;

        case VMAFD_DOMAIN_STATE_NONE:

            dwError = VECS_NOT_JOINED;

            break;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    VMAFD_SAFE_FREE_MEMORY(pszGUID);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvDirGetMemberships(
    PCSTR pszUPNName,
    PSTR  **pppszMemberships,
    PDWORD pdwMemberships
    )
{
    DWORD dwError = 0;
    PSTR *ppszMemberships = NULL;
    DWORD dwMemberships = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pszUPNName) ||
        pppszMemberships == NULL ||
        pdwMemberships == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetMachineAccountInfo(
                    &pwszAccount,
                    &pwszPassword,
                    &pwszAccountDN,
                    NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvDirOpenConnection(
                    pwszDCName,
                    pwszDomain,
                    pwszAccount,
                    pwszPassword,
                    &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirGetMemberships(
                    pConnection,
                    pszUPNName,
                    &ppszMemberships,
                    &dwMemberships);
    BAIL_ON_VMAFD_ERROR(dwError);

    *pppszMemberships = ppszMemberships;
    *pdwMemberships = dwMemberships;

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    return dwError;

error:
    if (ppszMemberships != NULL && dwMemberships > 0)
    {
        VmDirFreeMemberships(ppszMemberships, dwMemberships);
    }
    goto cleanup;
}

VOID
VmAfSrvDirFreeMemberships(
    PSTR* ppszMemberships,
    DWORD dwMemberships
    )
{
    if (ppszMemberships != NULL && dwMemberships > 0)
    {
        VmDirFreeMemberships(ppszMemberships, dwMemberships);
    }
}

BOOLEAN
VmAfSrvDirIsMember(
    PSTR* ppszMemberships,
    DWORD dwMemberships,
    PCSTR pszGroupName
    )
{
    BOOLEAN bRetVal = FALSE;
    int nCmpRet = 0;
    DWORD i = 0;

    for (i = 0; i < dwMemberships; i++)
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "Checking user's group: %s against group: %s ",
                 ppszMemberships[i], pszGroupName);
        nCmpRet = VmAfdStringCompareA(ppszMemberships[i], pszGroupName, FALSE);
        if (nCmpRet == 0)
        {
            bRetVal = TRUE;
            break;
        }
    }

    return bRetVal;
}

static
DWORD
VmAfSrvDirOpenConnection(
    PCWSTR pwszDCName,
    PCWSTR pwszDomain,
    PCWSTR pwszAccount,
    PCWSTR pwszPassword,
    PVMDIR_CONNECTION*ppConnection
    )
{
    DWORD dwError = 0;
    PSTR  pszDCName = NULL;
    PSTR  pszDomain = NULL;
    PSTR  pszAccount = NULL;
    PSTR  pszPassword = NULL;
    PSTR  pszURI = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomain, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszAccount, &pszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                &pszURI,
                "ldap://%s:%d",
                pszDCName,
                LDAP_PORT);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirConnectionOpen(
                pszURI,
                pszDomain,
                pszAccount,
                pszPassword,
                &pConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszURI);
    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pszDomain);
    VMAFD_SAFE_FREE_MEMORY(pszAccount);
    VMAFD_SAFE_FREE_MEMORY(pszPassword);

    return dwError;

error:

    if (ppConnection)
    {
        *ppConnection = NULL;
    }

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    goto cleanup;
}
