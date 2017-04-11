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

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
static
DWORD
VmAfSrvSignalVmdirProvider(
    VOID
    );
#endif

static
DWORD
VmAfdAppendDomain(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PSTR*   ppszServerFQDN
    );

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
VmAfSrvSetDNSRecords(
    PCSTR pszDCAddress,
    PCSTR pszDomain,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszMachineName
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

#ifdef USE_DEFAULT_KRB5_PATHS
    dwError = VmAfdBackupCopyKrbConfig(pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);
#else
    dwError = VmAfdBackupKrbConfig(pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfdWriteKrbConfig(pKrbConfig);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    dwError2 = VmAfdDestroyKrbConfig(pKrbConfig);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "Failed to create Kerberos configuration. Error(%u)",
             dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetSiteGUID(
    PWSTR*   ppwszSiteGUID   /*    OUT          */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszSiteGUID = NULL;
    PWSTR pwszSiteGUID = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszSiteGUID, dwError);

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

    dwError = VmDirGetSiteGuid(pConnection, &pszSiteGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszSiteGUID, &pwszSiteGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszSiteGUID = pwszSiteGUID;

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    if (pszSiteGUID)
    {
        VmDirFreeMemory(pszSiteGUID);
    }

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    if (ppwszSiteGUID)
    {
        *ppwszSiteGUID = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSiteGUID);

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
    PWSTR pwszDCSiteName = NULL;
    PWSTR pwszPNID = NULL;
    PSTR pszPartnerHostName = NULL;
    PSTR pszDefaultRealm = NULL;
    PWSTR pwszCurDomainName = NULL;
    BOOLEAN bFirstInstance = TRUE;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    PSTR pszCanonicalHostName = NULL;
    PSTR pszLocalHostName = NULL;

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
        VmAfdLog( VMAFD_DEBUG_ERROR,"Cur Domain name = %d",dwError);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszCurDomainName, &pszDomainName);
        VmAfdLog( VMAFD_DEBUG_ERROR,"Cur Domain name = %s",pszDomainName );
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

        dwError = VmAfdGetHostName(&pszLocalHostName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdGetCanonicalHostName(
                          pszLocalHostName,
                          &pszCanonicalHostName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvSetDNSRecords(
                          pszPartnerHostName,
                          pszDomainName,
                          pszUserName,
                          pszPassword,
                          pszCanonicalHostName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }


#ifndef _WIN32
    chmod(gVmafdGlobals.pszKrb5Keytab, 0600);
#endif

    dwError = VmAfSrvSetDomainNameA(pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CONTROLLER);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pwszSiteName))
    {
        dwError = VmAfSrvGetSiteNameForDC(pwszLotusServerName, &pwszDCSiteName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvSetSiteName(
                    IsNullOrEmptyString(pwszSiteName)?
                    pwszDCSiteName:
                    pwszSiteName
                    );
    BAIL_ON_VMAFD_ERROR(dwError);

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
    dwError = VmAfSrvSignalVmdirProvider();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    dwError = VmAfSrvGetPNID(&pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                           VMAFD_REG_KEY_DC_NAME,
                           pwszLotusServerName);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfSrvConfigureDNSW(
                      pwszPNID,
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword);
    if (dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s failed to initialize dns. Error(%u)",
            __FUNCTION__,
            dwError);
        dwError = 0;
    }
    else
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s successfully initialized dns.",
            __FUNCTION__);
    }

#if 0
    dwError = VmAfdInitSourceIpThread(&gVmafdGlobals.pSourceIpContext);
    if(dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ANY, "Source IP init failed!");
        dwError = 0;
    }
