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
_ConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
    );

static
DWORD
_ConfigSetString(
    PCSTR    pszSubKeyParam, /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PCWSTR   pwszValue       /* IN     */
    );

static
DWORD
_ConfigGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
    );

static
DWORD
_ConfigSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue         /* IN     */
    );

static
DWORD
_FormatUrl(
    PCSTR    pszScheme,      /* IN     */
    PCSTR    pszHost,        /* IN     */
    DWORD    dwPort,         /* IN     */
    PCSTR    pszPath,        /* IN     */
    PCSTR    pszQuery,       /* IN     */
    PSTR*    ppszUrl         /*    OUT */
    );

DWORD
VmAfSrvGetDomainName(
    PWSTR*   ppwszDomain        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;

    dwError = _ConfigGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DOMAIN_NAME,
                               &pwszDomain);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppwszDomain = pwszDomain;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetDomainNameA(
    PSTR* ppszDomain        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;
    PSTR  pszDomain = NULL;

    dwError = VmAfSrvGetDomainName(&pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszDomain, &pszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszDomain =  pszDomain;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    return dwError;

error:

    VMAFD_SAFE_FREE_STRINGA(pszDomain);

    goto cleanup;
}

DWORD
VmAfSrvSetDomainName(
    PWSTR    pwszDomain        /* IN     */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DOMAIN_NAME,
                               pwszDomain);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetSiteName(
    PWSTR    pwszSiteName        /* IN     */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_SITE_NAME,
                               pwszSiteName);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetSiteName(
    PWSTR*   ppwszSiteName        /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszSiteName = NULL;

    dwError = _ConfigGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_SITE_NAME,
                               &pwszSiteName);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppwszSiteName = pwszSiteName;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);
    goto cleanup;
}

DWORD
VmAfSrvSetDomainNameA(
    PSTR    pszDomain        /* IN     */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomain = NULL;

    dwError = VmAfdAllocateStringWFromA(
                      pszDomain,
                      &pwszDomain);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DOMAIN_NAME,
                               pwszDomain);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    VMAFD_SAFE_FREE_MEMORY(pwszDomain);

    goto cleanup;
}

