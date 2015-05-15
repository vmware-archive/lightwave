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
 * Module Name: vdcupgrade
 *
 * Filename: main.c
 *
 * Abstract:
 *
 * vdcupgrade module entry point
 *
 */

#include "includes.h"

#define VMDIR_MAX_PWD_LEN 127
static
DWORD
AddComputersContainer(
    LDAP* pLd,
    PCSTR pszServerName
    );

static
DWORD
AddCAContainer(
    LDAP* pLd,
    PCSTR pszServerName
    );

static
DWORD
AddBuiltinDCClientsGroup(
    LDAP* pLd,
    PCSTR pszServerName
    );

static
DWORD
AddBuiltinCAAdminsGroup(
    LDAP* pLd,
    PCSTR pszServerName
    );

static
DWORD
AccountDNToName(
    PCSTR pszDCAccountDN,
    PSTR* ppszDCAccount
    );

static
DWORD
SetDCAccountRegistryKey(
    VOID
    );

static
DWORD
UpgradeDirectory(
    LDAP* pLd,
    PCSTR pszServerName,
    PCSTR pszUserUpn,
    PCSTR pszPassword
    );

static
DWORD
UpdateDCAccountSRPSecret(
    LDAP* pLd
    );

static
DWORD
UpdateEntriesACL(
    LDAP*   pLd,
    PCSTR pszServerName,
    PCSTR pszAdminUPN
    );

static
DWORD
_UpdatePSCVersion(
    LDAP* pLd
    );

