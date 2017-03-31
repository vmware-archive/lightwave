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

// TODO : publish this through vmdirclient.h
DWORD
VmDirSetupLdu(
    PCSTR pszHostURI,
    PCSTR pszDomain,
    PCSTR pszUser,
    PCSTR pszPassword
    );

static
DWORD
VmwDeploySetupServerPrimary(
    PVMW_IC_SETUP_PARAMS pParams
    );

static
DWORD
VmwDeploySetupServerPartner(
    PVMW_IC_SETUP_PARAMS pParams
    );

static
DWORD
VmwDeploySetupServerCommon(
    PVMW_IC_SETUP_PARAMS pParams
    );

static
DWORD
VmwDeploySetupClientWithDC(
    PVMW_IC_SETUP_PARAMS pParams
    );

static
DWORD
VmwDeploySetupClient(
    PVMW_IC_SETUP_PARAMS pParams
    );

static
DWORD
VmwDeployDisableAfdListener(
    void
    );

static
DWORD
VmwDeployGetVmDirConfigPath(
    PSTR* ppszPath
    );

DWORD
VmwDeploySetupInstance(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;

    if (!pParams)
    {
        VMW_DEPLOY_LOG_ERROR("No setup parameters specified");

        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (!VmwDeployHaveAdminRights())
    {
        VMW_DEPLOY_LOG_ERROR("User does not have administrative rights");

        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    switch (pParams->dir_svc_mode)
    {
        case VMW_DIR_SVC_MODE_STANDALONE:

            dwError = VmwDeploySetupServerPrimary(pParams);

            break;

        case VMW_DIR_SVC_MODE_PARTNER:

            dwError = VmwDeploySetupServerPartner(pParams);

            break;

        case VMW_DIR_SVC_MODE_CLIENT:

            if (IsNullOrEmptyString(pParams->pszServer))
            {
                dwError = VmwDeploySetupClient(pParams);
            }
            else
            {
                dwError = VmwDeploySetupClientWithDC(pParams);
            }

            break;

        default:

            dwError = ERROR_INVALID_PARAMETER;

            break;
    }
    BAIL_ON_DEPLOY_ERROR(dwError);

error:

    return dwError;
}

DWORD
VmwDeployDeleteInstance(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;

    // TODO

    return dwError;
}

VOID
VmwDeployFreeSetupParams(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    if (pParams->pszHostname)
    {
        VmwDeployFreeMemory(pParams->pszHostname);
    }
    if (pParams->pszMachineAccount)
    {
        VmwDeployFreeMemory(pParams->pszMachineAccount);
    }
    if (pParams->pszOrgUnit)
    {
        VmwDeployFreeMemory(pParams->pszOrgUnit);
    }
    if (pParams->pszDomainName)
    {
        VmwDeployFreeMemory(pParams->pszDomainName);
    }
    if (pParams->pszPassword)
    {
        VmwDeployFreeMemory(pParams->pszPassword);
    }
    if (pParams->pszServer)
    {
        VmwDeployFreeMemory(pParams->pszServer);
    }
    if (pParams->pszSite)
    {
        VmwDeployFreeMemory(pParams->pszSite);
    }
    if (pParams->pszDNSForwarders)
    {
        VmwDeployFreeMemory(pParams->pszDNSForwarders);
    }
    if (pParams->pszSubjectAltName)
    {
        VmwDeployFreeMemory(pParams->pszSubjectAltName);
    }
    VmwDeployFreeMemory(pParams);
}

static
DWORD
VmwDeploySetupServerPrimary(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;
    PCSTR ppszServices[]=
    {
        VMW_DCERPC_SVC_NAME,
        VMW_VMDNS_SVC_NAME,
        VMW_VMAFD_SVC_NAME,
        VMW_DIR_SVC_NAME,
        VMW_VMCA_SVC_NAME
    };
    int iSvc = 0;

    VMW_DEPLOY_LOG_INFO("Setting up system as Infrastructure standalone node");

    dwError = VmwDeployValidateHostname(pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployValidatePassword(pParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployValidateSiteName(pParams->pszSite);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!IsNullOrEmptyString(pParams->pszDNSForwarders))
    {
        dwError = VmwDeployValidateDNSForwarders(pParams->pszDNSForwarders);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    for (; iSvc < sizeof(ppszServices)/sizeof(ppszServices[0]); iSvc++)
    {
        PCSTR pszService = ppszServices[iSvc];

        VMW_DEPLOY_LOG_INFO("Starting service [%s]", pszService);

        dwError = VmwDeployStartService(pszService);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeploySetupServerCommon(pParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeploySetupServerPartner(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;
    PCSTR ppszServices[]=
    {
        VMW_DCERPC_SVC_NAME,
        VMW_VMDNS_SVC_NAME,
        VMW_VMAFD_SVC_NAME,
        VMW_DIR_SVC_NAME,
        VMW_VMCA_SVC_NAME
    };
    int iSvc = 0;

    VMW_DEPLOY_LOG_INFO("Setting up system as Infrastructure partner node");

    dwError = VmwDeployValidateHostname(pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployValidatePartnerCredentials(
                    pParams->pszServer,
                    pParams->pszPassword,
                    pParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployValidateSiteName(pParams->pszSite);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!IsNullOrEmptyString(pParams->pszDNSForwarders))
    {
        dwError = VmwDeployValidateDNSForwarders(pParams->pszDNSForwarders);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    for (; iSvc < sizeof(ppszServices)/sizeof(ppszServices[0]); iSvc++)
    {
        PCSTR pszService = ppszServices[iSvc];

        VMW_DEPLOY_LOG_INFO("Starting service [%s]", pszService);

        dwError = VmwDeployStartService(pszService);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeploySetupServerCommon(pParams);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeploySetupServerCommon(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;
    PSTR  pszHostname = "localhost";
    PSTR  pszLdapURI = NULL;
    PSTR  pszUsername = VMW_ADMIN_NAME;
    PSTR  pszCACert = NULL;
    PSTR  pszSSLCert = NULL;
    PSTR  pszPrivateKey = NULL;
    PSTR  pszVmdirCfgPath = NULL;

    VMW_DEPLOY_LOG_INFO("Setting various configuration values");

    VMW_DEPLOY_LOG_VERBOSE(
            "Setting Domain Name to [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszDomainName));

    dwError = VmAfdSetDomainNameA(pszHostname, pParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_VERBOSE(
            "Setting Domain Controller Name to [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszHostname));

    dwError = VmAfdSetDCNameA(pszHostname, pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_VERBOSE(
            "Setting PNID to [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszHostname));

    dwError = VmAfdSetPNID(pszHostname, pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_VERBOSE("Setting CA Path to [%s]", VMW_DEFAULT_CA_PATH);

    dwError = VmAfdSetCAPathA(pszHostname, VMW_DEFAULT_CA_PATH);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Promoting directory service to be domain controller");

    dwError = VmAfdPromoteVmDirA(
                    pszHostname,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszSite,
                    pParams->pszServer);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Setting up the logical deployment unit");

    dwError = VmwDeployAllocateStringPrintf(
                    &pszLdapURI,
                    "ldap://%s",
                    pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmDirSetupLdu(
                    pszLdapURI,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (!IsNullOrEmptyString(pParams->pszDNSForwarders))
    {
        VMW_DEPLOY_LOG_INFO("Setting up DNS Forwarders [%s]",
                            pParams->pszDNSForwarders);

        dwError = VmwDeploySetForwarders(
                        pParams->pszDomainName,
                        pszUsername,
                        pParams->pszPassword,
                        pParams->pszDNSForwarders);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    VMW_DEPLOY_LOG_INFO("Setting up VMware Certificate Authority");

    dwError = VmwDeployMakeRootCACert(
                    pParams->pszHostname,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    &pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
         "Adding VMCA's root certificate to VMware endpoint certificate store");

    dwError = VmwDeployAddTrustedRoot(pParams->pszHostname, pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Generating Machine SSL cert");

    dwError = VmwDeployCreateMachineSSLCert(
                    pszHostname,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszHostname,
                    pParams->pszSubjectAltName ?
                        pParams->pszSubjectAltName : pParams->pszHostname,
                    &pszPrivateKey,
                    &pszSSLCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Setting Machine SSL certificate");

    dwError = VmAfdSetSSLCertificate(pszHostname, pszSSLCert, pszPrivateKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
                    "Publishing Machine SSL certificate for directory service");

    dwError = VmwDeployGetVmDirConfigPath(&pszVmdirCfgPath);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployWriteToFile(
                    pszSSLCert,
                    pszVmdirCfgPath,
                    VMW_VMDIR_SSL_CERT_FILE);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmwDeployWriteToFile(
                    pszPrivateKey,
                    pszVmdirCfgPath,
                    VMW_VMDIR_PRIV_KEY_FILE);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Restarting service [%s]", VMW_DIR_SVC_NAME);

    dwError = VmwDeployRestartService(VMW_DIR_SVC_NAME);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (pszVmdirCfgPath)
    {
        VmwDeployFreeMemory(pszVmdirCfgPath);
    }
    if (pszLdapURI)
    {
        VmwDeployFreeMemory(pszLdapURI);
    }
    if (pszCACert)
    {
        VmwDeployFreeMemory(pszCACert);
    }
    if (pszPrivateKey)
    {
        VmwDeployFreeMemory(pszPrivateKey);
    }
    if (pszSSLCert)
    {
        VmwDeployFreeMemory(pszSSLCert);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeploySetupClientWithDC(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;
    PCSTR ppszServices[]=
    {
        VMW_DCERPC_SVC_NAME,
        VMW_VMAFD_SVC_NAME
    };
    PCSTR pszHostname = "localhost";
    PCSTR pszUsername = VMW_ADMIN_NAME;
    int iSvc = 0;
    PSTR pszPrivateKey = NULL;
    PSTR pszCACert = NULL;
    PSTR pszSSLCert = NULL;

    VMW_DEPLOY_LOG_INFO(
            "Joining system to domain [%s] using controller at [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszDomainName),
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszServer));

    if (IsNullOrEmptyString(pParams->pszServer))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployValidateHostname(pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pParams->pszMachineAccount)
    {
        dwError = VmwDeployValidateHostname(pParams->pszMachineAccount);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pParams->pszOrgUnit)
    {
        dwError = VmwDeployValidateOrgUnit(pParams->pszOrgUnit);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    dwError = VmwDeployValidatePartnerCredentials(
                    pParams->pszServer,
                    pParams->pszPassword,
                    pParams->pszDomainName);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pParams->bDisableAfdListener)
    {
        VMW_DEPLOY_LOG_INFO("Disabling AFD Listener");

        dwError = VmwDeployDisableAfdListener();
        BAIL_ON_DEPLOY_ERROR(dwError);

        VMW_DEPLOY_LOG_INFO("Stopping the VMAFD Service...");

        dwError = VmwDeployStopService(VMW_VMAFD_SVC_NAME);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    for (; iSvc < sizeof(ppszServices)/sizeof(ppszServices[0]); iSvc++)
    {
        PCSTR pszService = ppszServices[iSvc];

        VMW_DEPLOY_LOG_INFO("Starting service [%s]", pszService);

        dwError = VmwDeployStartService(pszService);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    VMW_DEPLOY_LOG_INFO("Setting various configuration values");

    dwError = VmAfdSetPNID(pszHostname, pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmAfdSetCAPathA(pszHostname, VMW_DEFAULT_CA_PATH);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
            "Joining system to directory service at [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszServer));

    pszUsername = (pParams->bUseMachineAccount && pParams->pszMachineAccount)
                            ? pParams->pszMachineAccount : VMW_ADMIN_NAME;

    dwError = VmAfdJoinVmDirA(
                    pParams->pszServer,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszMachineAccount ?
                            pParams->pszMachineAccount : pParams->pszHostname,
                    pParams->pszDomainName,
                    pParams->pszOrgUnit);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
                    "Get root certificate from VMware Certificate Authority");

    dwError = VmwDeployGetRootCACert(
                    pParams->pszServer,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    &pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
         "Adding VMCA's root certificate to VMware endpoint certificate store");

    dwError = VmwDeployAddTrustedRoot(pParams->pszServer, pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Generating Machine SSL cert");

    dwError = VmwDeployCreateMachineSSLCert(
                    pParams->pszServer,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszHostname,
                    pParams->pszSubjectAltName ?
                        pParams->pszSubjectAltName : pParams->pszHostname,
                    &pszPrivateKey,
                    &pszSSLCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Setting Machine SSL certificate");

    dwError = VmAfdSetSSLCertificate(pszHostname, pszSSLCert, pszPrivateKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (pszPrivateKey)
    {
        VmwDeployFreeMemory(pszPrivateKey);
    }
    if (pszSSLCert)
    {
        VmwDeployFreeMemory(pszSSLCert);
    }
    if (pszCACert)
    {
        VmwDeployFreeMemory(pszCACert);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeploySetupClient(
    PVMW_IC_SETUP_PARAMS pParams
    )
{
    DWORD dwError = 0;
    PCSTR ppszServices[]=
    {
        VMW_DCERPC_SVC_NAME,
        VMW_VMAFD_SVC_NAME
    };
    PCSTR pszHostname = "localhost";
    PCSTR pszUsername = VMW_ADMIN_NAME;
    int iSvc = 0;
    PSTR pszPrivateKey = NULL;
    PSTR pszCACert = NULL;
    PSTR pszSSLCert = NULL;
    PSTR pszDC = NULL;

    VMW_DEPLOY_LOG_INFO(
            "Joining system to domain [%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszDomainName));

    dwError = VmwDeployValidateHostname(pParams->pszHostname);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pParams->pszMachineAccount)
    {
        dwError = VmwDeployValidateHostname(pParams->pszMachineAccount);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    if (pParams->pszOrgUnit)
    {
        dwError = VmwDeployValidateOrgUnit(pParams->pszOrgUnit);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    pszUsername = (pParams->bUseMachineAccount && pParams->pszMachineAccount)
                            ? pParams->pszMachineAccount : VMW_ADMIN_NAME;

    VMW_DEPLOY_LOG_INFO(
            "Validating Domain credentials for user [%s@%s]",
            VMW_DEPLOY_SAFE_LOG_STRING(pszUsername),
            VMW_DEPLOY_SAFE_LOG_STRING(pParams->pszDomainName));

    dwError = VmAfdJoinValidateDomainCredentialsA(
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword);
    BAIL_ON_DEPLOY_ERROR(dwError);

    if (pParams->bDisableAfdListener)
    {
        VMW_DEPLOY_LOG_INFO("Disabling AFD Listener");

        dwError = VmwDeployDisableAfdListener();
        BAIL_ON_DEPLOY_ERROR(dwError);

        VMW_DEPLOY_LOG_INFO("Stopping the VMAFD Service...");

        dwError = VmwDeployStopService(VMW_VMAFD_SVC_NAME);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    for (; iSvc < sizeof(ppszServices)/sizeof(ppszServices[0]); iSvc++)
    {
        PCSTR pszService = ppszServices[iSvc];

        VMW_DEPLOY_LOG_INFO("Starting service [%s]", pszService);

        dwError = VmwDeployStartService(pszService);
        BAIL_ON_DEPLOY_ERROR(dwError);
    }

    VMW_DEPLOY_LOG_INFO("Setting configuration values");

    dwError = VmAfdSetCAPathA(pszHostname, VMW_DEFAULT_CA_PATH);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Performing domain join operation");

    dwError = VmAfdJoinVmDir2A(
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszMachineAccount ?
                        pParams->pszMachineAccount : pParams->pszHostname,
                    pParams->pszOrgUnit,
                    pParams->bMachinePreJoined ?
                        VMAFD_JOIN_FLAGS_CLIENT_PREJOINED : 0);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = VmAfdGetDCNameA(pszHostname, &pszDC);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
                    "Get root certificate from VMware Certificate Authority");

    dwError = VmwDeployGetRootCACert(
                    pszDC,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    &pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO(
         "Adding VMCA's root certificate to VMware endpoint certificate store");

    dwError = VmwDeployAddTrustedRoot(pszDC, pszCACert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Generating Machine SSL cert");

    dwError = VmwDeployCreateMachineSSLCert(
                    pszDC,
                    pParams->pszDomainName,
                    pszUsername,
                    pParams->pszPassword,
                    pParams->pszHostname,
                    pParams->pszSubjectAltName ?
                        pParams->pszSubjectAltName : pParams->pszHostname,
                    &pszPrivateKey,
                    &pszSSLCert);
    BAIL_ON_DEPLOY_ERROR(dwError);

    VMW_DEPLOY_LOG_INFO("Setting Machine SSL certificate");

    dwError = VmAfdSetSSLCertificate(pszHostname, pszSSLCert, pszPrivateKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (pszPrivateKey)
    {
        VmwDeployFreeMemory(pszPrivateKey);
    }
    if (pszSSLCert)
    {
        VmwDeployFreeMemory(pszSSLCert);
    }
    if (pszCACert)
    {
        VmwDeployFreeMemory(pszCACert);
    }
    if (pszDC)
    {
        VmwDeployFreeMemory(pszDC);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeployDisableAfdListener(
    void
    )
{
    DWORD dwError = 0;
    HANDLE hConnection = NULL;
    HKEY   hRootKey = NULL;
    HKEY   hParamKey = NULL;
    DWORD  dwValue = 0;

    dwError = RegOpenServer(&hConnection);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    hConnection,
                    NULL,
                    "HKEY_THIS_MACHINE",
                    0,
                    KEY_READ,
                    &hRootKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = RegOpenKeyExA(
                    hConnection,
                    hRootKey,
                    "Services\\vmafd\\Parameters",
                    0,
                    KEY_SET_VALUE,
                    &hParamKey);
    BAIL_ON_DEPLOY_ERROR(dwError);

    dwError = RegSetValueExA(
                    hConnection,
                    hParamKey,
                    "EnableDCERPC",
                    0,
                    REG_DWORD,
                    (PBYTE)&dwValue,
                    sizeof(dwValue));
     BAIL_ON_DEPLOY_ERROR(dwError);

cleanup:

    if (hConnection)
    {
        if (hParamKey)
        {
            RegCloseKey(hConnection, hParamKey);
        }
        if (hRootKey)
        {
            RegCloseKey(hConnection, hRootKey);
        }

        RegCloseServer(hConnection);
    }

    return dwError;

error:

    goto cleanup;
}

static
DWORD
VmwDeployGetVmDirConfigPath(
    PSTR* ppszPath
    )
{
    DWORD dwError = 0;
    PSTR  pszPath = NULL;

    dwError = VmwDeployAllocateStringA(
                    VMDIR_CONFIG_PATH,
                    &pszPath);
    BAIL_ON_DEPLOY_ERROR(dwError);

    *ppszPath = pszPath;

cleanup:

    return dwError;

error:

    *ppszPath = NULL;

    goto cleanup;
}