DWORD
VmAfSrvGetDomainState(
    PVMAFD_DOMAIN_STATE pDomainState       /* IN */
    )
{
    DWORD dwError = 0;
    DWORD dwValue = 0;

    dwError = _ConfigGetInteger(VMAFD_REG_KEY_DOMAIN_STATE,
                                &dwValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pDomainState = (VMAFD_DOMAIN_STATE)dwValue;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetDomainState(
    VMAFD_DOMAIN_STATE domainState       /* IN */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetInteger(VMAFD_REG_KEY_DOMAIN_STATE,
                                (DWORD)domainState);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetLDU(
    PWSTR*   ppwszLDU       /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszLDU = NULL;

    dwError = _ConfigGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_LDU,
                               &pwszLDU);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppwszLDU = pwszLDU;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetLDU(
    PWSTR    pwszLDU           /* IN     */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_LDU,
                               pwszLDU);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetRHTTPProxyPort(
    PDWORD pdwPort           /* OUT */
    )
{
    DWORD dwError = 0;
    DWORD dwPort = 0;

    dwError = _ConfigGetInteger(VMAFD_REG_KEY_RHTTPPROXY_PORT,
                                &dwPort);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwPort = dwPort;

cleanup:

    return dwError;

error:

    if (dwError != ERROR_FILE_NOT_FOUND)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    }

    goto cleanup;
}

DWORD
VmAfSrvSetRHTTPProxyPort(
    DWORD dwPort               /* IN    */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetInteger(VMAFD_REG_KEY_RHTTPPROXY_PORT,
                                dwPort);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetDCPort(
    PDWORD pdwPort           /* OUT */
    )
{
    DWORD dwError = 0;
    DWORD dwPort = 0;

    dwError = _ConfigGetInteger(VMAFD_REG_KEY_DC_PORT,
                                &dwPort);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwPort = dwPort;

cleanup:

    return dwError;

error:

    if (dwError != ERROR_FILE_NOT_FOUND)
    {
        VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);
    }

    goto cleanup;
}

DWORD
VmAfSrvSetDCPort(
    DWORD dwPort               /* IN    */
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetInteger(VMAFD_REG_KEY_DC_PORT,
                                dwPort);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetCMLocation(
    PWSTR*   ppwszCMLocation  /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszCMLocation = NULL;
    PSTR pszCMLocation = NULL;
    PWSTR pwszPNID = NULL;
    PSTR pszPNID = NULL;
    PWSTR pwszMachineID = NULL;
    PSTR pszMachineID = NULL;
    PSTR pszQuery = NULL;
    DWORD dwPort = 0;

    dwError = VmAfSrvGetPNID(&pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                         pwszPNID,
                         &pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvCfgGetMachineID(&pwszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                         pwszMachineID,
                         &pszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetRHTTPProxyPort(&dwPort);
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringPrintf(
                         &pszQuery,
                         "?hostid=%s",
                         pszMachineID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _FormatUrl("https",
                         pszPNID,
                         dwPort,
                         "/cm/sdk",
                         pszQuery,
                         &pszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(
                         pszCMLocation,
                         &pwszCMLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszCMLocation = pwszCMLocation;

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszQuery);
    VMAFD_SAFE_FREE_MEMORY(pwszPNID);
    VMAFD_SAFE_FREE_STRINGA(pszPNID);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);
    VMAFD_SAFE_FREE_STRINGA(pszMachineID);
    VMAFD_SAFE_FREE_STRINGA(pszCMLocation);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetLSLocation(
    PWSTR*   ppwszLSLocation  /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszLSLocation = NULL;
    PSTR pszLSLocation = NULL;
    PWSTR pwszDCName = NULL;
    PSTR pszDCName = NULL;
    DWORD dwPort = 0;

    dwError = VmAfSrvGetAffinitizedDC(&pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(
                               pwszDCName,
                               &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDCPort(&dwPort);
    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        dwError = 0;
    }
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _FormatUrl("https",
                         pszDCName,
                         dwPort,
                         "/lookupservice/sdk",
                         NULL,
                         &pszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringWFromA(
                               pszLSLocation,
                               &pwszLSLocation);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszLSLocation = pwszLSLocation;

cleanup:

    VMAFD_SAFE_FREE_MEMORY(pszDCName);
    VMAFD_SAFE_FREE_MEMORY(pwszDCName);
    VMAFD_SAFE_FREE_STRINGA(pszLSLocation);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetDCName(
    PWSTR*   ppwszDCName    /*    OUT */
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;

    dwError = _ConfigGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_NAME,
                               &pwszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszDCName = pwszDCName;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetDCName(
    PWSTR    pwszDCName     /* IN     */
    )
{
    DWORD           dwError = 0;
    PSTR            pszDCName = NULL;
    PVMAFD_REG_ARG  pArgs = NULL;
    LDAP*           pLDAP = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    dwError = VmAfdAllocateStringAFromW(pwszDCName, &pszDCName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    switch (domainState)
    {
    case VMAFD_DOMAIN_STATE_NONE:
        /* allow initial setting of DCName */
        dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_NAME,
                               pwszDCName);
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);
        break;

    case VMAFD_DOMAIN_STATE_CONTROLLER:
        /* cannot change DC after promoted to controller */
        dwError = ERROR_OPERATION_NOT_PERMITTED;
        BAIL_ON_VMAFD_ERROR(dwError);
        break;

    case VMAFD_DOMAIN_STATE_CLIENT:
        /*
         * allow re-pointing if new DC is in the same Lotus federation.
         * Verify this via machine account authentication to new DC.
         */
        dwError = VmAfdGetMachineInfo( &pArgs );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdLDAPConnect(
                        pszDCName,  // new DC
                        0,          // use default LDAP port
                        pArgs->pszAccountUPN,
                        pArgs->pszPassword,
                        &pLDAP);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_NAME,
                               pwszDCName);
        BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

        dwError = VmAfSrvRefreshSiteName();
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcSrvShutdownDefaultHAMode(gVmafdGlobals.pCdcContext);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = CdcSrvInitDefaultHAMode(gVmafdGlobals.pCdcContext);
        BAIL_ON_VMAFD_ERROR(dwError);

        break;

    default:
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }


    VmAfdLog(VMAFD_DEBUG_DEBUG, "%s succeeded, DCName=%s", __FUNCTION__, pszDCName);

cleanup:

    if ( pArgs != NULL )
    {
        VmAfdFreeRegArgs( pArgs );
    }

    if ( pLDAP != NULL )
    {
        ldap_unbind_ext(pLDAP, NULL, NULL);
    }

    VMAFD_SAFE_FREE_STRINGA(pszDCName);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetPNID(
    PWSTR* ppwszPNID
    )
{
    DWORD dwError = 0;
    PWSTR pwszPNID = NULL;

    dwError = _ConfigGetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_PNID,
                               &pwszPNID);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppwszPNID = pwszPNID;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetPNID(
    PWSTR pwszPNID
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_PNID,
                               pwszPNID);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetCAPath(
    PWSTR* ppwszPath
    )
{
    DWORD dwError = 0;
    PWSTR pwszPath = NULL;

    dwError = _ConfigGetString( VMAFD_CONFIG_PARAMETER_KEY_PATH,
                                VMAFD_REG_KEY_CA_PATH,
                                &pwszPath);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *ppwszPath = pwszPath;

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetCAPath(
    PWSTR pwszPath
    )
{
    DWORD dwError = 0;

    dwError = _ConfigSetString(VMAFD_CONFIG_PARAMETER_KEY_PATH, VMAFD_REG_KEY_CA_PATH, pwszPath);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvSetDCActPassword(
    PWSTR    pwszPassword   /* IN     */
    )
{
    DWORD dwError = 0;
    PWSTR pwszOldPassword = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszPassword, dwError);

    _ConfigGetString( VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                      VMAFD_REG_KEY_DC_PASSWORD,
                      &pwszOldPassword);

    dwError = _ConfigSetString( VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                VMAFD_REG_KEY_DC_PASSWORD,
                                pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (pwszOldPassword)
    {
        dwError = _ConfigSetString(VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMAFD_REG_KEY_DC_OLD_PASSWORD,
                                   pwszOldPassword);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszOldPassword);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvGetMachineAccountInfo(
    PWSTR*   ppwszAccount,     /*    OUT */
    PWSTR*   ppwszPassword,    /*    OUT */
    PWSTR*   ppwszAccountDN,   /* Optional, out */
    PWSTR*   ppwszMachineGUID  /* Optional, out */
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszAccountDN = NULL;
    PWSTR pwszMachineGUID = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszAccount, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPassword, dwError);

    dwError = _ConfigGetString(VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_ACCOUNT,
                               &pwszAccount);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _ConfigGetString(VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_PASSWORD,
                               &pwszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    /* Optional Argument */
    if (ppwszAccountDN != NULL)
    {
        dwError = _ConfigGetString(VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                               VMAFD_REG_KEY_DC_ACCOUNT_DN,
                               &pwszAccountDN);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    /* Optional Argument */
    if (ppwszMachineGUID != NULL)
    {
        dwError = VmAfSrvCfgGetMachineID(&pwszMachineGUID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszAccount = pwszAccount;
    *ppwszPassword = pwszPassword;
    if (ppwszAccountDN)
    {
        *ppwszAccountDN = pwszAccountDN;
    }
    if (ppwszMachineGUID)
    {
        *ppwszMachineGUID = pwszMachineGUID;
    }

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pwszAccountDN);
    VMAFD_SAFE_FREE_MEMORY(pwszMachineGUID);

    goto cleanup;
}


DWORD
VecsSrvGetDBBasePath(
    PSTR *ppszDbPath
    )
{
    DWORD dwError = 0;
    PWSTR pwszDbPath = NULL;
    PSTR  pszDbPath = NULL;

    if (!ppszDbPath)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = _ConfigGetString(VMAFD_REG_PATH,
                               VMAFD_REG_KEY_DB_PATH,
                               &pwszDbPath);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    dwError = VmAfdAllocateStringAFromW(
                                        pwszDbPath,
                                        &pszDbPath
                                       );
    BAIL_ON_VMAFD_ERROR (dwError);

    *ppszDbPath = pszDbPath;

cleanup:
    VMAFD_SAFE_FREE_MEMORY(pwszDbPath);

    return dwError;

error:
    if (ppszDbPath)
    {
        *ppszDbPath = NULL;
    }

    VMAFD_SAFE_FREE_MEMORY (pszDbPath);

    goto cleanup;
}

DWORD
VmAfSrvSetMachineSSLCert(
    PWSTR    pwszPrivateKey,    /* IN      */
    DWORD    dwKeyLength,       /* IN     */
    PBYTE    pPublicKey         /* IN     */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pwszPrivateKey, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pPublicKey, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmAfSrvGetMachineSSLCert(
    PWSTR*   ppwszPrivateKey,  /*    OUT */
    PBYTE*   ppPublicKey,      /*    OUT */
    PDWORD   pdwKeyLength      /* IN OUT */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszPrivateKey, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(ppPublicKey, dwError);
    BAIL_ON_VMAFD_INVALID_POINTER(pdwKeyLength, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Opens a cert store, the Store types are
    Private, Trusted Store, Revoked Store
    and CA Root Certificate Store.
*/
UINT32
VmAfSrvOpenCertStore(
    DWORD    type,
    DWORD *  hStore
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}
/*
    Closes a cert store
*/
UINT32
VmAfSrvCloseCertStore(
    DWORD    hStore
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}
/*
    Add Certificate takes Certificate allows the user
    to put an alias to the Certificate.
*/
UINT32
VmAfSrvAddCertificate(
    UINT32   hStore,
    PWSTR    pszAlias,
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey,
    UINT32   uAutoRefresh
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Delete Certificate takes Certificate or Alias
    to find the cert and delete it
*/
UINT32
VmAfSrvDeleteCertificate(
    DWORD    hStore,
    PWSTR    pszCertificate
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Allows the User to Enum a Certificate Store
*/
UINT32
VmAfSrvEnumCertificates(
    DWORD                   hStore,
    DWORD                   dwStartIndex,
    DWORD                   dwNumCertificates,
    PVMAFD_CERT_CONTAINER** ppCertContainer
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}
/*
    Checks if a particular certificate is Trusted in the
    Store Trusted Store or CA_ROOT Store , fails it is
    in Revoked Store
*/
UINT32
VmAfSrvVerifyCertificateTrust(
    PWSTR    pszCertificate
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Returns a Certificate Chain if the Parent
    Certs are in the TRUSTED, or CA Store.
*/
UINT32
VmAfSrvGetCertificateChain(
    PWSTR                  pszCertificate,
    PVMAFD_CERT_CONTAINER* ppCertContainer
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
     Returns a Certificate and Private key (if Available) by its alias or by Certificate itself.
*/
UINT32
VmAfSrvGetCertificateByAlias(
    DWORD                  hStore,
    PWSTR                  pszAlias,
    PVMAFD_CERT_CONTAINER* ppCertContainer
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Password Store - Add
*/

UINT32
VmAfSrvSetPassword(
    PWSTR    pszAliasName,
    PWSTR    pszPassword
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
    Password Store - Set Password
*/

UINT32
VmAfSrvGetPassword(
    PWSTR    pszAliasName,
    PWSTR *  ppszPassword
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
  Set Machine SSL Cert
*/
DWORD
VmAfSrvSetSSLCertificate(
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
  Get Machine SSL Cert
*/
DWORD
VmAfSrvGetSSLCertificate(
    PWSTR    pszCertificate,
    PWSTR    pszPrivateKey
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
  Internal - set machine id
*/
DWORD
VmAfSrvCfgSetMachineID(
    PCWSTR pwszMachineID
    )
{
    DWORD dwError = 0;
    BOOLEAN bInLock = FALSE;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE ;
    PWSTR pwszID = NULL;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = VECS_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pwszMachineID))
    {
        dwError = VmAfdAllocateStringW(pwszMachineID, &pwszID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMAFD_LOCK_MUTEX(bInLock, &gVmafdGlobals.mutex);

    VMAFD_SAFE_FREE_MEMORY(gVmafdGlobals.pwszMachineId);

    gVmafdGlobals.pwszMachineId = pwszID;
    pwszID = NULL;

cleanup:

    VMAFD_UNLOCK_MUTEX(bInLock, &gVmafdGlobals.mutex);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    VMAFD_SAFE_FREE_MEMORY(pwszID);

    goto cleanup;
}

/*
  Return machine id guid
*/
DWORD
VmAfSrvCfgGetMachineID(
    PWSTR *ppwszMachineID
    )
{
    DWORD   dwError = 0;
    BOOLEAN bInLock = FALSE;
    PWSTR   pwszMachineID = NULL;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE ;

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState == VMAFD_DOMAIN_STATE_NONE)
    {
        dwError = VECS_NOT_JOINED;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    VMAFD_LOCK_MUTEX(bInLock, &gVmafdGlobals.mutex);

    if (IsNullOrEmptyString(gVmafdGlobals.pwszMachineId))
    {
        VMAFD_UNLOCK_MUTEX(bInLock, &gVmafdGlobals.mutex);

        dwError = VmAfSrvDirGetMachineId(domainState, &pwszMachineID);
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfSrvCfgSetMachineID(pwszMachineID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    else
    {
        dwError = VmAfdAllocateStringW(
                        gVmafdGlobals.pwszMachineId,
                        &pwszMachineID);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    *ppwszMachineID = pwszMachineID;

cleanup:

    VMAFD_UNLOCK_MUTEX(bInLock, &gVmafdGlobals.mutex);

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    if (ppwszMachineID)
    {
        *ppwszMachineID = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);

    goto cleanup;
}

DWORD
VmAfSrvSetMachineID(
    PWSTR pwszGUID
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;

    if (!VmAfdSrvIsValidGUID(pwszGUID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Directory

    dwError = VmAfSrvDirSetMachineId(domainState, pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

    // Configuration

    dwError = VmAfSrvCfgSetMachineID(pwszGUID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmAfdLog(VMAFD_DEBUG_ERROR, "%s failed. Error(%u)", __FUNCTION__, dwError);

    goto cleanup;
}

DWORD
VmAfSrvChangePNID(
    PCWSTR pwszUserName,
    PCWSTR pwszPassword,
    PCWSTR pwszPNID
    )
{
    DWORD dwError = 0;
    PSTR pszUserName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszPNID = NULL;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszPNID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateStringAFromW(pwszUserName, &pszUserName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPassword, &pszPassword);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszPNID, &pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmDirChangePNID(pszUserName, pszPassword, pszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _ConfigSetString(
            VMAFD_CONFIG_PARAMETER_KEY_PATH,
            VMAFD_REG_KEY_PNID,
            pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = _ConfigSetString(
            VMAFD_CONFIG_PARAMETER_KEY_PATH,
            VMAFD_REG_KEY_DC_NAME,
            pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

cleanup:
    VMAFD_SAFE_FREE_STRINGA(pszUserName);
    VMAFD_SAFE_FREE_STRINGA(pszPassword);
    VMAFD_SAFE_FREE_STRINGA(pszPNID);
    return dwError;

error:
    VmAfdLog(VMAFD_DEBUG_ANY, "%s failed. Error(%u)", __FUNCTION__, dwError);
    goto cleanup;
}

static
DWORD
_ConfigGetString(
    PCSTR    pszSubKey,      /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PWSTR*   ppwszValue      /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PSTR  pszValue = NULL;
    PWSTR pwszValue = NULL;

    BAIL_ON_VMAFD_INVALID_POINTER(ppwszValue, dwError);

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

    dwError = VmAfdAllocateStringWFromA(pszValue, &pwszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppwszValue = pwszValue;

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

    VMAFD_SAFE_FREE_STRINGA(pszValue);

    return dwError;

error:

    if (ppwszValue)
    {
        *ppwszValue = NULL;
    }

    goto cleanup;
}

static
DWORD
_ConfigSetString(
    PCSTR    pszSubKeyParam, /* IN     */
    PCSTR    pszValueName,   /* IN     */
    PCWSTR   pwszValue       /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = pszSubKeyParam ? pszSubKeyParam : VMAFD_CONFIG_PARAMETER_KEY_PATH;
    PSTR  pszValue = NULL;

    if (IsNullOrEmptyString(pszValueName) ||
        IsNullOrEmptyString(pwszValue))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

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
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdAllocateStringAFromW(pwszValue, &pszValue);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pParamsKey,
                    pszValueName,
                    REG_SZ,
                    pszValue,
                    (DWORD)strlen(pszValue) + 1);
    BAIL_ON_VMAFD_ERROR(dwError);

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

    VMAFD_SAFE_FREE_STRINGA(pszValue);

    return dwError;

error:

    goto cleanup;
}

static
DWORD
_ConfigGetInteger(
    PCSTR    pszValueName,   /* IN     */
    PDWORD   pdwValue        /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;
    DWORD dwValue = 0;

    BAIL_ON_VMAFD_INVALID_POINTER(pdwValue, dwError);

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

    dwError = VmAfConfigReadDWORDValue(
                    pParamsKey,
                    NULL,
                    pszValueName,
                    &dwValue);
    BAIL_ON_VMAFD_ERROR_NO_LOG(dwError);

    *pdwValue = dwValue;

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

    goto cleanup;
}

static
DWORD
_ConfigSetInteger(
    PCSTR    pszValueName,   /* IN     */
    DWORD    dwValue         /* IN     */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PCSTR pszSubKey = VMAFD_CONFIG_PARAMETER_KEY_PATH;

    if (IsNullOrEmptyString(pszValueName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

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
                    KEY_SET_VALUE,
                    &pParamsKey);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfConfigSetValue(
                    pParamsKey,
                    pszValueName,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(DWORD));
    BAIL_ON_VMAFD_ERROR(dwError);

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

    goto cleanup;
}

static
DWORD
_FormatUrl(
    PCSTR    pszScheme,      /* IN     */
    PCSTR    pszHost,        /* IN     */
    DWORD    dwPort,         /* IN     */
    PCSTR    pszPath,        /* IN     */
    PCSTR    pszQuery,       /* IN     */
    PSTR*    ppszUrl         /*    OUT */
    )
{
    DWORD dwError = 0;
    PSTR pszUrl = NULL;
    PSTR pszPort = NULL;
    PSTR pszHostPrefix = "";
    PSTR pszHostSuffix = "";

    if (IsNullOrEmptyString(pszScheme) ||
        IsNullOrEmptyString(pszHost) ||
        IsNullOrEmptyString(pszPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    if (VmAfdIsIPV6AddrFormat(pszHost))
    {
        pszHostPrefix = "[";
        pszHostSuffix = "]";
    }

    if (dwPort)
    {
        dwError = VmAfdAllocateStringPrintf(
                        &pszPort,
                        ":%d",
                        dwPort);
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfdAllocateStringPrintf(
                        &pszUrl,
                        "%s://%s%s%s%s%s%s",
                        pszScheme,
                        pszHostPrefix,
                        pszHost,
                        pszHostSuffix,
                        (pszPort ? pszPort : ""),
                        pszPath,
                        (pszQuery ? pszQuery : ""));
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszUrl = pszUrl;

cleanup:

    VMAFD_SAFE_FREE_STRINGA(pszPort);

    return dwError;

error:

    VMAFD_SAFE_FREE_STRINGA(pszUrl);

    goto cleanup;
}

DWORD
VmAfSrvGetRegKeySecurity(
    PCSTR    pszSubKey,      /* IN     */
    PSTR*    ppszSecurity    /*    OUT */
    )
{
    DWORD dwError = 0;
    PVMAF_CFG_CONNECTION pConnection = NULL;
    PVMAF_CFG_KEY pRootKey = NULL;
    PVMAF_CFG_KEY pParamsKey = NULL;
    PSTR  pszSecurity = NULL;

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

    dwError = VmAfConfigGetSecurity(
                    pParamsKey,
                    &pszSecurity);
    BAIL_ON_VMAFD_ERROR(dwError);

    *ppszSecurity = pszSecurity;

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

    goto cleanup;
}
