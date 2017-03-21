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



#ifndef __DEPLOY_UTILS_H__
#define __DEPLOY_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    VMW_DEPLOY_LOG_LEVEL_ERROR = 0,
    VMW_DEPLOY_LOG_LEVEL_WARNING,
    VMW_DEPLOY_LOG_LEVEL_INFO,
    VMW_DEPLOY_LOG_LEVEL_VERBOSE,
    VMW_DEPLOY_LOG_LEVEL_DEBUG
} VMW_DEPLOY_LOG_LEVEL;

typedef enum
{
    VMW_DEPLOY_LOG_TARGET_NONE = 0,
    VMW_DEPLOY_LOG_TARGET_SYSLOG,
    VMW_DEPLOY_LOG_TARGET_FILE
} VMW_DEPLOY_LOG_TARGET;

typedef enum
{
    VMW_DIR_SVC_MODE_UNKNOWN = 0,
    VMW_DIR_SVC_MODE_STANDALONE,
    VMW_DIR_SVC_MODE_PARTNER,
    VMW_DIR_SVC_MODE_CLIENT
} VMW_DIR_SVC_MODE;

typedef struct _VMW_IC_SETUP_PARAMS
{
    PSTR pszHostname;
    PSTR pszMachineAccount;
    PSTR pszOrgUnit;

    PSTR pszDomainName;
    PSTR pszPassword;

    VMW_DIR_SVC_MODE dir_svc_mode;

    PSTR pszServer;
    PSTR pszSite;

    PSTR pszDNSForwarders;
    PSTR pszSubjectAltName;

    BOOLEAN bDisableAfdListener;
    BOOLEAN bUseMachineAccount;
    BOOLEAN bMachinePreJoined;

} VMW_IC_SETUP_PARAMS, *PVMW_IC_SETUP_PARAMS;

typedef struct _VMW_DEPLOY_LOG_CONTEXT* PVMW_DEPLOY_LOG_CONTEXT;

DWORD
VmwDeployInitialize(
    VOID
    );

DWORD
VmwDeployCreateLogContext(
    VMW_DEPLOY_LOG_TARGET    logTarget,
    VMW_DEPLOY_LOG_LEVEL     logLevel,
    PCSTR                    pszFilePath,
    PVMW_DEPLOY_LOG_CONTEXT* ppContext
    );

DWORD
VmwDeploySetLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    );

DWORD
VmwDeployGetLogLevel(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL*   pLogLevel
    );

DWORD
VmwDeploySetLogLevel(
    PVMW_DEPLOY_LOG_CONTEXT pContext,
    VMW_DEPLOY_LOG_LEVEL    logLevel
    );

VOID
VmwDeployLogMessage(
    VMW_DEPLOY_LOG_LEVEL logLevel,
    PCSTR                pszFormat,
    ...
    );

VOID
VmwDeployReleaseLogContext(
    PVMW_DEPLOY_LOG_CONTEXT pContext
    );

DWORD
VmwDeploySetupInstance(
    PVMW_IC_SETUP_PARAMS pParams
    );

DWORD
VmwDeployDeleteInstance(
    PVMW_IC_SETUP_PARAMS pParams
    );

VOID
VmwDeployFreeSetupParams(
    PVMW_IC_SETUP_PARAMS pParams
    );

DWORD
VmwDeployAllocateMemory(
    size_t size,
    PVOID* ppMemory
    );

VOID
VmwDeployFreeMemory(
    PVOID pMemory
    );

DWORD
VmwDeployAllocateStringW(
    PCWSTR pwszSrc,
    PWSTR* ppwszDst
    );

DWORD
VmwDeployAllocateStringA(
    PCSTR pszSrc,
    PSTR* ppszDst
    );

DWORD
VmwDeployAllocateStringWFromA(
    PCSTR pszSrc,
    PWSTR* ppwszDst
    );

DWORD
VmwDeployAllocateStringAFromW(
    PCWSTR pwszSrc,
    PSTR*  ppszDst
    );

DWORD
VmwDeployAllocateStringPrintf(
    PSTR* ppszString,
    PCSTR pszFormat,
    ...
    );

DWORD
VmwDeployAllocateStringPrintfV(
    PSTR*   ppszStr,
    PCSTR   pszFormat,
    va_list argList
    );

DWORD
VmwDeployGetHostname(
    PSTR* ppszHostname
    );

BOOLEAN
VmwDeployHaveAdminRights(
    VOID
    );

DWORD
VmwDeployValidateHostname(
    PCSTR pszHostname
    );

DWORD
VmwDeployValidateOrgUnit(
    PCSTR pszOrgUnit
    );

DWORD
VmwDeployValidatePassword(
    PCSTR pszPassword
    );

DWORD
VmwDeployValidateSiteName(
    PCSTR pszSite
    );

DWORD
VmwDeployValidatePartnerCredentials(
    PCSTR pszServer,
    PCSTR pszPassword,
    PCSTR pszDomainName
    );

VOID
VmwDeployShutdown(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif /* __DEPLOY_UTILS_H__ */