int
main(
    int argc,
    char* argv[]
    )
{
    DWORD   dwError = 0;
    PSTR    pszServerName = NULL;
    PSTR    pszAdminUPN = NULL;
    PSTR    pszPassword = NULL;
    PSTR    pszPasswordFile = NULL;
    PSTR    pszErrorMessage = NULL;
    LDAP*   pLd = NULL;
    CHAR    pszPasswordBuf[VMDIR_MAX_PWD_LEN + 1];
    BOOLEAN bAclOnly = FALSE;

#ifndef _WIN32
    setlocale(LC_ALL, "");
#endif

    dwError = VmDirParseArgs(
                        argc, argv,
                        &pszServerName,
                        &pszAdminUPN,
                        &pszPassword,
                        &pszPasswordFile,
                        &bAclOnly);
    if (dwError != ERROR_SUCCESS)
    {
        ShowUsage();
        goto cleanup;
    }

    if (!pszServerName ||
        !pszAdminUPN   ||
        (pszPassword && pszPasswordFile))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    memset(pszPasswordBuf, 0, sizeof(pszPasswordBuf));

    if (pszPassword == NULL && pszPasswordFile != NULL)
    {
        dwError = VmDirReadStringFromFile(pszPasswordFile, pszPasswordBuf, sizeof(pszPasswordBuf));
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (pszPassword != NULL && pszPasswordFile == NULL)
    {
        dwError = VmDirStringCpyA(pszPasswordBuf, VMDIR_MAX_PWD_LEN, pszPassword);
        BAIL_ON_VMDIR_ERROR(dwError);
    } else
    {
        VmDirReadString("password: ", pszPasswordBuf, VMDIR_MAX_PWD_LEN+1, FALSE);
    }

    dwError = VmDirSafeLDAPBind(&pLd, pszServerName, pszAdminUPN, pszPasswordBuf);
    BAIL_ON_VMDIR_ERROR(dwError);

    // do ACL patch first, so newly added entry will have correct ACL.
    dwError = UpdateEntriesACL( pLd, pszServerName, pszAdminUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!bAclOnly)
    {

        dwError = UpgradeDirectory(pLd, pszServerName, pszAdminUPN, pszPasswordBuf);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = UpdateDCAccountSRPSecret( pLd );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszAdminUPN);
    if (pszPassword)
        memset(pszPassword, 0, strlen(pszPassword));
    memset(pszPasswordBuf, 0, sizeof(pszPasswordBuf));
    VMDIR_SAFE_FREE_STRINGA(pszPassword);
    VMDIR_SAFE_FREE_STRINGA(pszPasswordFile);
    VMDIR_SAFE_FREE_MEMORY(pszErrorMessage);

    VdcLdapUnbind(pLd);
    pLd = NULL;

    return dwError;

error:
    VmDirGetErrorMessage(dwError, &pszErrorMessage);
    printf("Vdcupgrade failed. Error[%d] - %s\n",
        dwError, ( pszErrorMessage ) ? pszErrorMessage : "");
    goto cleanup;
}

static
DWORD
UpgradeDirectory(
    LDAP* pLd,
    PCSTR pszServerName,
    PCSTR pszAdminUPN,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;

    dwError = _UpdatePSCVersion(pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AddComputersContainer(pLd, pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AddCAContainer(pLd, pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AddBuiltinDCClientsGroup(pLd, pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AddBuiltinCAAdminsGroup(pLd, pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = SetDCAccountRegistryKey();
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

static
DWORD
AddComputersContainer(
    LDAP* pLd,
    PCSTR pszServerName
    )
{
    DWORD  dwError = 0;
    PCSTR  pszComputersContainerName = VMDIR_COMPUTERS_RDN_VAL;
    PSTR   pszDomainName = NULL;
    PSTR   pszDomainDN = NULL;
    PSTR   pszComputersContainerDN = NULL;

    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(
                  pszServerName,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(
                  pszDomainName,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszComputersContainerDN, "%s=%s,%s",
                  ATTR_OU,
                  pszComputersContainerName,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VdcIfDNExist(pLd, pszComputersContainerDN))
    {
        dwError = VdcLdapAddContainer(
                      pLd,
                      pszComputersContainerDN,
                      pszComputersContainerName);
        if (dwError)
        {
            printf("Failed to add container %s to directory (%d)\n",
                   pszComputersContainerDN, dwError);
        }
        else
        {
            printf("Added container %s to directory.\n",
                   pszComputersContainerDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        printf("Container %s already exists, not added.\n",
               pszComputersContainerDN);
    }

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainDN);
    VMDIR_SAFE_FREE_STRINGA(pszComputersContainerDN);

    return dwError;

error:
    goto cleanup;
}

/*
 * patch cn=certificate-authorities,cn=configuration,DOMAIN_DN entry
 */
static
DWORD
AddCAContainer(
    LDAP* pLd,
    PCSTR pszServerName
    )
{
    DWORD  dwError = 0;
    PCSTR  pszCAContainerName = VMDIR_CA_CONTAINER_NAME;
    PCSTR  pszConfigurationContainerName = VMDIR_CONFIGURATION_CONTAINER_NAME;
    PSTR   pszDomainName = NULL;
    PSTR   pszDomainDN = NULL;
    PSTR   pszCAContainerDN = NULL;

    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(
                  pszServerName,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(
                  pszDomainName,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszCAContainerDN, "%s=%s,%s=%s,%s",
                  ATTR_CN,
                  pszCAContainerName,
                  ATTR_CN,
                  pszConfigurationContainerName,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VdcIfDNExist(pLd, pszCAContainerDN))
    {
        dwError = VdcLdapAddContainer(
                      pLd,
                      pszCAContainerDN,
                      pszCAContainerName);
        if (dwError)
        {
            printf("Failed to add container %s to directory (%d)\n",
                            pszCAContainerDN, dwError);
        }
        else
        {
            printf("Added container %s to directory.\n",
                            pszCAContainerDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        printf("Container %s already exists, not added.\n",
                        pszCAContainerDN);
    }

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainDN);
    VMDIR_SAFE_FREE_STRINGA(pszCAContainerDN);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
AddBuiltinDCClientsGroup(
    LDAP* pLd,
    PCSTR pszServerName
    )
{

    DWORD  dwError = 0;
    PSTR   pszDomainName = NULL;
    PSTR   pszDomainDN = NULL;
    PSTR   pszDCClientsGroupDN = NULL;

    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(
                  pszServerName,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(
                  pszDomainName,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszDCClientsGroupDN, "cn=%s,cn=%s,%s",
                  VMDIR_DCCLIENT_GROUP_NAME,
                  VMDIR_BUILTIN_CONTAINER_NAME,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VdcIfDNExist(pLd, pszDCClientsGroupDN))
    {
        dwError = VdcLdapAddGroup(
                      pLd,
                      pszDCClientsGroupDN,
                      VMDIR_DCCLIENT_GROUP_NAME);
        if (dwError)
        {
            printf("Failed to add group %s to directory (%d)\n",
                   pszDCClientsGroupDN, dwError);
        }
        else
        {
            printf("Added group %s to directory.\n",
                   pszDCClientsGroupDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        printf("Group %s already exists, not added.\n",
               pszDCClientsGroupDN);
    }

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainDN);
    VMDIR_SAFE_FREE_STRINGA(pszDCClientsGroupDN);

    return dwError;

error:
    goto cleanup;

}

static
DWORD
AddBuiltinCAAdminsGroup(
    LDAP* pLd,
    PCSTR pszServerName
    )
{
    DWORD  dwError = 0;
    PSTR   pszDomainName = NULL;
    PSTR   pszDomainDN = NULL;
    PSTR   pszCAAdminsGroupDN = NULL;
    PSTR   pszAdministratorDN = NULL;
    PSTR   pszDCAdminsGroupDN = NULL;
    PSTR   pszDCClientsGroupDN = NULL;
    PSTR   ppszVals [4] = { NULL, NULL, NULL, NULL };
    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(
                  pszServerName,
                  &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN(
                  pszDomainName,
                  &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszCAAdminsGroupDN, "cn=%s,cn=%s,%s",
                  VMDIR_CERT_GROUP_NAME,
                  VMDIR_BUILTIN_CONTAINER_NAME,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszDCAdminsGroupDN, "cn=%s,cn=%s,%s",
                  VMDIR_DC_GROUP_NAME,
                  VMDIR_BUILTIN_CONTAINER_NAME,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszDCClientsGroupDN, "cn=%s,cn=%s,%s",
                  VMDIR_DCCLIENT_GROUP_NAME,
                  VMDIR_BUILTIN_CONTAINER_NAME,
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(
                  &pszAdministratorDN, "cn=%s,cn=%s,%s",
                  "Administrator",
                  "Users",
                  pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!VdcIfDNExist(pLd, pszCAAdminsGroupDN))
    {
        dwError = VdcLdapAddGroup(
                      pLd,
                      pszCAAdminsGroupDN,
                      VMDIR_CERT_GROUP_NAME);
        if (dwError)
        {
            printf("Failed to add group %s to directory (%d)\n",
                   pszCAAdminsGroupDN, dwError);
        }
        else
        {
            printf("Added group %s to directory.\n",
                   pszCAAdminsGroupDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        ppszVals[0] = pszAdministratorDN;
        ppszVals[1] = pszDCAdminsGroupDN;
        ppszVals[2] = pszDCClientsGroupDN;

        dwError = VdcLdapAddAttributeValues(
                    pLd,
                    pszCAAdminsGroupDN,
                    ATTR_MEMBER,
                    (PCSTR*) ppszVals);
        if (dwError)
        {
            printf("Failed to add group members %s (%d)\n",
                pszCAAdminsGroupDN, dwError);
        }
        else
        {
            printf("Added group members to %s.\n",
                pszCAAdminsGroupDN);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        printf("Group %s already exists, not added.\n",
               pszCAAdminsGroupDN);
    }

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDomainDN);
    VMDIR_SAFE_FREE_STRINGA(pszCAAdminsGroupDN);
    VMDIR_SAFE_FREE_STRINGA(pszDCAdminsGroupDN);
    VMDIR_SAFE_FREE_STRINGA(pszDCClientsGroupDN);
    VMDIR_SAFE_FREE_STRINGA(pszAdministratorDN);

    return dwError;

error:
    goto cleanup;
}

static
DWORD
_UpdatePSCVersion(
    LDAP* pLd
    )
{
    DWORD  dwError = 0;
    PSTR   ppszVals [] = { VDIR_PSC_VERSION, NULL };

    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VdcLdapReplaceAttributeValues(
                pLd,
                PERSISTED_DSE_ROOT_DN,
                ATTR_PSC_VERSION,
                (PCSTR*) ppszVals);
    if (dwError)
    {
        printf("Failed to update PSC version to %s, error (%d)\n", VDIR_PSC_VERSION, dwError);
    }
    else
    {
        printf("Update PSC version to %s.\n", VDIR_PSC_VERSION);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:
    goto cleanup;
}

static
DWORD
AccountDNToName(
    PCSTR pszDCAccountDN,
    PSTR* ppszDCAccount)
{
    DWORD dwError = 0;
    PSTR pszDCAccount = NULL;
    PSTR pStart = NULL;
    PSTR pEnd = NULL;
    size_t len = 0;

    if (VmDirStringNCompareA(pszDCAccountDN, "cn=", 3, FALSE))
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    pStart = (PSTR)&pszDCAccountDN[3];
    pEnd = VmDirStringChrA(pStart, ',');
    if (!pEnd)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    len = pEnd - pStart;
    dwError = VmDirAllocateMemory(len+1, (PVOID)&pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringNCpyA(pszDCAccount, len+1, pStart, len);
    BAIL_ON_VMDIR_ERROR(dwError);

    pszDCAccount[len] = '\0';

    *ppszDCAccount = pszDCAccount;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    goto cleanup;
}

#ifndef _WIN32
static
DWORD
SetDCAccountRegistryKey(
    VOID
    )
{
    DWORD  dwError = 0;
    char   szDCAccountDN[256] = {0};
    PSTR   pszDCAccount = NULL;

    dwError = VmDirGetRegKeyValue(
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_DC_ACCOUNT_DN,
                  szDCAccountDN,
                  sizeof(szDCAccountDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AccountDNToName(
                  szDCAccountDN,
                  &pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegUtilSetValue(
                  NULL,
                  HKEY_THIS_MACHINE,
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  NULL,
                  VMDIR_REG_KEY_DC_ACCOUNT,
                  REG_SZ,
                  (PVOID)pszDCAccount,
                  VmDirStringLenA(pszDCAccount)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Set dcAccount registry key to %s\n", pszDCAccount);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    return dwError;

error:
    printf("Failed to set dcAccount registry key (%d)\n", dwError);
    goto cleanup;
}
#else
static
DWORD
SetDCAccountRegistryKey(
    VOID
    )
{
    DWORD  dwError = 0;
    char   szDCAccountDN[256] = {0};
    PSTR   pszDCAccount = NULL;
    HKEY   hKey = NULL;

    dwError = VmDirGetRegKeyValue(
                  VMDIR_CONFIG_PARAMETER_KEY_PATH,
                  VMDIR_REG_KEY_DC_ACCOUNT_DN,
                  szDCAccountDN,
                  sizeof(szDCAccountDN));
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = AccountDNToName(
                  szDCAccountDN,
                  &pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegCreateKeyExA(
                        HKEY_LOCAL_MACHINE,
                        VMDIR_CONFIG_PARAMETER_KEY_PATH,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = RegSetValueExA(
                        hKey,
                        VMDIR_REG_KEY_DC_ACCOUNT,
                        0,
                        REG_SZ,
                        (BYTE*)pszDCAccount,
                        (DWORD)VmDirStringLenA(pszDCAccount)+1);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("Set dcAccount registry key to %s\n", pszDCAccount);

cleanup:
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);

    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwError;

error:
    printf("Failed to set dcAccount registry key (%d)\n", dwError);
    goto cleanup;
}
#endif

static
DWORD
UpdateDCAccountSRPSecret(
    LDAP*   pLd
    )
{
    DWORD       dwError = 0;
    PSTR        pszSRPSecret = NULL;
    PSTR        pszUPN = NULL;
    PSTR        pszPassword = NULL;
    PSTR        pszDomain = NULL;
    PSTR        pszDCAccount = NULL;
    PSTR        pszFilter = NULL;

    dwError = VmDirRegReadDCAccount( &pszDCAccount );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf( &pszFilter,
                                             "sAMAccountName=%s",
                                             pszDCAccount);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetAttributeValue( pLd,
                                        "",
                                        LDAP_SCOPE_BASE,
                                        pszFilter,
                                        "vmwSrpSecret",
                                        &pszSRPSecret);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( pszSRPSecret == NULL )
    {
        dwError = VmDirGetDomainName( "localhost", &pszDomain );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringAVsnprintf( &pszUPN,
                                                 "%s@%s",
                                                 pszDCAccount,
                                                 pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirReadDCAccountPassword( &pszPassword );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirSetSRPSecret( pszUPN, pszPassword );
        if ( dwError == 0 )
        {
            printf("DC Account UPN(%s) SRP Secret set\n", pszUPN);
        }
        else if ( dwError == VMDIR_ERROR_ENTRY_ALREADY_EXIST )
        {
            dwError = 0;
            printf("DC Account UPN(%s) SRP Secret already exists\n", pszUPN);

        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszSRPSecret);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszPassword);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszFilter);

    return dwError;

error:
    printf("Failed to UpdateDCActSRPSecret (%d)\n", dwError);
    goto cleanup;
}

/* Replace ATTR_ACL_STRING on entries which have attribute nTSecurityDescriptor */
static
DWORD
UpdateEntriesACL(
    LDAP*   pLd,
    PCSTR pszServerName,
    PCSTR pszAdminUPN
    )
{
    DWORD       dwError = 0;
    PSTR        pszFilter = NULL;
    PSTR        pAdminSid = NULL;
    PSTR        pAclString = NULL;
    PSTR        pszDomainDn = NULL;
    int         totalCnt = 0;
    int         failedCnt = 0;

    dwError = VmDirAllocateStringAVsnprintf( &pszFilter,
                                             "%s=%s",
                                             ATTR_KRB_UPN,
                                             pszAdminUPN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetAttributeValue( pLd,
                                        "",
                                        LDAP_SCOPE_SUB,
                                        pszFilter,
                                        ATTR_OBJECT_SID,
                                        &pAdminSid);
    BAIL_ON_VMDIR_ERROR(dwError);


    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    dwError = VmDirAllocateStringAVsnprintf( &pszFilter,
                                             "%s=*",
                                             ATTR_OBJECT_CLASS);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapGetAttributeValue( pLd,
                                        "",
                                        LDAP_SCOPE_BASE,
                                        pszFilter,
                                        ATTR_ROOT_DOMAIN_NAMING_CONTEXT,
                                        &pszDomainDn);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(&pAclString,
                "O:%sG:BAD:(A;;RPWP;;;%s)(A;;GXNRNWGXCCDCRPWP;;;BA)(A;;GXNRNWGXCCDCRPWP;;;%s)",
                                            pAdminSid,
                                            VMDIR_SELF_SID,
                                            pAdminSid);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    dwError = VmDirAllocateStringAVsnprintf( &pszFilter,
                                             "%s=*",
                                             ATTR_OBJECT_SECURITY_DESCRIPTOR);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VdcLdapReplaceAttrOnEntries( pLd,
                                          pszDomainDn,
                                          LDAP_SCOPE_SUB,
                                          pszFilter,
                                          ATTR_ACL_STRING,
                                          pAclString,
                                          &totalCnt,
                                          &failedCnt);
    BAIL_ON_VMDIR_ERROR(dwError);
    if (failedCnt !=0 || totalCnt == 0)
    {
        printf("vdcupgrade Warn: %d out of %d entries failed to upgrade\n", failedCnt, totalCnt);
    } else
    {
       printf("vdcupgrade successfully update %d entries for ACL\n", totalCnt);
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pAdminSid);
    VMDIR_SAFE_FREE_MEMORY(pszFilter);
    VMDIR_SAFE_FREE_MEMORY(pAclString);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDn);
    return dwError;

error:
    printf("UpdateEntriesACL got error %d - vdcupgrade proceeds, and please upgrade ACL manually.\n", dwError);
    dwError = 0;
    goto cleanup;
}