#endif

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
    dwError = VmAfSrvSignalVmdirProvider();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszLocalHostName);
    VMAFD_SAFE_FREE_STRINGA(pszCanonicalHostName);
    VMAFD_SAFE_FREE_STRINGA(pszLotusServerName);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszSiteName);
    VMAFD_SAFE_FREE_MEMORY(pwszDCSiteName);
    VMAFD_SAFE_FREE_STRINGA(pszPartnerHostName);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);
    VMAFD_SAFE_FREE_MEMORY(pwszCurDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvDemoteVmDir(
    PWSTR    pwszServerName,    /* IN     OPTIONAL */
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword       /* IN              */
    )
{
    DWORD dwError = 0;
    PWSTR pwszServerNameLocal = pwszServerName;
    PWSTR pwszServerName1 = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    PWSTR pwszDomainName = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_CANNOT_DEMOTE_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!pwszServerNameLocal)
    {
        dwError = VmAfdAllocateStringWFromA("localhost", &pwszServerName1);
        BAIL_ON_VMAFD_ERROR(dwError);
        pwszServerNameLocal = pwszServerName1;
    }

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainName(&pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvUnconfigureDNSW(
                        pwszServerNameLocal,
                        pwszDomainName,
                        pwszUserName,
                        pwszPassword);

    if (dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s failed to uninitialize dns. Error(%u)",
            __FUNCTION__,
            dwError);
        dwError = 0;
    }
    else
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s successfully uninitialized dns.",
            __FUNCTION__);
    }

    dwError = VmDirDemote(pszUserName, pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

#if 0
    VmAfdShutdownSrcIpThread(gVmafdGlobals.pSourceIpContext);
    gVmafdGlobals.pSourceIpContext = NULL;
#endif

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
    dwError = VmAfSrvSignalVmdirProvider();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    /* TODO: remove KrbConfig entries */

    if (gVmafdGlobals.pCertUpdateThr)
    {
        VmAfdShutdownCertificateThread(gVmafdGlobals.pCertUpdateThr);
        gVmafdGlobals.pCertUpdateThr = NULL;
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGW(pwszDomainName);
    VMAFD_SAFE_FREE_STRINGW(pwszServerName1);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvJoinValidateCredentials(
    PWSTR pwszDomainName,       /* IN            */
    PWSTR pwszUserName,         /* IN            */
    PWSTR pwszPassword          /* IN            */
    )
{
    DWORD dwError = 0;
    PSTR pszDomainName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszDCHostname = NULL;
    PSTR pszDCAddress = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetDomainController(
                    pszDomainName,
                    pszUserName,
                    pszPassword,
                    &pszDCHostname,
                    &pszDCAddress);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDCHostname);
    VMAFD_SAFE_FREE_STRINGA(pszDCAddress);

    return dwError;

error:

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
    PWSTR pwszSiteName = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszServerName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszMachineName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszDomainName, dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    // If already joined as a client, then force-leave first
    if (domainState == VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = VmAfSrvLeaveVmDir(NULL, NULL, VMAFD_DOMAIN_LEAVE_FLAGS_FORCE);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (domainState != VMAFD_DOMAIN_STATE_NONE)
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
                      pszOrgUnit,
                      0);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _CreateKrbConfig(
                      pszDefaultRealm,
                      gVmafdGlobals.pszKrb5Config,
                      gVmafdGlobals.pszKrb5Keytab,
                      pszServerName,
                      NULL);
    BAIL_ON_VMAFD_ERROR(dwError);

#ifndef _WIN32
    chmod(gVmafdGlobals.pszKrb5Keytab, 0600);
#endif

    dwError = VmAfSrvSetDomainName(pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDCName(pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetSiteNameForDC(pwszServerName, &pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetSiteName(pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDNSRecords(
                      pszServerName,
                      pszDomainName,
                      pszUserName,
                      pszPassword,
                      pszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CLIENT);
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: joined Vmdir.",
             __FUNCTION__);

#if 0

    dwError = VmDdnsInitThread(&gVmafdGlobals.pDdnsContext);
    if(dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "Ddns client failed to Initialise. Error(%d)",
            dwError
        );
    }
#endif

    dwError = CdcSrvInitDefaultHAMode(gVmafdGlobals.pCdcContext);
    if (dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s failed to initiate HA. Error(%u)",
            __FUNCTION__,
            dwError);
        dwError = 0;
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszOrgUnit);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: Failed to join Vmdir. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvJoinVmDir2(
    PWSTR            pwszDomainName,     /* IN            */
    PWSTR            pwszUserName,       /* IN            */
    PWSTR            pwszPassword,       /* IN            */
    PWSTR            pwszMachineName,    /* IN   OPTIONAL */
    PWSTR            pwszOrgUnit,        /* IN   OPTIONAL */
    VMAFD_JOIN_FLAGS dwFlags             /* IN            */
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszMachineName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszOrgUnit = NULL;
    PSTR pszDefaultRealm = NULL;
    PSTR pszHostname = NULL;
    PSTR pszCanonicalHostName = NULL;
    PSTR pszDCHostname = NULL;
    PSTR pszDCAddress = NULL;
    PWSTR pwszDCHostname = NULL;
    PWSTR pwszSiteName = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    DWORD dwDirJoinFlags = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    if (IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_DOMAINNAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        SIZE_T len = 0;

        dwError = VmAfdGetStringLengthW(pwszDomainName, &len);
        BAIL_ON_VMAFD_ERROR(dwError);

        if (len > 255)
        {
            dwError = ERROR_INVALID_DOMAINNAME;
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    // If already joined as a client, then force-leave first
    if (domainState == VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = VmAfSrvLeaveVmDir(NULL, NULL, VMAFD_DOMAIN_LEAVE_FLAGS_FORCE);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else if (domainState != VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = ERROR_CANNOT_JOIN_VMDIR;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdGetHostName(&pszHostname);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!VmAfdStringCompareA(pszHostname, "localhost", TRUE) ||
        !VmAfdStringCompareA(pszHostname, "localhost.localdom", TRUE) ||
        strlen(pszHostname) > 63)
    {
        dwError = ERROR_INVALID_COMPUTERNAME;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pwszDomainName, &pszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdGetDomainController(
                    pszDomainName,
                    pszUserName,
                    pszPassword,
                    &pszDCHostname,
                    &pszDCAddress);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszMachineName)
    {
        dwError = VmAfdAllocateStringAFromW(pwszMachineName, &pszMachineName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdGetCanonicalHostName(pszHostname, &pszCanonicalHostName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringA(pszCanonicalHostName, &pszMachineName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (pwszOrgUnit)
    {
        dwError = VmAfdAllocateStringAFromW(pwszOrgUnit, &pszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringA(pszDomainName, &pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdUpperCaseStringA(pszDefaultRealm);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwDirJoinFlags = (IsFlagSet(dwFlags, VMAFD_JOIN_FLAGS_CLIENT_PREJOINED)) ?
                            VMDIR_CLIENT_JOIN_FLAGS_PREJOINED : 0;

    dwError = VmDirClientJoin(
                      pszDCHostname,
                      pszUserName,
                      pszPassword,
                      pszMachineName,
                      pszOrgUnit,
                      dwDirJoinFlags);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (!IsFlagSet(dwFlags, VMAFD_JOIN_FLAGS_CLIENT_PREJOINED))
    {
        dwError = _CreateKrbConfig(
                        pszDefaultRealm,
                        gVmafdGlobals.pszKrb5Config,
                        gVmafdGlobals.pszKrb5Keytab,
                        pszDCHostname,
                        NULL);
        BAIL_ON_VMAFD_ERROR(dwError);

#ifndef _WIN32
        chmod(gVmafdGlobals.pszKrb5Keytab, 0600);
#endif
    }

    dwError = VmAfSrvSetDomainName(pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszDCHostname, &pwszDCHostname);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetSiteNameForDC(pwszDCHostname, &pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDCName(pwszDCHostname);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetSiteName(pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDNSRecords(
                      pszDCAddress,
                      pszDomainName,
                      pszUserName,
                      pszPassword,
                      pszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

#if !defined(_WIN32) && !defined(PLATFORM_VMWARE_ESX)
    if (IsFlagSet(dwFlags, VMAFD_JOIN_FLAGS_ENABLE_NSSWITCH))
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "%s: configuring NSSWITCH.",
                 __FUNCTION__);

        dwError = DJConfigureNSSwitch();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (IsFlagSet(dwFlags, VMAFD_JOIN_FLAGS_ENABLE_PAM))
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "%s: configuring PAM.",
                 __FUNCTION__);

        dwError = DJConfigurePAM();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    if (IsFlagSet(dwFlags, VMAFD_JOIN_FLAGS_ENABLE_SSH))
    {
        VmAfdLog(VMAFD_DEBUG_ANY,
                 "%s: configuring SSH.",
                 __FUNCTION__);

        dwError = DJConfigureSSH();
        BAIL_ON_VMAFD_ERROR(dwError);
    }
#endif

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_CLIENT);
    BAIL_ON_VMAFD_ERROR(dwError);

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
    dwError = VmAfSrvSignalVmdirProvider();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: joined Vmdir.",
             __FUNCTION__);

    dwError = VmAfdInitCertificateThread(&gVmafdGlobals.pCertUpdateThr);
    if (dwError)
    {
        VmAfdLog(
            VMAFD_DEBUG_ANY,
            "%s failed to initiate certificate thread. Error(%u)",
            __FUNCTION__,
            dwError);
        dwError = 0;
    }

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDomainName);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszOrgUnit);
    VMAFD_SAFE_FREE_STRINGA(pszDefaultRealm);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    VMAFD_SAFE_FREE_MEMORY(pszHostname);
    VMAFD_SAFE_FREE_MEMORY(pszCanonicalHostName);
    VMAFD_SAFE_FREE_MEMORY(pszDCAddress);
    VMAFD_SAFE_FREE_MEMORY(pszDCHostname);
    VMAFD_SAFE_FREE_MEMORY(pwszDCHostname);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: Failed to join Vmdir. Error(%u)",
             __FUNCTION__, dwError);

    if (dwError == VMDIR_ERROR_SERVER_DOWN)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "Failed to reach domain controller at [%s]",
                 VMAFD_SAFE_STRING(pszDCAddress));

        dwError = ERROR_HOST_DOWN;
    }

    goto cleanup;
}

DWORD
VmAfSrvForceLeave(
    VOID
    )
{
    DWORD   dwError = 0;

    dwError = VmAfdRegDeleteValue(
              VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
              VMAFD_REG_KEY_DC_ACCOUNT);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegDeleteValue(
              VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
              VMAFD_REG_KEY_DC_ACCOUNT_DN);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegDeleteValue(
              VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
              VMAFD_REG_KEY_DC_PASSWORD);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdRegDeleteValue(
              VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
              VMAFD_REG_KEY_MACHINE_GUID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfSrvLeaveVmDir(
    PWSTR    pwszUserName,      /* IN              */
    PWSTR    pwszPassword,      /* IN              */
    DWORD    dwLeaveFlags       /* IN              */
    )
{
    DWORD dwError = 0;
    PSTR pszServerName = NULL;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PWSTR pwszServerName = NULL;
    PWSTR pwszMachineAccount = NULL;
    PWSTR pwszMachinePassword = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    if (
        (IsNullOrEmptyString(pwszUserName) && !IsNullOrEmptyString(pwszPassword))
        ||
        (!IsNullOrEmptyString(pwszUserName) && IsNullOrEmptyString(pwszPassword))
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CLIENT)
    {
        dwError = ERROR_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetAffinitizedDC(&pwszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszServerName, &pszServerName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (IsNullOrEmptyString(pwszUserName) &&
        IsNullOrEmptyString(pwszPassword) )
    {
        dwError = VmAfSrvGetMachineAccountInfo(
                                        &pwszMachineAccount,
                                        &pwszMachinePassword,
                                        NULL,
                                        NULL
                                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszMachineAccount, &pszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszMachinePassword, &pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    else
    {
        dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    // Machine credentials will be used if the user name or password are NULL.

    dwError = VmDirClientLeave(
                    pszServerName,
                    pszUserName,
                    pszPassword
                    );

    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_TRACE, "VmDirClientLeave failed. Error [%d].", dwError);

        if (dwLeaveFlags & VMAFD_DOMAIN_LEAVE_FLAGS_FORCE)
        {
            //TODO: Add check for administrator access
            dwError = VmAfSrvForceLeave();
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    dwError = VecsDbReset();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetDomainState(VMAFD_DOMAIN_STATE_NONE);
    BAIL_ON_VMAFD_ERROR(dwError);

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
    dwError = VmAfSrvSignalVmdirProvider();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    /*
     * TODO: remove krb5.conf entries, machine account, etc.
     */

#if !defined(_WIN32) && !defined(PLATFORM_VMWARE_ESX)
    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: unconfiguring NSSWITCH.",
             __FUNCTION__);

    dwError = DJUnconfigureNSSwitch();
    BAIL_ON_VMAFD_ERROR(dwError);

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: unconfiguring PAM.",
             __FUNCTION__);

    dwError = DJUnconfigurePAM();
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = DJUnconfigureSSH();
    BAIL_ON_VMAFD_ERROR(dwError);
#endif

    if (gVmafdGlobals.pCertUpdateThr)
    {
        VmAfdShutdownCertificateThread(gVmafdGlobals.pCertUpdateThr);
        gVmafdGlobals.pCertUpdateThr = NULL;
    }

    if (gVmafdGlobals.pCdcContext)
    {
        CdcSrvShutdownDefaultHAMode(gVmafdGlobals.pCdcContext);
    }

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszServerName);
    VMAFD_SAFE_FREE_STRINGA(pszServerName);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszMachinePassword);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: Failed to leave Vmdir. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvCreateComputerAccount(
    PCWSTR            pwszUserName,       /* IN            */
    PCWSTR            pwszPassword,       /* IN            */
    PCWSTR            pwszMachineName,    /* IN            */
    PCWSTR            pwszOrgUnit,        /* IN   OPTIONAL */
    PWSTR*            ppwszOutPassword    /* OUT           */
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszMachineName = NULL;
    PSTR pszOrgUnit = NULL;
    PWSTR pwszDCName = NULL;
    PSTR pszDCName = NULL;
    PWSTR pwszOutPassword = NULL;
    PSTR pszOutPassword = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszUserName, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pwszMachineName, dwError);

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszMachineName, &pszMachineName);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszOrgUnit)
    {
        dwError = VmAfdAllocateStringAFromW(pwszOrgUnit, &pszOrgUnit);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirCreateComputerAccount(
                  pszDCName,
                  pszUserName,
                  pszPassword,
                  pszMachineName,
                  pszOrgUnit,
                  &pszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszOutPassword, &pwszOutPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (ppwszOutPassword)
    {
        *ppwszOutPassword = pwszOutPassword;
        pwszOutPassword = NULL;
    }

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: created computer account.",
             __FUNCTION__);

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszOutPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszOutPassword);
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszMachineName);
    VMAFD_SAFE_FREE_STRINGA(pszOrgUnit);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s: Failed to create computer account. Error(%u)",
             __FUNCTION__, dwError);

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

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

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

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
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

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

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

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
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

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

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

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
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

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

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

DWORD
VmAfSrvGetSiteNameForDC(
    PWSTR pwszDCName,
    PWSTR* ppwszSiteName
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszDomain = NULL;
    PSTR  pszSiteName = NULL;
    PWSTR pwszSiteName = NULL;
    PVMDIR_CONNECTION pConnection = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszSiteName, dwError);

    if (IsNullOrEmptyString(pwszDCName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

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

    dwError = VmDirGetSiteName(pConnection, &pszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(pszSiteName, &pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszSiteName = pwszSiteName;

cleanup:

    if (pConnection)
    {
        VmDirConnectionClose(pConnection);
    }

    if (pszSiteName)
    {
        VmDirFreeMemory(pszSiteName);
    }

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);

    if (ppwszSiteName)
    {
        *ppwszSiteName = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    goto cleanup;
}

DWORD
VmAfSrvRefreshSiteName(
    VOID
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszSiteName = NULL;

    dwError = VmAfSrvGetDCName(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetSiteNameForDC(
                               pwszDCName,
                               &pwszSiteName
                               );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvSetSiteName(pwszSiteName);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
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

    VmAfdLog(VMAFD_DEBUG_ANY,
             "%s failed. Error(%u)",
             __FUNCTION__, dwError);

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


static
DWORD
VmAfSrvSetDNSRecords(
    PCSTR pszDCAddress,
    PCSTR pszDomain,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszMachineName)
{
    DWORD dwError = 0;
    DWORD dwFlags = 0;
    PVMDNS_SERVER_CONTEXT pServerContext = NULL;
    VMDNS_IP4_ADDRESS* pV4Addresses = NULL;
    DWORD dwNumV4Address = 0;
    VMDNS_IP6_ADDRESS* pV6Addresses = NULL;
    DWORD dwNumV6Address = 0;
    size_t i = 0;
    PVMDNS_RECORD_ARRAY pRecordArray = NULL;
    PSTR pszName = NULL;
    VMDNS_RECORD record = {0};
    CHAR szZone[255] = {0};
    DWORD dwDomainNameStrLen = 0;

    if (VmAfdCheckIfIPV4AddressA(pszMachineName) ||
        VmAfdCheckIfIPV6AddressA(pszMachineName))
    {
       return dwError;
    }

    dwError = VmDnsOpenServerA(
                    pszDCAddress,
                    pszUserName,
                    pszDomain,
                    pszPassword,
                    dwFlags,
                    NULL,
                    &pServerContext);

    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "%s: failed to connect to DNS server %s (%u)",
                 __FUNCTION__, pszDCAddress, dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdStringCpyA(szZone,255,pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwDomainNameStrLen = strlen(szZone);

    if (szZone[dwDomainNameStrLen -1 ] != '.')
    {
        szZone[dwDomainNameStrLen] = '.';
        szZone[dwDomainNameStrLen +1] = 0;
    }

    dwError = VmAfdAppendDomain(pszMachineName, pszDomain, &pszName);
    VmAfdLog(VMAFD_DEBUG_ERROR, "%s: DNS name %s (%u)",
                 __FUNCTION__, pszName, dwError);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetIPAddressesWrap(
                    &pV4Addresses,
                    &dwNumV4Address,
                    &pV6Addresses,
                    &dwNumV6Address);
    if (dwError)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "%s: failed to get interface addresses (%u)",
                 __FUNCTION__, dwError);
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    record.iClass = VMDNS_CLASS_IN;
    record.pszName = pszName;
    record.dwType = VMDNS_RR_TYPE_A;
    record.dwTtl = 3600;

    dwError = VmDnsQueryRecordsA(
                    pServerContext,
                    szZone,
                    pszName,
                    VMDNS_RR_TYPE_A,
                    0,
                    &pRecordArray);
    if (dwError != 0 && dwError != ERROR_NOT_FOUND)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR,
                 "%s: failed to query DNS records (%u),%s, %s",
                 __FUNCTION__, dwError, szZone,pszName);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (dwError == 0)
    {
        record.Data.A.IpAddress = pV4Addresses[0];

        /* delete existing A records for this hostname */
        for (i = 0; i < pRecordArray->dwCount; i++)
        {
            dwError = VmDnsDeleteRecordA(
                            pServerContext,
                            szZone,
                            &pRecordArray->Records[i]);
            if (dwError)
            {
                VmAfdLog(VMAFD_DEBUG_ANY,
                         "%s: failed to delete DNS record for %s (%u)",
                         __FUNCTION__, pRecordArray->Records[i].pszName, dwError);
            }
            BAIL_ON_VMAFD_ERROR(dwError);
        }
    }

    /* add A records for this hostname */
    for (i = 0; i < dwNumV4Address; i++)
    {
        record.Data.A.IpAddress = pV4Addresses[i];

        dwError = VmDnsAddRecordA(
                        pServerContext,
                        szZone,
                        &record);
        if (dwError)
        {
            VmAfdLog(VMAFD_DEBUG_ANY,
                     "%s: failed to add DNS A record for %s (%u)",
                     __FUNCTION__, record.pszName, dwError);
        }
    }

cleanup:

    if (pRecordArray)
    {
        VmDnsFreeRecordArray(pRecordArray);
    }
    VMAFD_SAFE_FREE_MEMORY(pV4Addresses);
    VMAFD_SAFE_FREE_MEMORY(pV6Addresses);

    if (pServerContext)
    {
        VmDnsCloseServer(pServerContext);
    }

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}

#if !defined(_WIN32) && defined(NOTIFY_VMDIR_PROVIDER)
static
DWORD
VmAfSrvSignalVmdirProvider(
    VOID
    )
{
    DWORD dwError = 0;
    HANDLE hLsaConnection = (HANDLE)NULL;

    dwError = LsaOpenServer(&hLsaConnection);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = LsaVmdirSignal(
                    hLsaConnection,
                    LSA_VMDIR_SIGNAL_RELOAD_CONFIG);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    if (hLsaConnection != (HANDLE)NULL)
    {
        LsaCloseServer(hLsaConnection);
    }

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)",
             __FUNCTION__, dwError);

    goto cleanup;
}
#endif

static
DWORD
VmAfdAppendDomain(
    PCSTR   pszServerName,
    PCSTR   pszDomainName,
    PSTR*   ppszServerFQDN
    )
{

    DWORD dwError = 0;
    PSTR  pszServerFQDN = NULL;
    DWORD dwServerStrLen = 0;
    DWORD dwDomainStrLen = 0;
    DWORD dwCursor = 0 ;

    if (!pszServerName || !pszDomainName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    dwServerStrLen = (DWORD)strlen (pszServerName);
    dwDomainStrLen = (DWORD)strlen (pszDomainName);

    if (dwDomainStrLen  > dwServerStrLen)
    {
        // Let assume the hostname is abc and the domain name is xyz.com
        if (pszServerName[dwDomainStrLen-1] != '.')
        {
            // Let assume the hostname is abc and the domain name is xyz.com
            if (pszDomainName[dwDomainStrLen -1] != '.')
            {
                VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s.%s.",pszServerName,pszDomainName);
            }
            else
            {
                VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s.%s",pszServerName,pszDomainName);
            }
        }
        else
        {
            // Let assume the hostname is abc. and the domain name is xyz.com
            if (pszDomainName[dwDomainStrLen -1] != '.')
            {
                VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s.%s.",pszServerName,pszDomainName);
            }
            else
            {
                VmAfdAllocateStringPrintf(
                        &pszServerFQDN,"%s.%s",pszServerName,pszDomainName);
            }

        }
    }
    else if (VmAfdStringStrA(pszServerName,pszDomainName))
    {
        if (pszServerName[dwDomainStrLen -1 ] == '.')
        {
           // server name is abc.xyz.com. and domain name is xyz.com
           pszServerFQDN = (PSTR) pszServerName;
        }
        else
        {
           // server name is abc.xyz.com and domain name is xyz.com
           VmAfdAllocateStringPrintf(
                               &pszServerFQDN,
                               "%s.",
                               pszServerName);
        }

    }
    else
    {
       dwCursor = dwServerStrLen - dwDomainStrLen;
       if (VmAfdStringCompareA(
                  &pszServerName[dwCursor],
                  pszDomainName,
                  FALSE) != 0)
       {
            if (pszServerName[dwServerStrLen - 1] != '.')
            {
                if (pszDomainName[dwDomainStrLen -1 ] != '.')
                {
                    VmAfdAllocateStringPrintf(
                               &pszServerFQDN,
                               "%s.%s.",
                               pszServerName,
                               pszDomainName);
                }
                else
                {
                    VmAfdAllocateStringPrintf(
                               &pszServerFQDN,
                               "%s.%s",
                               pszServerName,
                               pszDomainName);
                }

            }
            else
            {
                if (pszDomainName[dwDomainStrLen -1 ] != '.')
                {
                    VmAfdAllocateStringPrintf(
                                &pszServerFQDN,
                                "%s%s.",
                                pszServerName,
                                pszDomainName);
                }
                else
                {
                    VmAfdAllocateStringPrintf(
                                &pszServerFQDN,
                                "%s%s",
                                pszServerName,
                                pszDomainName);

                }
            }
       }
       else
       {
           VmAfdAllocateStringPrintf(
                          &pszServerFQDN,
                          "%s.",
                          pszServerName);
       }
    }
    *ppszServerFQDN = pszServerFQDN;
cleanup:
    return dwError;

error:
    goto cleanup;

}

