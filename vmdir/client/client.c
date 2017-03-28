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

#define VMDIR_RPC_FREE_MEMORY VmDirRpcClientFreeMemory

static
DWORD
_VmDirLdapCheckVmDirStatus(
    PCSTR pszPartnerHostName
    );

static
DWORD
_VmDirFindAllReplPartnerHost(
    PCSTR    pszHostName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PSTR**   pppszPartnerHost,
    DWORD*   pdwSize
    );

static
DWORD
_VmDirDeleteReplAgreementToHost(
    LDAP*    pLD,
    PCSTR    pszHost
    );

static
DWORD
_VmDirLeaveFederationOffline(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword
    );

static
DWORD
_VmDirLeaveFederationSelf(
    PCSTR pszUserName,
    PCSTR pszPassword
    );

static
BOOLEAN
_VmDirIsRemoteServerDown(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszDomain
    );

static
DWORD
_VmDirCreateServerPLD(
    PCSTR    pszServerName,
    PCSTR    pszDomain,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    LDAP**  ppLD
    );

static
DWORD
_VmDirRemoveComputer(
    LDAP *pLd,
    PCSTR pszDomainName,
    PCSTR pszComputerHostName
    );

static
DWORD
_VmDirModDcPassword(
    PCSTR pszLocalHostName,
    PCSTR pszUPN,
    PCSTR pszPassword,
    PCSTR pszMachineActDn,
    PBYTE pszNewPassword
    );

static
DWORD
_VmDirAllocateSuperLogEntryLdapOperationArray(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pSrcEntries,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppDstEntries
    );

static
DWORD
_VmDirDeleteSelfDCActInOtherDC(
    PCSTR   pszSelfDC,
    PCSTR   pszDomain,
    PCSTR   pszUserName,
    PCSTR   pszPassword);

static
DWORD
_VmDirJoinPreCondition(
    PCSTR       pszHostName,
    PCSTR       pszDomainName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PSTR*       ppszErrMsg
    );

/*
 * Refresh account password before it expires.
 */
DWORD
VmDirRefreshActPassword(
    PCSTR   pszHost,
    PCSTR   pszDomain,
    PCSTR   pszActUPN,
    PCSTR   pszActDN,
    PSTR    pszActPassword,
    PSTR*   ppszNewPassword
    )
{
    DWORD       dwError=0;
    LDAP*       pLD=NULL;
    PSTR        pszExpInDays=NULL;
    PSTR        pszLastChange=NULL;
    PSTR        pszDomainDN=NULL;
    PSTR        pszPolicyDN=NULL;
    PBYTE       pByteDCAccountPasswd=NULL;
    DWORD       dwDCAccountPasswdSize=0;
    DWORD       dwLen=0;

    int         iExpInDays=0;
    int         iCnt=0;

    if ( !pszHost || !pszDomain || !pszActUPN || !pszActDN || !pszActPassword || !ppszNewPassword )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSafeLDAPBind( &pLD,
                                 pszHost,
                                 pszActUPN,
                                 pszActPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSrvCreateDomainDN( pszDomain, &pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszPolicyDN,
                                             "cn=%s,%s",
                                             PASSWD_LOCKOUT_POLICY_DEFAULT_CN,
                                             pszDomainDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get policy expireInDays attribute
    dwError = VmDirLdapGetSingleAttribute( pLD,
                                           pszPolicyDN,
                                           ATTR_PASS_EXP_IN_DAY,
                                           (PBYTE*)&pszExpInDays,
                                           &dwLen);

    if (!dwError)
    {
        iExpInDays = atoi(pszExpInDays);
    }
    else if (dwError == LDAP_NO_SUCH_ATTRIBUTE ||
             dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
    {
        dwError = 0;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

    // iExpInDays == 0 or no such value means never expire
    if ( iExpInDays > 0 )
    {
        // get accunt last password change time
        dwError = VmDirLdapGetSingleAttribute( pLD,
                                               pszActDN,
                                               ATTR_PWD_LAST_SET,
                                               (PBYTE*)&pszLastChange,
                                               &dwLen);
        BAIL_ON_VMDIR_ERROR(dwError);

        {
            time_t  tNow = time(NULL);
            time_t  tPwdLastSet = VmDirStringToLA(pszLastChange, NULL, 10);
            time_t  tDiff = (iExpInDays * 24 * 60 * 60) / 2; // Attempt reset when halfway to expiration.

            if ( (tNow - tPwdLastSet) >= tDiff )
            {
                for (iCnt=0; iCnt < VMDIR_MAX_PASSWORD_RETRIES; iCnt++)
                {
                    VMDIR_SAFE_FREE_MEMORY(pByteDCAccountPasswd);
                    dwError = VmDirGeneratePassword( pszHost,
                                                     pszActUPN,
                                                     pszActPassword,
                                                     &pByteDCAccountPasswd,
                                                     &dwDCAccountPasswdSize );
                    BAIL_ON_VMDIR_ERROR(dwError);

                    dwError = VmDirLdapModReplaceAttribute( pLD,
                                                            pszActDN,
                                                            ATTR_USER_PASSWORD,
                                                            pByteDCAccountPasswd);
                    if (dwError == LDAP_CONSTRAINT_VIOLATION)
                    {
                        continue;
                    }
                    BAIL_ON_VMDIR_ERROR(dwError);

                    break;
                }

                if (iCnt == VMDIR_MAX_PASSWORD_RETRIES)
                {
                    dwError = LDAP_CONSTRAINT_VIOLATION;
                    BAIL_ON_VMDIR_ERROR(dwError);
                }

                *ppszNewPassword = pByteDCAccountPasswd;
                pByteDCAccountPasswd = NULL;
                VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Act (%s) password refreshed", pszActUPN);
            }
        }
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLastChange);
    VMDIR_SAFE_FREE_MEMORY(pszExpInDays);
    VMDIR_SAFE_FREE_MEMORY(pszPolicyDN);
    VMDIR_SAFE_FREE_MEMORY(pByteDCAccountPasswd);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);

    if (pLD)
    {
        ldap_unbind_ext_s(pLD, NULL, NULL);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "Reset (%s) password failed (%u)", VDIR_SAFE_STRING(pszActUPN), dwError);

    goto cleanup;
}

/*
 * Refresh account password of the node as well as all the partners
 */
DWORD
VmDirResetMachineActCred(
    PCSTR pszLocalHost,
    PCSTR pszPartnerHost,
    PCSTR pszUserName,
    PCSTR pszPassword
    )
{
    DWORD   i                  = 0;
    DWORD   dwInfoCount        = 0;
    DWORD   dwError            = 0;
    DWORD   pszNewPasswordSize = 0;

    PSTR    pszDomain          = NULL;
    PSTR    pszHostName        = NULL;
    PSTR    pszUPN             = NULL;
    PSTR    pszMachineActDn    = NULL;

    PBYTE   pszNewPassword     = NULL;

    PVMDIR_REPL_PARTNER_INFO pPartnerInfo = NULL;

    if ( !pszUserName || !pszPassword )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        if ( !pszPartnerHost )
        {
            dwError = VmDirGetDomainName(pszLocalHost,
                                         &pszDomain);
        }
        else
        {
            dwError = VmDirGetDomainName(pszPartnerHost,
                                         &pszDomain);
        }
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(&pszUPN,
                                                "%s@%s",
                                                pszUserName,
                                                pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirRegReadDCAccountDn(&pszMachineActDn);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszNewPassword);
        dwError = VmDirGeneratePassword(pszPartnerHost,
                                        pszUPN,
                                        pszPassword,
                                        &pszNewPassword,
                                        &pszNewPasswordSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( !pszPartnerHost )
        {
            //change the password in the local host machine
            dwError = _VmDirModDcPassword(pszLocalHost,
                                          pszUPN,
                                          pszPassword,
                                          pszMachineActDn,
                                          pszNewPassword);
            BAIL_ON_VMDIR_ERROR(dwError); // when the password modification failed on the local host.
        }
        else
        {
            //change the password in the partner host machine
            dwError = _VmDirModDcPassword(pszPartnerHost,
                                          pszUPN,
                                          pszPassword,
                                          pszMachineActDn,
                                          pszNewPassword);
            BAIL_ON_VMDIR_ERROR(dwError); // when the password modification failed on the partner host.

            dwError = VmDirGetReplicationPartners(pszPartnerHost,
                                                  pszUserName,
                                                  pszPassword,
                                                  &pPartnerInfo,
                                                  &dwInfoCount);
            BAIL_ON_VMDIR_ERROR(dwError);

            //change the password in all the partners of pszPartnerHost.
            for (i=0; i<dwInfoCount; i++)
            {
                dwError = VmDirLdapURI2Host(pPartnerInfo[i].pszURI,
                                            &pszHostName);
                BAIL_ON_VMDIR_ERROR(dwError);

                dwError = _VmDirModDcPassword(pszHostName,
                                              pszUPN,
                                              pszPassword,
                                              pszMachineActDn,
                                              pszNewPassword); // when the password modification failed on one of the federation of the partner host.
                VMDIR_SAFE_FREE_MEMORY(pszHostName);

                if ( dwError == VMDIR_ERROR_SERVER_DOWN ) // In case the server is down we will ignore the error and continue
                {
                    continue;
                }
                else
                {
                    BAIL_ON_VMDIR_ERROR(dwError);
                }
            }
        }

        //set the new password in the machine registry
        dwError = VmDirConfigSetDCAccountPassword(pszNewPassword,
                                                  pszNewPasswordSize);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "Update account (%s) password registry key passed.", pszMachineActDn);
    }

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszUPN);
    VMDIR_SAFE_FREE_MEMORY(pszMachineActDn);
    VMDIR_SAFE_FREE_MEMORY(pszNewPassword);
    VMDIR_SAFE_FREE_MEMORY(pszHostName);

    // Free internal memory used
    for (i=0; i<dwInfoCount; i++)
    {
        VMDIR_SAFE_FREE_MEMORY(pPartnerInfo[i].pszURI);
    }
    VMDIR_SAFE_FREE_MEMORY(pPartnerInfo);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirResetMachineActCred for account (%s), partner (%s), UPN (%s) failed (%d)", pszMachineActDn, pszPartnerHost, pszUPN, dwError);

    goto cleanup;
}

DWORD
VmDirConnectionOpen(
    PCSTR pszLdapURI,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PVMDIR_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMDIR_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pszLdapURI) ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszUsername) ||
        !pszPassword ||
        !ppConnection)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_CONNECTION),
                (PVOID*)&pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                pszDomain,
                &pConnection->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServerByURI(
                &pConnection->pLd,
                pszLdapURI,
                pConnection->pszDomain,
                pszUsername,
                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

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

DWORD
VmDirConnectionOpenByHost(
    PCSTR pszHost,
    PCSTR pszDomain,
    PCSTR pszUsername,
    PCSTR pszPassword,
    PVMDIR_CONNECTION* ppConnection
    )
{
    DWORD dwError = 0;
    PVMDIR_CONNECTION pConnection = NULL;

    if (IsNullOrEmptyString(pszHost)    ||
        IsNullOrEmptyString(pszDomain)  ||
        IsNullOrEmptyString(pszUsername)||
        !pszPassword                    ||
        !ppConnection
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_CONNECTION),
                (PVOID*)&pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(
                pszDomain,
                &pConnection->pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                &pConnection->pLd,
                pszHost,
                pConnection->pszDomain,
                pszUsername,
                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppConnection = pConnection;

cleanup:

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

LDAP*
VmDirConnectionGetLdap(
    PVMDIR_CONNECTION pConnection
    )
{
    LDAP* pLd = NULL;

    if ( pConnection )
    {
        pLd = pConnection->pLd;
    }
    return pLd;
}

DWORD
VmDirGetSiteGuid(
    PVMDIR_CONNECTION pConnection,
    PSTR*             ppszGuid
    )
{
    DWORD dwError = 0;
    PSTR  pszGuid = NULL;

    if (!pConnection || !ppszGuid)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetSiteGuidInternal(
                pConnection->pLd,
                pConnection->pszDomain,
                &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;

cleanup:

    return dwError;

error:

    if (ppszGuid)
    {
        *ppszGuid = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGuid);

    goto cleanup;
}

DWORD
VmDirGetSiteName(
    PVMDIR_CONNECTION pConnection,
    PSTR*             ppszSiteName
    )
{
    DWORD dwError = 0;
    PSTR  pszSiteName = NULL;

    if (!pConnection || !ppszSiteName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetSiteNameInternal(
                pConnection->pLd,
                &pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszSiteName = pszSiteName;

cleanup:

    return dwError;

error:

    if (ppszSiteName)
    {
        *ppszSiteName = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszSiteName);

    goto cleanup;
}

VOID
VmDirConnectionClose(
    PVMDIR_CONNECTION pConnection
    )
{
    if (pConnection)
    {
        if (pConnection->pLd)
        {
            ldap_unbind_ext_s(pConnection->pLd, NULL, NULL);
            pConnection->pLd = NULL;
        }

        VMDIR_SAFE_FREE_MEMORY(pConnection->pszDomain);
        pConnection->pszDomain = NULL;

        VMDIR_SAFE_FREE_MEMORY(pConnection);
    }
}

PSTR
VmDirGetTopDomain(
    PSTR pszDomain
)
{
    PSTR pszTopDomain = NULL;
    if (!IsNullOrEmptyString(pszDomain))
    {
        if ((pszTopDomain = VmDirStringRChrA(pszDomain, '.')) != NULL)
        {
            pszTopDomain++; // skip '.'
        }
        else
        {
            pszTopDomain = pszDomain;
        }
    }
    return pszTopDomain;
}

static
DWORD
VmDirSetupHostInstanceEx(
    PCSTR                       pszDomainName,
    PCSTR                       pszUsername,
    PCSTR                       pszPassword,
    PCSTR                       pszSiteName,
    PCSTR                       pszPartnerHostName,
    VMDIR_FIRST_REPL_CYCLE_MODE firstReplCycleMode
    )
{
    DWORD dwError = 0;

    PSTR pszReplURI = NULL;
    PSTR pszDomainDN = NULL;

    PWSTR pwszDomainname = NULL;
    PWSTR pwszUsername = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszSiteName = NULL;
    PWSTR pwszReplURI = NULL;

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword))
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!IsNullOrEmptyString(pszPartnerHostName))
    {
        dwError = VmDirSrvCreateDomainDN(pszDomainName, &pszDomainDN);
        BAIL_ON_VMDIR_ERROR(dwError);

        if ( VmDirIsIPV6AddrFormat( pszPartnerHostName ) )
        {
            dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://[%s]",
                                                     VMDIR_LDAP_PROTOCOL,
                                                     pszPartnerHostName);
        }
        else
        {
            dwError = VmDirAllocateStringPrintf( &pszReplURI, "%s://%s",
                                                     VMDIR_LDAP_PROTOCOL,
                                                     pszPartnerHostName);
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(pszDomainName, &pwszDomainname);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(pszUsername, &pwszUsername);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (pszSiteName)
    {
        dwError = VmDirAllocateStringWFromA(pszSiteName, &pwszSiteName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (pszReplURI)
    {
        dwError = VmDirAllocateStringWFromA(pszReplURI, &pwszReplURI);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirLocalInitializeHost(
                    pwszDomainname,
                    pwszUsername,
                    pwszPassword,
                    pwszSiteName,
                    pwszReplURI,
                    firstReplCycleMode);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirLocalInitializeHost (%s)(%s)(%s) passed",
                    pszDomainName, VDIR_SAFE_STRING(pszSiteName), VDIR_SAFE_STRING(pszPartnerHostName));

    dwError = _VmDirLdapCheckVmDirStatus(pszPartnerHostName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Vmdir instance ready for LDAP service");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszReplURI);
    VMDIR_SAFE_FREE_MEMORY(pszDomainDN);
    VMDIR_SAFE_FREE_MEMORY(pwszDomainname);

    VMDIR_SAFE_FREE_MEMORY(pwszUsername);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);
    VMDIR_SAFE_FREE_MEMORY(pwszSiteName);
    VMDIR_SAFE_FREE_MEMORY(pwszReplURI);

    return dwError;
error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetupHostInstanceEx failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirSetupHostInstance(
    PCSTR    pszDomainName,
    PCSTR    pszLotusServerName, // optional lotus server name.
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszSiteName
    )
{
    DWORD   dwError = 0;
    UCHAR   pszDCAccountPassword[VMDIR_KDC_RANDOM_PWD_LEN+1] = {0};
    PSTR    pszLotusServerNameCanon = NULL;
    int     err = 0;
    int     i = 0;

    // Generate an initial DC account password and store it in the registry.

    err = RAND_pseudo_bytes(pszDCAccountPassword, VMDIR_KDC_RANDOM_PWD_LEN);
    for (i=0; i<VMDIR_KDC_RANDOM_PWD_LEN; i++)
    {
        pszDCAccountPassword[i] &= 0x7f;
        if (pszDCAccountPassword[i] < 0x30)
        {
            pszDCAccountPassword[i] += 0x30;
        }
    }
    pszDCAccountPassword[VMDIR_KDC_RANDOM_PWD_LEN] = '\0';

    dwError = VmDirConfigSetDCAccountPassword(
                        pszDCAccountPassword,
                        (DWORD) VmDirStringLenA(pszDCAccountPassword));
    BAIL_ON_VMDIR_ERROR(dwError);

    // Determine the name of lotus server
    dwError = VmDirGetLotusServerName( pszLotusServerName ? pszLotusServerName : "localhost",
                                       &pszLotusServerNameCanon );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set registry key DCAccount.  So in server RPC VmDirSrvInitializeHost, it can read and use DCAccount.
    dwError = VmDirConfigSetSZKey( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszLotusServerNameCanon);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError =  VmDirSetupHostInstanceEx(
                        pszDomainName,
                        pszUserName,
                        pszPassword,
                        pszSiteName,
                        NULL,
                        FIRST_REPL_CYCLE_MODE_OBJECT_BY_OBJECT);
    BAIL_ON_VMDIR_ERROR(dwError);

    // This task must be performed after VmDirSetupHostInstanceEx(), because the server does not start listening on
    // LDAP ports till SetupHostInstance is done.
    dwError = VmDirSetupDefaultAccount(
                        pszDomainName,
                        pszLotusServerNameCanon,    // Partner is self in this 1st instance case.
                        pszLotusServerNameCanon,
                        pszUserName,
                        pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUpdateKeytabFile(
                        pszLotusServerNameCanon,
                        pszDomainName,
                        pszLotusServerNameCanon,
                        pszUserName,
                        pszPassword,
                        TRUE );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSetupHostInstance (%s)(%s)(%s) passed",
                                        VDIR_SAFE_STRING(pszDomainName),
                                        VDIR_SAFE_STRING(pszSiteName),
                                        VDIR_SAFE_STRING(pszLotusServerNameCanon) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszLotusServerNameCanon );
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetupHostInstance failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * API to demote local DC.
 * Local DC must be in running state.
 *
 * 1. put local DC into read only mode
 * 2. try to clean up local DC data at one of its available partner
 * 3. stop vmdir
 * 4. remove vmdir database
 * 5. start vmdir into uninitialized state, i.e. waiting for promotion call.
 */
DWORD
VmDirDemote(
    PCSTR    pszUserName,
    PCSTR    pszPassword
    )
{
    DWORD   dwError = 0;

    // admin privileges is required to call VmDirSetState.
    dwError = VmDirSetState( NULL, VMDIRD_STATE_READ_ONLY_DEMOTE );
    BAIL_ON_VMDIR_ERROR( dwError );
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Vmdir in read only mode" );

    // let partner caughtup with me, then cleanup my node specific entries.
    dwError = VmDirLeaveFederation( NULL, (PSTR)pszUserName, (PSTR)pszPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    // destroy default cred cache.
    (VOID) VmDirDestroyDefaultKRB5CC();

    // This stop vmdir, destroy db then start vmdir in uninitialized state.
    dwError = VmDirResetVmdir();
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirDemote succeeded." );

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirDemote failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirJoin(
    PCSTR    pszLotusServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszSiteName,
    PCSTR    pszPartnerHostName,
    UINT32   firstReplCycleMode
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = 0;
    PCSTR   pszTopDomain = NULL;
    PSTR    pszPartnerServerName = NULL;
    PSTR    pszLotusServerNameCanon = NULL;
    PSTR    pszCurrentServerObjectDN = NULL;
    PSTR    pszErrMsg = NULL;
    DWORD   dwHighWatermark = 0;
    LDAP*   pLd = NULL;
    PVMDIR_REPL_STATE pReplState = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszPartnerHostName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Determine the name of lotus server
    dwError = VmDirGetLotusServerName( pszLotusServerName ? pszLotusServerName : "localhost",
                                       &pszLotusServerNameCanon );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Set registry key DCAccount.  So in server RPC VmDirSrvInitializeHost, it can read and use DCAccount.
    dwError = VmDirConfigSetSZKey( VMDIR_CONFIG_PARAMETER_KEY_PATH,
                                   VMDIR_REG_KEY_DC_ACCOUNT,
                                   pszLotusServerNameCanon);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("System Host Name: %s\n", pszLotusServerNameCanon);

    dwError = VmDirGetServerName( pszPartnerHostName, &pszPartnerServerName);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("Partner Server Name: %s\n", pszPartnerServerName);

    dwError = VmDirGetDomainName( pszPartnerServerName, &pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("Target Domain Name: %s\n", pszDomainName);

    if ((pszTopDomain = VmDirStringRChrA(pszDomainName, '.')) != NULL)
    {
        pszTopDomain++; // skip '.'
    }
    else
    {
        pszTopDomain = pszDomainName;
    }

    // make sure there is NO server object with same name in the federation
    dwError = VmDirGetServerObjectDN(
                                pszPartnerServerName,
                                pszDomainName,
                                pszUserName,
                                pszPassword,
                                pszLotusServerNameCanon,
                                &pszCurrentServerObjectDN);
    BAIL_ON_VMDIR_ERROR(dwError);
    if ( pszCurrentServerObjectDN )
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s, server object (%s) exists already, DN (%s).",
                                             __FUNCTION__,
                                             VDIR_SAFE_STRING(pszLotusServerNameCanon),
                                             pszCurrentServerObjectDN );
        dwError = VMDIR_ERROR_ENTRY_ALREADY_EXIST;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // Make sure we can join the partner
    dwError = _VmDirJoinPreCondition(
                                pszPartnerServerName,
                                pszDomainName,
                                pszUserName,
                                pszPassword,
                                &pszErrMsg);
    BAIL_ON_VMDIR_ERROR(dwError);

    // This task must be performed before VmDirSetupHostInstanceEx(), because the accounts setup in this task,
    // on the partner machine, are used by the replication triggered by VmDirSetupHostInstanceEx()
    // IMPORTANT: In general, the following sequence of operations should be strictly kept like this, otherwise SASL
    // binds in replication may break.

    dwError = VmDirSetupDefaultAccount(
                                pszDomainName,
                                pszPartnerServerName,       // remote lotus server FQDN/IP
                                pszLotusServerNameCanon,    // local lotus name
                                pszUserName,
                                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSetupHostInstanceEx(
                                 pszDomainName,
                                 pszUserName,
                                 pszPassword,
                                 pszSiteName,
                                 pszPartnerServerName,
                                 firstReplCycleMode );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirUpdateKeytabFile(
                                pszLotusServerNameCanon,
                                pszDomainName,
                                pszLotusServerNameCanon,
                                pszUserName,
                                pszPassword,
                                TRUE );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Error intentionally ignored
    dwError = VmDirLdapRemoveRemoteHostRA(
                                    pszDomainName,
                                    pszPartnerServerName,
                                    pszUserName,    /* we use same username and password */
                                    pszPassword,
                                    pszLotusServerNameCanon );

    // If db copy, use the local usn as the highwater mark for the partner's RA
    if (firstReplCycleMode == FIRST_REPL_CYCLE_MODE_COPY_DB)
    {
        dwError = _VmDirCreateServerPLD(
                                pszLotusServerNameCanon,
                                pszDomainName,
                                pszUserName,
                                pszPassword,
                                &pLd);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetReplicationStateInternal(pLd, &pReplState);
        BAIL_ON_VMDIR_ERROR(dwError);

        // Buffer the highwater mark but just not down to zero
        dwHighWatermark= (DWORD)VMDIR_MAX(pReplState->maxVisibleUSN - HIGHWATER_USN_REPL_BUFFER, 1);
    }

    dwError = VmDirLdapSetupRemoteHostRA(
                                    pszDomainName,
                                    pszPartnerServerName,
                                    pszUserName,    /* we use same username and password */
                                    pszPassword,
                                    pszLotusServerNameCanon,
                                    dwHighWatermark);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "VmDirJoin (%s)(%s)(%s) passed",
                    VDIR_SAFE_STRING(pszPartnerHostName),
                    VDIR_SAFE_STRING(pszSiteName),
                    VDIR_SAFE_STRING(pszLotusServerNameCanon) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszPartnerServerName);
    VMDIR_SAFE_FREE_MEMORY(pszLotusServerNameCanon);
    VMDIR_SAFE_FREE_MEMORY(pszCurrentServerObjectDN);
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    VmDirFreeReplicationStateInternal(pReplState);
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirJoin (%s)(%s)(%s)(%s) failed. Error(%u)",
                                         VDIR_SAFE_STRING(pszPartnerHostName),
                                         VDIR_SAFE_STRING(pszSiteName),
                                         VDIR_SAFE_STRING(pszLotusServerNameCanon),
                                         VDIR_SAFE_STRING(pszErrMsg),
                                         dwError);
    goto cleanup;
}

DWORD
VmDirClientJoin(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszMachineName,
    PCSTR    pszOrgUnit,
    DWORD    dwFlags)
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    PCSTR   pszServiceTable[] = VMDIR_CLIENT_SERVICE_PRINCIPAL_INITIALIZER;
    int     iCnt = 0;
    LDAP*   pLd = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszMachineName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(pszServerName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                &pLd,
                pszServerName,
                pszDomainName,
                pszUserName,
                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwFlags & VMDIR_CLIENT_JOIN_FLAGS_PREJOINED)
    {
        dwError = VmDirLdapConfigureComputerAccount(
                    pLd,
                    pszDomainName,
                    pszPassword,
                    pszMachineName);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirLdapCreateComputerOUContainer(
                    pLd,
                    pszDomainName,
                    pszOrgUnit ? pszOrgUnit : VMDIR_COMPUTERS_RDN_VAL);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapSetupComputerAccount(
                    pLd,
                    pszDomainName,
                    pszServerName,
                    pszUserName,
                    pszPassword,
                    pszOrgUnit ? pszOrgUnit : VMDIR_COMPUTERS_RDN_VAL,
                    pszMachineName,
                    TRUE,
                    NULL,
                    NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (iCnt = 0; iCnt < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); iCnt++)
        {
            dwError = VmDirLdapSetupServiceAccount(
                            pLd,
                            pszDomainName,
                            pszServerName,
                            pszUserName,
                            pszPassword,
                            pszServiceTable[iCnt],
                            pszMachineName );
            if (dwError == LDAP_ALREADY_EXISTS)
            {
                dwError = LDAP_SUCCESS; // ignore if entry already exists (maybe due to prior client join)
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirSetupServiceAccount (%s) return LDAP_ALREADY_EXISTS",
                                                        pszServiceTable[iCnt] );
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirUpdateKeytabFile(
                pszServerName,
                pszDomainName,
                pszMachineName,
                pszUserName,
                pszPassword,
                FALSE);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s (%s)(%s)(%s) passed",
                                        __FUNCTION__,
                                        VDIR_SAFE_STRING(pszServerName),
                                        VDIR_SAFE_STRING(pszUserName),
                                        VDIR_SAFE_STRING(pszMachineName) );

cleanup:

    VmDirLdapUnbind(&pLd);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirClientJoin failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmDirClientLeave(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword
    )
{
    DWORD   dwError = 0;
    PSTR    pszDomainName = NULL;
    PSTR    pszDCAccount = NULL;
    PSTR    pszDCAccountPass = NULL;

    PCSTR   pszUser = NULL;
    PCSTR   pszPass = NULL;
    PCSTR   pszMachine = NULL;


    if ((!pszUserName &&  pszPassword) ||
        ( pszUserName && !pszPassword))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (IsNullOrEmptyString(pszServerName))
    {
        pszServerName = "localhost";
    }

    dwError = VmDirGetDomainName( pszServerName, &pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirRegReadDCAccount( &pszDCAccount );
    BAIL_ON_VMDIR_ERROR(dwError);

    pszMachine = pszDCAccount;

    if (IsNullOrEmptyString(pszUserName))
    {
        pszUser = pszDCAccount;

        dwError = VmDirReadDCAccountPassword(&pszDCAccountPass);
        BAIL_ON_VMDIR_ERROR(dwError);

        pszPass = pszDCAccountPass;
    }
    else
    {
        pszUser = pszUserName;
        pszPass = pszPassword;
    }

    dwError = VmDirLdapRemoveComputerAccount(
                      pszDomainName,
                      pszServerName,
                      pszUser,
                      pszPass,
                      pszMachine);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s (%s)(%s) passed",
                                        __FUNCTION__,
                                        VDIR_SAFE_STRING(pszServerName),
                                        VDIR_SAFE_STRING(pszUserName) );

cleanup:

    VMDIR_SAFE_FREE_STRINGA(pszDomainName);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccount);
    VMDIR_SAFE_FREE_STRINGA(pszDCAccountPass);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirClientLeave failed. Error(%u)", dwError);

    goto cleanup;
}

DWORD
VmDirCreateComputerAccount(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszMachineName,
    PCSTR pszOrgUnit,
    PSTR* ppszOutPassword)
{
    DWORD dwError = 0;
    PSTR  pszDomainName = NULL;
    LDAP* pLd = NULL;
    PBYTE pByteOutPassword = NULL;
    DWORD dwOutPasswordSize = 0;
    PSTR pszOutPassword = NULL;

    if (IsNullOrEmptyString(pszServerName) ||
        IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszMachineName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetDomainName(pszServerName, &pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                &pLd,
                pszServerName,
                pszDomainName,
                pszUserName,
                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapCreateComputerOUContainer(
                pLd,
                pszDomainName,
                pszOrgUnit ? pszOrgUnit : VMDIR_COMPUTERS_RDN_VAL);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSetupComputerAccount(
                pLd,
                pszDomainName,
                pszServerName,
                pszUserName,
                pszPassword,
                pszOrgUnit ? pszOrgUnit : VMDIR_COMPUTERS_RDN_VAL,
                pszMachineName,
                FALSE,
                &pByteOutPassword,
                &dwOutPasswordSize);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateAndCopyMemory(
                pByteOutPassword,
                dwOutPasswordSize + 1,
                (PVOID*)&pszOutPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

   if (ppszOutPassword)
   {
       *ppszOutPassword = pszOutPassword;
       pszOutPassword = NULL;
   }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "%s (%s)(%s)(%s) passed",
                                        __FUNCTION__,
                                        VDIR_SAFE_STRING(pszServerName),
                                        VDIR_SAFE_STRING(pszUserName),
                                        VDIR_SAFE_STRING(pszMachineName) );

cleanup:

    VmDirLdapUnbind(&pLd);
    VMDIR_SAFE_FREE_MEMORY(pByteOutPassword);
    VMDIR_SAFE_FREE_STRINGA(pszOutPassword);
    VMDIR_SAFE_FREE_STRINGA(pszDomainName);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirCreateComputerAccount failed. Error(%u)", dwError);

    goto cleanup;
}

/*
 * In order for a node to leave federation, we need to clean up -
 * 1. Make sure at least one partner is up-to-date with respect to us
 *    Wait until the first partner has caughtup with my changes.
 *    Max wait time is 2.5 mins.
 *
 * 2. All replication agreements that have this node as partner
 *
 * 3. ServerObject sub tree of this node.
 *
 * 4. Its domain controller object on all partners.
 *
 */
static
DWORD
_VmDirLeaveFederationSelf(
    PCSTR pszUserName,
    PCSTR pszPassword
    )
{
    DWORD       dwError=0;
    PSTR        pszServerDN=NULL;
    PSTR        pszDomain=NULL;
    PSTR        pszDCAccount=NULL;
    PSTR*       ppszPartnerHosts=NULL;
    DWORD       dwNumHost=0;
    DWORD       dwCnt=0;
    LDAP*       pLD=NULL;
    PCSTR       pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;
    int         iCnt = 0;
    BOOLEAN     bUpToDate = FALSE;
    time_t      startTime = 0;
    DWORD       dwPartnerConnected = 0;
    BOOLEAN     bFinishedOnePartner = FALSE;
    BOOLEAN     bWaitForPartner = TRUE;
    LDAP**      ppLDArray = NULL;

    dwError = VmDirRegReadDCAccount( &pszDCAccount );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName( pszDCAccount, &pszDomain );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerDN( pszDCAccount, &pszServerDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // find hosts with myself as the partner
    dwError = _VmDirFindAllReplPartnerHost(pszDCAccount, pszUserName, pszPassword,  &ppszPartnerHosts, &dwNumHost );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwNumHost > 0)
    {
        dwError = VmDirAllocateMemory(sizeof(ppLDArray)*(dwNumHost), (PVOID*)&ppLDArray);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
       goto cleanup;
    }

    // connect to reachable partners
    for (dwCnt=0; dwCnt < dwNumHost; dwCnt++)
    {
        pLD = NULL;
        dwError = _VmDirCreateServerPLD(ppszPartnerHosts[dwCnt], pszDomain, pszUserName, pszPassword, &pLD );
        if (dwError == 0)
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner (%s) connected", ppszPartnerHosts[dwCnt] );
            ppLDArray[dwCnt] = pLD;
            dwPartnerConnected++;
        }
        else
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner (%s) not available (%u)", ppszPartnerHosts[dwCnt], dwError );
            dwError=0;
        }
    }

    // best effort to wait for one partner to pick up all my changes
    for (dwCnt=0; !bUpToDate && dwCnt < dwNumHost; dwCnt++)
    {
        if (ppLDArray[dwCnt] == NULL)
        {
            continue;
        }

        startTime = time(NULL);
        do
        {
            dwError = VmDirIsPartnerReplicationUpToDate(
                                        ppLDArray[dwCnt],
                                        ppszPartnerHosts[dwCnt],
                                        pszDomain,
                                        pszDCAccount,
                                        pszUserName,
                                        pszPassword,
                                        &bUpToDate);
            if (dwError || !bUpToDate)
            {
               VmDirSleep(1000);
            }

        } while (bUpToDate == FALSE &&
                 bWaitForPartner    &&
                 time(NULL) - startTime < 2.5 * SECONDS_IN_MINUTE); // default idle pLD idle timeout is 3 mins, so set max wait to 2.5 mins.

        bWaitForPartner = FALSE;

        if (!bUpToDate)
        {
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner (%s) is not up to date", ppszPartnerHosts[dwCnt]);
        }
        else
        {
            VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner (%s) is up to date", ppszPartnerHosts[dwCnt]);
        }
    }

    // clean up node specific entires in one partner
    for (dwCnt=0; dwCnt < dwNumHost; dwCnt++)
    {
        if (ppLDArray[dwCnt] == NULL)
        {
            continue;
        }

        for (iCnt = 0; iCnt < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); iCnt++)
        {
            dwError = VmDirLdapDeleteServiceAccount(
                                        ppLDArray[dwCnt],
                                        pszDomain,
                                        pszServiceTable[iCnt],
                                        pszDCAccount,
                                        TRUE);
            // TODO, we should have an RPC api to get server state.
            // first partner write operaton, hanlde partner in read only mode scenario.
            if (dwError == LDAP_UNWILLING_TO_PERFORM)
            {
                VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner (%s) is in read only mode", ppszPartnerHosts[dwCnt]);
                break;
            }
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        if (dwError == LDAP_UNWILLING_TO_PERFORM)
        {
            dwError = 0;
            continue; // try next partner
        }

        dwError = _VmDirDeleteReplAgreementToHost( ppLDArray[dwCnt], pszDCAccount );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirDeleteDITSubtree( ppLDArray[dwCnt], pszServerDN );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirLdapDeleteDCAccount( ppLDArray[dwCnt], pszDomain, pszDCAccount, TRUE, TRUE);
        BAIL_ON_VMDIR_ERROR(dwError);

        bFinishedOnePartner = TRUE;
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, clean up done on partner (%s)", ppszPartnerHosts[dwCnt]);
        break;
    }

    if (bFinishedOnePartner)
    {
        // ignore error, best effort to delete pszDCAccount on all other DC.
        // this helps VC repointing race condition where deleted act has not yet been replicated and
        // causes constraint violcation.
        _VmDirDeleteSelfDCActInOtherDC(pszDCAccount, pszDomain, pszUserName, pszPassword);
    }

    if (dwPartnerConnected == 0)
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, no partner available" );
    }
    else if (!bFinishedOnePartner)
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, clean up effort is NOT successful" );
    }
    else if (!bUpToDate)
    {
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf, partner may not have my up to date changes" );
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf passed");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszServerDN);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    for (dwCnt=0; dwCnt < dwNumHost; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY( ppszPartnerHosts[dwCnt] );
        if (ppLDArray[dwCnt])
        {
            ldap_unbind_ext_s(ppLDArray[dwCnt], NULL, NULL);
        }
    }
    VMDIR_SAFE_FREE_MEMORY( ppszPartnerHosts );
    VMDIR_SAFE_FREE_MEMORY( ppLDArray );

    return dwError;

error:
    VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationSelf failed (%u)", dwError );

    goto cleanup;
}

/*
 * Best effort to delete selfDC account in all other DCs.
 */
static
DWORD
_VmDirDeleteSelfDCActInOtherDC(
    PCSTR   pszSelfDC,
    PCSTR   pszDomain,
    PCSTR   pszUserName,
    PCSTR   pszPassword)
{
    DWORD   dwError = 0;
    LDAP*   pLD = NULL;
    DWORD   dwCnt = 0;
    PVMDIR_STRING_LIST  pDCStrList = NULL;

    dwError = _VmDirCreateServerPLD(pszSelfDC, pszDomain, pszUserName, pszPassword, &pLD );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetAllDCInternal(pLD, pszDomain, &pDCStrList);
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt<pDCStrList->dwCount; dwCnt++)
    {
        if (pLD)
        {
            ldap_unbind_ext_s(pLD, NULL, NULL);
            pLD = NULL;
        }

        if (VmDirStringCompareA(pszSelfDC, pDCStrList->pStringList[dwCnt],FALSE) == 0)
        {   // no need to delete self
            continue;
        }

        // ignore error
        dwError = _VmDirCreateServerPLD(pDCStrList->pStringList[dwCnt], pszDomain, pszUserName, pszPassword, &pLD );
        if (dwError == 0)
        {   // ignore error
            VmDirLdapDeleteDCAccount( pLD, pszDomain, pszSelfDC, TRUE, TRUE);
        }
    }

cleanup:
    if (pLD)
    {
        ldap_unbind_ext_s(pLD, NULL, NULL);
    }
    VmDirStringListFree(pDCStrList);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirSetupTenantInstance(
    PCSTR pszDomainName,
    PCSTR pszUsername,
    PCSTR pszPassword
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomainname = NULL;
    PWSTR pwszUsername = NULL;
    PWSTR pwszPassword = NULL;

    if (IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszUsername) ||
        IsNullOrEmptyString(pszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA(pszDomainName, &pwszDomainname);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(pszUsername, &pwszUsername);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(pszPassword, &pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLocalInitializeTenant(
                    pwszDomainname,
                    pwszUsername,
                    pwszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSetupTenantInstance (%s) passed", pszDomainName );

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pwszDomainname);
    VMDIR_SAFE_FREE_MEMORY(pwszUsername);
    VMDIR_SAFE_FREE_MEMORY(pwszPassword);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetupTenantInstance failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirCreateTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pszDomainName,
    PCSTR pszNewUserName,
    PCSTR pszNewUserPassword
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszUserUPN) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDomainName) ||
        IsNullOrEmptyString(pszNewUserName) ||
        IsNullOrEmptyString(pszNewUserPassword))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLocalCreateTenant(
                pszUserUPN,
                pszPassword,
                pszDomainName,
                pszNewUserName,
                pszNewUserPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;
error:
    goto cleanup;
}

DWORD
VmDirDeleteTenant(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PCSTR pszDomainName
    )
{
    DWORD dwError = 0;

    if (IsNullOrEmptyString(pszUserUPN) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszDomainName))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLocalDeleteTenant(pszUserUPN, pszPassword, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO(VMDIR_LOG_MASK_ALL, "VmDirDeleteTenant (%s) passed", pszDomainName);

cleanup:
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirDeleteTenant failed. Error(%u)", dwError);
    goto cleanup;
}

DWORD
VmDirEnumerateTenants(
    PCSTR pszUserUPN,
    PCSTR pszPassword,
    PSTR **pppszTenants,
    DWORD *pdwNumTenants
    )
{
    DWORD dwError = 0;
    DWORD dwNumTenants = 0;
    PSTR *ppszTenants = NULL;

    if (IsNullOrEmptyString(pszUserUPN) ||
        IsNullOrEmptyString(pszPassword) ||
        pppszTenants == NULL ||
        pdwNumTenants == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirLocalEnumerateTenants(
                pszUserUPN,
                pszPassword,
                &ppszTenants,
                &dwNumTenants);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmDirEnumerateTenants passed");

    *pppszTenants = ppszTenants;
    *pdwNumTenants = dwNumTenants;

cleanup:
    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirEnumerateTenants failed. Error(%u)", dwError);
    goto cleanup;
}

/*
 * IPC call, needs root privileges.
 */
DWORD
VmDirForceResetPassword(
    PCSTR       pszTargetUPN,
    PBYTE*      ppByte,
    DWORD*      pSize
    )
{
    DWORD                   dwError = 0;
    VMDIR_DATA_CONTAINER    dataContainer = {0};
    PBYTE                   pLocalByte = NULL;
    PWSTR                   pwszTargetUPN = NULL;

    if ( !pszTargetUPN || !ppByte || !pSize )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA( pszTargetUPN, &pwszTargetUPN );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLocalForceResetPassword(
                    pwszTargetUPN,
                    &dataContainer
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                    dataContainer.dwCount,
                    (PVOID*)&pLocalByte
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory (
                    pLocalByte,
                    dataContainer.dwCount,
                    dataContainer.data,
                    dataContainer.dwCount
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppByte     = pLocalByte;
    *pSize      = dataContainer.dwCount;
    pLocalByte  = NULL;

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirForceResetPassword (UPN=%s) passed", pszTargetUPN );

cleanup:

    VMDIR_SAFE_FREE_MEMORY( pwszTargetUPN );
    VMDIR_RPC_FREE_MEMORY( dataContainer.data );

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirForceResetPassword (UPN=%s) failed (%u)",
                     VDIR_SAFE_STRING(pszTargetUPN), dwError );

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);

    goto cleanup;
}

/*
 * *ppByte will be null terminated
 */
DWORD
VmDirGeneratePassword(
    PCSTR       pszServerName,
    PCSTR       pszSRPUPN,
    PCSTR       pszSRPPassword,
    PBYTE*      ppByte,
    DWORD*      pSize
    )
{
    DWORD                   dwError = 0;
    PCSTR                   pszServerEndpoint = NULL;
    handle_t                hBinding = NULL;
    VMDIR_DATA_CONTAINER    dataContainer = {0};
    PBYTE                   pLocalByte = NULL;

    if ( !ppByte || !pSize )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!pszServerName || VmDirIsLocalHost(pszServerName))
    {
        dwError = VmDirLocalGeneratePassword(
                        &dataContainer);
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppByte     = dataContainer.data;
        *pSize      = dataContainer.dwCount;

        dataContainer.data = NULL;
    }
    else
    {
        dwError = VmDirCreateBindingHandleAuthA(
                        pszServerName,
                        pszServerEndpoint,
                        pszSRPUPN,
                        NULL,
                        pszSRPPassword,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_RPC_TRY
        {
            dwError = RpcVmDirGeneratePassword(
                            hBinding,
                            &dataContainer
                            );
        }
        VMDIR_RPC_CATCH
        {
            VMDIR_RPC_GETERROR_CODE(dwError);
        }
        VMDIR_RPC_ENDTRY;
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(
                        dataContainer.dwCount + 1,  // pLocalByte ends with '\0'
                        (PVOID*)&pLocalByte
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirCopyMemory (
                        pLocalByte,
                        dataContainer.dwCount,
                        dataContainer.data,
                        dataContainer.dwCount
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        *ppByte     = pLocalByte;
        *pSize      = dataContainer.dwCount;
        pLocalByte  = NULL;
    }

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    VMDIR_RPC_FREE_MEMORY( dataContainer.data );

    return dwError;

error:

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGeneratePassword failed (%u)", dwError );

    goto cleanup;
}

static
DWORD
_VmDirGetKeyTabRecBlob(
    PCSTR       pszServerName,
    PCSTR       pszSRPUPN,
    PCSTR       pszSRPPassword,
    PCSTR       pszUPN,
    PBYTE*      ppByte,
    DWORD*      pSize
    )
{
    DWORD                   dwError = 0;
    PCSTR                   pszServerEndpoint = NULL;
    PWSTR                   pwszUPN = NULL;
    handle_t                hBinding = NULL;
    VMDIR_DATA_CONTAINER    dataContainer = {0};
    PBYTE                   pLocalByte = NULL;

    if (IsNullOrEmptyString(pszUPN) || !ppByte || !pSize)
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleAuthA(
                    pszServerName,
                    pszServerEndpoint,
                    pszSRPUPN,
                    NULL,
                    pszSRPPassword,
                    &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                        pszUPN,
                        &pwszUPN
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetKeyTabRecBlob(
                        hBinding,
                        pwszUPN,
                        &dataContainer
                        );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                    dataContainer.dwCount,
                    (PVOID*)&pLocalByte
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory (
                    pLocalByte,
                    dataContainer.dwCount,
                    dataContainer.data,
                    dataContainer.dwCount
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppByte     = pLocalByte;
    *pSize      = dataContainer.dwCount;
    pLocalByte  = NULL;

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    VMDIR_SAFE_FREE_MEMORY(pwszUPN);
    VMDIR_SAFE_FREE_MEMORY(pLocalByte);
    VMDIR_RPC_FREE_MEMORY( dataContainer.data );

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetKeyTabRecBlob (UPN=%s) failed (%u)",
                     VDIR_SAFE_STRING(pszUPN), dwError );
    goto cleanup;
}

DWORD
VmDirGetKrbMasterKey(
    PSTR        pszDomainName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    )
{
    DWORD       dwError = 0;
    PCSTR       pszServerName = "localhost";
    PCSTR       pszServerEndpoint = NULL;
    PWSTR       pwszDomainname = NULL;
    handle_t    hBinding = NULL;
    PBYTE       pLocalByte = NULL;
    VMDIR_DATA_CONTAINER    dataContainer = {0};

    if (IsNullOrEmptyString(pszDomainName)  || !ppKeyBlob || !pSize )
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleMachineAccountA(
                    pszServerName,
                    pszServerEndpoint,
                    &hBinding
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                        pszDomainName,
                        &pwszDomainname
                        );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetKrbMasterKey(
                        hBinding,
                        pwszDomainname,
                        &dataContainer
                        );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(dataContainer.dwCount, (PVOID*)&pLocalByte );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory( pLocalByte,
                               dataContainer.dwCount,
                               dataContainer.data,
                               dataContainer.dwCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppKeyBlob = pLocalByte;
    *pSize     = dataContainer.dwCount;
    pLocalByte = NULL;

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    VMDIR_SAFE_FREE_MEMORY(pwszDomainname);
    VMDIR_RPC_FREE_MEMORY(dataContainer.data);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetKrbMasterKey failed (%u)", dwError );

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);

    goto cleanup;
}

/*
 * IPC call, needs root privileges.
 */
DWORD
VmDirSetSRPSecret(
    PCSTR       pszUPN,
    PCSTR       pszSecret
    )
{
    DWORD       dwError = 0;
    PWSTR       pwszUPN = NULL;
    PWSTR       pwszSecret = NULL;

    if (IsNullOrEmptyString(pszUPN) || IsNullOrEmptyString(pszSecret))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA( pszUPN, &pwszUPN );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA( pszSecret, &pwszSecret );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLocalSetSRPSecret(
                    pwszUPN,
                    pwszSecret
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pwszUPN);
    VMDIR_SAFE_FREE_MEMORY(pwszSecret);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetSRPSecret failed (%u)(%s)",
                     dwError, VDIR_SAFE_STRING(pszUPN) );

    goto cleanup;
}

DWORD
VmDirGetKrbUPNKey(
    PSTR        pszUpnName,
    PBYTE*      ppKeyBlob,
    DWORD*      pSize
    )
{
    DWORD       dwError = 0;
    PCSTR       pszServerName = "localhost";
    PCSTR       pszServerEndpoint = NULL;
    PWSTR       pwszUpnname = NULL;
    handle_t    hBinding = NULL;
    PBYTE       pLocalByte = NULL;
    VMDIR_DATA_CONTAINER    dataContainer = {0};

    if (IsNullOrEmptyString(pszUpnName) || !ppKeyBlob || !pSize )
    {
        dwError =  ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleMachineAccountA(
                    pszServerName,
                    pszServerEndpoint,
                    &hBinding
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringWFromA(
                    pszUpnName,
                    &pwszUpnname
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetKrbUPNKey(
                        hBinding,
                        pwszUpnname,
                        &dataContainer
                        );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(dataContainer.dwCount, (PVOID*)&pLocalByte );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory( pLocalByte,
                               dataContainer.dwCount,
                               dataContainer.data,
                               dataContainer.dwCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppKeyBlob = pLocalByte;
    *pSize     = dataContainer.dwCount;
    pLocalByte = NULL;

cleanup:

    if (hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    VMDIR_SAFE_FREE_MEMORY(pwszUpnname);
    VMDIR_RPC_FREE_MEMORY(dataContainer.data);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetKrbUPNKey (UPN=%s) failed (%u)",
                     VDIR_SAFE_STRING(pszUpnName), dwError );

    VMDIR_SAFE_FREE_MEMORY(pLocalByte);

    goto cleanup;

}

DWORD
VmDirAddReplicationAgreement(
    BOOLEAN bTwoWayRepl,
    PCSTR pszSrcHostName,
    PCSTR pszSrcPort,
    PCSTR pszSrcUserName,
    PCSTR pszSrcPassword,
    PCSTR pszTgtHostName,
    PCSTR pszTgtPort
)
{
    DWORD       dwError                 = 0;
    PSTR        pszDomainName           = NULL;
    PSTR        pszTopDomain            = NULL;
    PSTR        pszSrcServerName        = NULL;
    PSTR        pszTgtServerName        = NULL;

    // parameter check
    if (
         IsNullOrEmptyString (pszSrcHostName) ||
         IsNullOrEmptyString (pszSrcPort) ||
         IsNullOrEmptyString (pszSrcUserName) ||
         IsNullOrEmptyString (pszSrcPassword) ||
         IsNullOrEmptyString (pszTgtHostName) ||
         IsNullOrEmptyString (pszTgtPort)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName(
                            pszSrcHostName,
                            &pszSrcServerName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName(
                            pszTgtHostName,
                            &pszTgtServerName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(
                            pszTgtServerName,
                            &pszDomainName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_VERBOSE(
            VMDIR_LOG_MASK_ALL,
            "Domain name: %s\n",
            pszDomainName
            );

    pszTopDomain = VmDirGetTopDomain( pszDomainName );

    // setup one way first
    dwError = VmDirLdapSetupRemoteHostRA(
                    pszDomainName,
                    pszTgtServerName,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszSrcServerName,
                    0
                    );
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_VERBOSE(
            VMDIR_LOG_MASK_ALL,
            "Replication agreement created successfully:\nSource: %s\nTarget: %s\n",
            pszSrcServerName,
            pszTgtServerName
            );

    // setup the other way if specified
    if ( bTwoWayRepl )
    {
        dwError = VmDirLdapSetupRemoteHostRA(
                    pszDomainName,
                    pszSrcServerName,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtServerName,
                    0
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "Replication agreement created successfully:\nSource: %s\nTarget: %s\n",
                pszTgtServerName,
                pszSrcServerName
                );
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszSrcServerName);
    VMDIR_SAFE_FREE_MEMORY(pszTgtServerName);
    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Failed to create replication agreement. Error[%d]\n",
            dwError
            );
    goto cleanup;
}

DWORD
VmDirRemoveReplicationAgreement(
    BOOLEAN bTwoWayRepl,
    PCSTR pszSrcHostName,
    PCSTR pszSrcPort,
    PCSTR pszSrcUserName,
    PCSTR pszSrcPassword,
    PCSTR pszTgtHostName,
    PCSTR pszTgtPort
)
{
    DWORD       dwError                 = 0;
    PSTR        pszDomainName           = NULL;
    PSTR        pszTopDomain            = NULL;
    PSTR        pszSrcServerName        = NULL;
    PSTR        pszTgtServerName        = NULL;
    PVMDIR_REPL_PARTNER_INFO    pReplPartnerInfo = NULL;
    DWORD       dwNumReplPartner        = 0;

    // parameter check
    if (
         IsNullOrEmptyString (pszSrcHostName) ||
         IsNullOrEmptyString (pszSrcPort) ||
         IsNullOrEmptyString (pszSrcUserName) ||
         IsNullOrEmptyString (pszSrcPassword) ||
         IsNullOrEmptyString (pszTgtHostName) ||
         IsNullOrEmptyString (pszTgtPort)
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetReplicationPartners(
                        pszTgtHostName,
                        pszSrcUserName,
                        pszSrcPassword,
                        &pReplPartnerInfo,
                        &dwNumReplPartner
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Warn if less than two partners
    if (dwNumReplPartner < 2)
    {
        VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s: Attempting to remove replication agreements with %s having only %d partner\n",
                __FUNCTION__, pszTgtHostName, dwNumReplPartner);
    }

    if (bTwoWayRepl)
    {
        dwError = VmDirGetReplicationPartners(
                            pszSrcHostName,
                            pszSrcUserName,
                            pszSrcPassword,
                            &pReplPartnerInfo,
                            &dwNumReplPartner
                        );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Warn if less than two partners
        if (dwNumReplPartner < 2)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "%s: Attempting to remove replication agreements with %s having only %d partner\n",
                __FUNCTION__, pszSrcHostName, dwNumReplPartner);
        }
    }

    dwError = VmDirGetServerName(
                            pszSrcHostName,
                            &pszSrcServerName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetServerName(
                            pszTgtHostName,
                            &pszTgtServerName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(
                            pszTgtServerName,
                            &pszDomainName
                            );
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_VERBOSE(
            VMDIR_LOG_MASK_ALL,
            "Domain name: %s\n",
            pszDomainName
            );

    pszTopDomain = VmDirGetTopDomain( pszDomainName );

    // Remove one way first
    dwError = VmDirLdapRemoveRemoteHostRA(
                    pszDomainName,
                    pszTgtServerName,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszSrcServerName
                    );
    BAIL_ON_VMDIR_ERROR(dwError);
    VMDIR_LOG_VERBOSE(
            VMDIR_LOG_MASK_ALL,
            "Replication agreement removed successfully:\nSource: %s\nTarget: %s\n",
            pszSrcServerName,
            pszTgtServerName
            );

    // Remove the other way if speicified
    if ( bTwoWayRepl )
    {
        dwError = VmDirLdapRemoveRemoteHostRA(
                    pszDomainName,
                    pszSrcServerName,
                    pszSrcUserName,
                    pszSrcPassword,
                    pszTgtServerName
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
        VMDIR_LOG_VERBOSE(
                VMDIR_LOG_MASK_ALL,
                "Replication agreement removed successfully:\nSource: %s\nTarget: %s\n",
                pszTgtServerName,
                pszSrcServerName
                );
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "Failed to create replication agreement. Error[%d]\n",
            dwError
            );
    goto cleanup;
}

DWORD
VmDirGetReplicationPartners(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_REPL_PARTNER_INFO*  ppReplPartnerInfo,   // output
    DWORD*              pdwNumReplPartner    // output
)
{
    DWORD       dwError                 = 0;
    DWORD       i                       = 0;
    PSTR        pszDomain               = NULL;
    LDAP*       pLd                     = NULL;
    DWORD       dwInfoCount             = 0;
    PREPLICATION_INFO pReplicationInfo  = NULL;
    PVMDIR_REPL_PARTNER_INFO pReplPartnerInfo = NULL;
    PSTR        pszServerName = NULL;

    // parameter check
    if (
            IsNullOrEmptyString (pszHostName) ||
            IsNullOrEmptyString (pszUserName) ||
            pszPassword         == NULL       ||
            pdwNumReplPartner   == NULL       ||
            ppReplPartnerInfo   == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get domain name
    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // bind to server
    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszServerName,
                            pszDomain,
                            pszUserName,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    //get replication agreement info for replication LDUs
    dwError = VmDirGetReplicationInfo(
                            pLd,
                            pszServerName,
                            pszDomain,
                            &pReplicationInfo,
                            &dwInfoCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( dwInfoCount > 0 )
    {
        dwError = VmDirAllocateMemory(
                dwInfoCount*sizeof(VMDIR_REPL_PARTNER_INFO),
                (PVOID*)&pReplPartnerInfo
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        pReplPartnerInfo = NULL;
    }

    for ( i=0; i<dwInfoCount; i++)
    {
        dwError = VmDirAllocateStringA(
                    pReplicationInfo[i].pszURI,
                    &(pReplPartnerInfo[i].pszURI)
                    );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // fill in output parameters
    *pdwNumReplPartner = dwInfoCount;
    *ppReplPartnerInfo = pReplPartnerInfo;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);
    VMDIR_SAFE_FREE_MEMORY(pReplicationInfo);

    // unbind
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "VmDirGetReplicationPartners failed, Error[%d]\n",
            dwError
            );

    for ( i=0; i<dwInfoCount; i++ )
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo[i].pszURI);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);

    if ( ppReplPartnerInfo )
    {
        *ppReplPartnerInfo = NULL;
    }

    goto cleanup;
}

DWORD
VmDirGetReplicationPartnerStatus(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_REPL_PARTNER_STATUS*  ppReplPartnerStatus,   // output
    DWORD*              pdwNumReplPartner    // output
)
{
    DWORD       dwError = 0;
    DWORD       dwNumHost = 0;
    DWORD       dwCnt = 0;
    PVMDIR_REPL_PARTNER_STATUS  pReplPartnerStatus = NULL;
    PSTR*       ppszPartnerHosts = NULL;
    PSTR        pszDomain = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszServerName = NULL;

    // find hosts with hostname as the partner
    dwError = VmDirGetServerName(pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirFindAllReplPartnerHost(pszServerName, pszUserName, pszPassword, &ppszPartnerHosts, &dwNumHost);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwNumHost > 0)
    {
        dwError = VmDirAllocateMemory(
                dwNumHost*sizeof(VMDIR_REPL_PARTNER_STATUS),
                (PVOID*)&pReplPartnerStatus
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        goto cleanup;
    }


    for (dwCnt = 0; dwCnt < dwNumHost; dwCnt++)
    {
        dwError = VmDirAllocateStringA(ppszPartnerHosts[dwCnt], &pReplPartnerStatus[dwCnt].pszHost);
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_MEMORY(pszDomain);
        dwError = VmDirGetDomainName(
                        ppszPartnerHosts[dwCnt],
                        &pszDomain
                        );
        if (dwError)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "VmDirGetReplicationPartnerStatus, partner (%s) not available (%u)",
                ppszPartnerHosts[dwCnt],
                dwError);
            dwError  = 0;
            continue;
        }

        VmDirLdapUnbind(&pLd);
        dwError = VmDirConnectLDAPServer(
                                &pLd,
                                ppszPartnerHosts[dwCnt],
                                pszDomain,
                                pszUserName,
                                pszPassword);
        if (dwError)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "VmDirGetReplicationPartnerStatus, partner (%s) not available (%u)",
                ppszPartnerHosts[dwCnt],
                dwError);
            dwError  = 0;
            continue;
        }

        pReplPartnerStatus[dwCnt].bHostAvailable = TRUE;

        // get partner replication status
        dwError = VmDirGetPartnerReplicationStatus(pLd, pszServerName, &pReplPartnerStatus[dwCnt]);
        if (dwError)
        {
            VMDIR_LOG_WARNING(
                VMDIR_LOG_MASK_ALL,
                "VmDirGetReplicationPartnerStatus, partner (%s) status not available (%u)",
                ppszPartnerHosts[dwCnt],
                dwError);
            dwError  = 0;
            continue;
        }

        pReplPartnerStatus[dwCnt].bStatusAvailable = TRUE;
    }

    *ppReplPartnerStatus = pReplPartnerStatus;
    *pdwNumReplPartner = dwNumHost;

cleanup:

    VmDirLdapUnbind(&pLd);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    for (dwCnt=0; dwCnt < dwNumHost; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(ppszPartnerHosts[dwCnt]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppszPartnerHosts);

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "VmDirGetReplicationPartnerStatus failed. Error[%d]\n",
            dwError
            );

    for (dwCnt = 0; dwCnt < dwNumHost; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus[dwCnt].pszHost);
    }

    VMDIR_SAFE_FREE_MEMORY(pReplPartnerStatus);

    if (ppReplPartnerStatus)
    {
        *ppReplPartnerStatus = NULL;
    }

    goto cleanup;
}

DWORD
VmDirGetReplicationState(
    PVMDIR_CONNECTION  pConnection,
    PVMDIR_REPL_STATE* ppReplState
    )
{
    return VmDirGetReplicationStateInternal(pConnection->pLd, ppReplState);
}

VOID
VmDirFreeReplicationState(
    PVMDIR_REPL_STATE   pReplState
    )
{
    VmDirFreeReplicationStateInternal(pReplState);
    return;
}

DWORD
VmDirGetReplicationCycleCount(
    PVMDIR_CONNECTION   pConnection,
    DWORD*              pdwReplCycleCount
    )
{
    return VmDirGetReplicateCycleCountInternal( pConnection, pdwReplCycleCount );
}

DWORD
VmDirGetServers(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_SERVER_INFO*  ppServerInfo,   // output
    DWORD*              pdwNumServer    // output
)
{
    DWORD       dwError                 = 0;
    DWORD       i                       = 0;
    PSTR        pszDomain               = NULL;
    LDAP*       pLd                     = NULL;
    DWORD       dwInfoCount             = 0;
    PINTERNAL_SERVER_INFO pInternalServerInfo  = NULL;
    PVMDIR_SERVER_INFO pServerInfo = NULL;
    PSTR        pszServerName = NULL;
    char        bufUPN[VMDIR_MAX_UPN_LEN] = {0};

    // parameter check
    if (
            IsNullOrEmptyString (pszHostName) ||
            IsNullOrEmptyString (pszUserName) ||
            pszPassword         == NULL       ||
            pdwNumServer   == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get domain name
    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s", pszUserName, pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLd,
                                 pszServerName,
                                 bufUPN,
                                 pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    //get all vmdir servers in the forest.
    dwError = VmDirGetServersInfo(
                            pLd,
                            pszServerName,
                            pszDomain,
                            &pInternalServerInfo,
                            &dwInfoCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (ppServerInfo)
    {
        dwError = VmDirAllocateMemory(
                dwInfoCount*sizeof(VMDIR_SERVER_INFO),
                (PVOID*)&pServerInfo
        );
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( i=0; i<dwInfoCount; i++)
        {
            dwError = VmDirAllocateStringA(
                    pInternalServerInfo[i].pszServerDN,
                    &(pServerInfo[i].pszServerDN)
            );
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        *ppServerInfo = pServerInfo;
    }

    *pdwNumServer = dwInfoCount;

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pInternalServerInfo);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    // unbind
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "VmDirGetServers failed. Error[%d]\n",
            dwError
            );
    goto cleanup;
}

DWORD
VmDirGetComputersByOrgUnit(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PCSTR               pszOrgUnit,
    PSTR**              pppszComputers,
    DWORD*              pdwNumComputers
    )
{
    DWORD       dwError                 = 0;
    PSTR        pszServerName           = NULL;
    PSTR        pszDomain               = NULL;
    LDAP*       pLd                     = NULL;
    PSTR        pszComputerDNPrefix     = NULL;
    PSTR*       ppszComputers           = NULL;
    DWORD       dwNumComputers          = 0;

    if (
            IsNullOrEmptyString (pszHostName) ||
            IsNullOrEmptyString (pszUserName) ||
            IsNullOrEmptyString (pszOrgUnit)  ||
            pszPassword         == NULL       ||
            pdwNumComputers     == NULL       ||
            pppszComputers      == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName(pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszServerName,
                            pszDomain,
                            pszUserName,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( VmDirStringCompareA(pszOrgUnit, VMDIR_COMPUTERS_RDN_VAL, FALSE) == 0)
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszComputerDNPrefix,
                    "%s=%s",
                    ATTR_OU,
                    pszOrgUnit);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        dwError = VmDirAllocateStringPrintf(
                    &pszComputerDNPrefix,
                    "%s=%s,%s=%s",
                    ATTR_OU,
                    pszOrgUnit,
                    ATTR_OU,
                    VMDIR_COMPUTERS_RDN_VAL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetObjectAttribute(
                pLd,
                pszDomain,
                pszComputerDNPrefix,
                OC_COMPUTER,
                ATTR_CN,
                LDAP_SCOPE_SUBTREE,
                &ppszComputers,
                &dwNumComputers
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    *pppszComputers = ppszComputers;
    *pdwNumComputers = dwNumComputers;

cleanup:

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    VMDIR_SAFE_FREE_STRINGA(pszDomain);
    VMDIR_SAFE_FREE_STRINGA(pszServerName);
    VMDIR_SAFE_FREE_STRINGA(pszComputerDNPrefix);
    return dwError;

error:
    if (pppszComputers)
    {
        *pppszComputers = NULL;
    }
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed. Error[%d]\n",
            __FUNCTION__,
            dwError
            );
    goto cleanup;
}

/*
 * Get all computers under "ou=computers" which include sub OU container as well.
 */
DWORD
VmDirGetComputers(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PSTR**              pppszComputers,
    DWORD*              pdwNumComputers
    )
{
    return VmDirGetComputersByOrgUnit(
                pszHostName,
                pszUserName,
                pszPassword,
                VMDIR_COMPUTERS_RDN_VAL,
                pppszComputers,
                pdwNumComputers);
}

DWORD
VmdirGetSiteDCInfo(
    LDAP*           pLd,
    PCSTR           pszSiteName,
    PCSTR           pszDomain,
    DWORD*          pIdxDC,
    PVMDIR_DC_INFO* ppDC,
    DWORD           dwNumDC
    )
{
    DWORD dwError = 0;
    DWORD idxServer = 0;
    PSTR  pszServerDNPrefix = NULL;
    PSTR  pszSiteServersDNPrefix = NULL;
    PSTR* ppszServers = NULL;
    DWORD dwNumServer = 0;
    PSTR* ppszPartners = NULL;
    DWORD dwNumPartners = 0;

    dwError = VmDirAllocateStringPrintf(
                &pszSiteServersDNPrefix,
                "cn=Servers,cn=%s,cn=Sites,cn=Configuration",
                pszSiteName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetObjectAttribute(
                pLd,
                pszDomain,
                pszSiteServersDNPrefix,
                OC_DIR_SERVER,
                ATTR_CN,
                LDAP_SCOPE_ONELEVEL,
                &ppszServers,
                &dwNumServer
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (idxServer = 0; idxServer < dwNumServer; ++idxServer, ++(*pIdxDC))
    {
        dwError = VmDirAllocateMemory(
                sizeof(VMDIR_DC_INFO),
                (PVOID*)&ppDC[*pIdxDC]
                );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                    ppszServers[idxServer],
                    &ppDC[*pIdxDC]->pszHostName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringA(
                    pszSiteName,
                    &ppDC[*pIdxDC]->pszSiteName);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateStringPrintf(
                    &pszServerDNPrefix,
                    "cn=%s,%s",
                    ppszServers[idxServer],
                    pszSiteServersDNPrefix);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirGetObjectAttribute(
                    pLd,
                    pszDomain,
                    pszServerDNPrefix,
                    OC_REPLICATION_AGREEMENT,
                    ATTR_LABELED_URI,
                    LDAP_SCOPE_SUBTREE,
                    &ppszPartners,
                    &dwNumPartners
                    );
        BAIL_ON_VMDIR_ERROR(dwError);

        VMDIR_SAFE_FREE_STRINGA(pszServerDNPrefix);

        if (dwNumPartners > 0)
        {
            DWORD idxPartner = 0;
            dwError = VmDirAllocateMemory(
                        sizeof(PSTR)*dwNumPartners,
                        (PVOID)&ppDC[*pIdxDC]->ppPartners);
            BAIL_ON_VMDIR_ERROR(dwError);
            for (idxPartner = 0; idxPartner < dwNumPartners; ++idxPartner)
            {
                dwError = VmDirAllocateStringA(
                    ppszPartners[idxPartner],
                    &ppDC[*pIdxDC]->ppPartners[idxPartner]);
            }
            BAIL_ON_VMDIR_ERROR(dwError);

            ppDC[*pIdxDC]->dwPartnerCount = dwNumPartners;
       }

        VmDirFreeStringArray(ppszPartners, dwNumPartners);
        ppszPartners = NULL;
    }

cleanup:
    VmDirFreeStringArray(ppszPartners, dwNumPartners);
    VmDirFreeStringArray(ppszServers, dwNumServer);
    VMDIR_SAFE_FREE_STRINGA(pszServerDNPrefix);
    VMDIR_SAFE_FREE_MEMORY(pszSiteServersDNPrefix);
    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirGetDCInfo(
    PCSTR               pszHostName,
    PCSTR               pszUserName,
    PCSTR               pszPassword,
    PVMDIR_DC_INFO**    pppDC,
    DWORD*              pdwNumDC
    )
{
    DWORD       dwError = 0;
    DWORD       idxDC = 0;
    DWORD       idxSite = 0;
    PSTR        pszDomain = NULL;
    LDAP*       pLd = NULL;
    PSTR        pszServerName = NULL;
    PSTR*       ppszServers = NULL;
    DWORD       dwNumDC = 0;
    PSTR*       ppszSites = NULL;
    DWORD       dwNumSite = 0;
    PVMDIR_DC_INFO* ppDC = NULL;

    if (
            IsNullOrEmptyString (pszHostName) ||
            IsNullOrEmptyString (pszUserName) ||
            pszPassword         == NULL       ||
            pppDC               == NULL       ||
            pdwNumDC            == NULL
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerName( pszHostName, &pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName(
                    pszServerName,
                    &pszDomain
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConnectLDAPServer(
                            &pLd,
                            pszServerName,
                            pszDomain,
                            pszUserName,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetObjectAttribute(
                pLd,
                pszDomain,
                "cn=Sites,cn=Configuration",
                OC_DIR_SERVER,
                ATTR_CN,
                LDAP_SCOPE_SUBTREE,
                &ppszServers,
                &dwNumDC
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (dwNumDC > 0 )
    {
        dwError = VmDirAllocateMemory(
                dwNumDC*sizeof(VMDIR_DC_INFO),
                (PVOID*)&ppDC
                );
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        *pdwNumDC = 0;
        goto cleanup;
    }

    dwError = VmDirGetObjectAttribute(
                pLd,
                pszDomain,
                "cn=Sites,cn=Configuration",
                OC_CONTAINER,
                ATTR_CN,
                LDAP_SCOPE_ONELEVEL,
                &ppszSites,
                &dwNumSite
                );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (idxSite = 0; idxSite < dwNumSite; idxSite++)
    {
        dwError = VmdirGetSiteDCInfo(
                        pLd,
                        ppszSites[idxSite],
                        pszDomain,
                        &idxDC,
                        ppDC,
                        dwNumDC);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pppDC = ppDC;
    *pdwNumDC = dwNumDC;

cleanup:
    VmDirFreeStringArray(ppszSites, dwNumSite);
    VmDirFreeStringArray(ppszServers, dwNumDC);
    ppszServers = NULL;

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszServerName);

    return dwError;

error:
    VMDIR_LOG_ERROR(
            VMDIR_LOG_MASK_ALL,
            "%s failed, Error[%d]\n",
            __FUNCTION__,
            dwError
            );

    VmDirFreeDCInfoArray(ppDC, dwNumDC);
    if (pppDC)
    {
        *pppDC = NULL;
    }

    goto cleanup;
}

VOID
VmDirFreeDCInfo(
    PVMDIR_DC_INFO      pDC
    )
{
    if (pDC)
    {
        VmDirFreeStringArray(pDC->ppPartners, pDC->dwPartnerCount);
        VMDIR_SAFE_FREE_STRINGA(pDC->pszHostName);
        VMDIR_SAFE_FREE_STRINGA(pDC->pszSiteName);
        VMDIR_SAFE_FREE_MEMORY(pDC);
    }
}

VOID
VmDirFreeDCInfoArray(
    PVMDIR_DC_INFO*     ppDC,
    DWORD               dwNumDC
    )
{
    DWORD idx = 0;
    if (ppDC && dwNumDC > 0)
    {
        for (; idx < dwNumDC; ++idx)
        {
            VmDirFreeDCInfo(ppDC[idx]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppDC);
    }
}

DWORD
VmDirReplNow(
    PCSTR pszServerName)
{
    DWORD       dwError = 0;
    PCSTR       pszServerEndpoint = NULL;
    handle_t    hBinding = NULL;

    // parameter check
    if (pszServerName == NULL)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirCreateBindingHandleMachineAccountA(
                    pszServerName,
                    pszServerEndpoint,
                    &hBinding);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirReplNow(hBinding);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirReplNow failed. Error[%d]\n", dwError );
    goto cleanup;
}

DWORD
VmDirSetLogLevelH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    PCSTR       pszLogLevel
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;
    VMDIR_LOG_LEVEL myLogLevel = VMDIR_LOG_INFO;

    if ( !pszLogLevel)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if ( VmDirStringCompareA(pszLogLevel, "ERROR", FALSE) == 0)
    {
        myLogLevel = VMDIR_LOG_ERROR;
    }
    else if ( VmDirStringCompareA(pszLogLevel, "WARNING", FALSE) == 0)
    {
        myLogLevel = VMDIR_LOG_WARNING;
    }
    else if ( VmDirStringCompareA(pszLogLevel, "INFO", FALSE) == 0)
    {
        myLogLevel = VMDIR_LOG_INFO;
    }
    else if ( VmDirStringCompareA(pszLogLevel, "VERBOSE", FALSE) == 0)
    {
        myLogLevel = VMDIR_LOG_VERBOSE;
    }
    else if ( VmDirStringCompareA(pszLogLevel, "DEBUG", FALSE) == 0)
    {
        myLogLevel = VMDIR_LOG_DEBUG;
    }
    else
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSetLogLevel(
                          hBinding,
                          myLogLevel);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSetLogLevel failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirGetLogLevelH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    VMDIR_LOG_LEVEL*         pLogLevel
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;
    VMDIR_LOG_LEVEL logLevel = 0;

    if (!pLogLevel)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetLogLevel(
                          hBinding,
                          &logLevel);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pLogLevel = logLevel;

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirGetLogLevel failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSetLogLevel(
    PCSTR       pszLogLevel
    )
{
    DWORD dwError = 0;
    dwError = VmDirSetLogLevelH(
                  NULL,
                  pszLogLevel);
    return dwError;
}

DWORD
VmDirSetLogMaskH(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    UINT32      iVmDirLogMask
    )
{
    DWORD       dwError = 0;
    PCSTR       pszServerName = "localhost";
    PCSTR       pszServerEndpoint = NULL;
    handle_t    hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSetLogMask(
                          hBinding,
                          iVmDirLogMask);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSetLogMask failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirGetLogMaskH(
    PVMDIR_SERVER_CONTEXT   hInBinding,
    UINT32*                 piVmDirLogMask
    )
{
    DWORD       dwError = 0;
    PCSTR       pszServerName = "localhost";
    PCSTR       pszServerEndpoint = NULL;
    handle_t    hBinding = NULL;
    UINT32      iMask = 0;

    if (!piVmDirLogMask)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetLogMask(
                          hBinding,
                          &iMask);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *piVmDirLogMask = iMask;

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirGetLogMask failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSetLogMask(
    UINT32      iVmDirLogMask
    )
{
    DWORD dwError = 0;

    dwError = VmDirSetLogMaskH(
                  NULL,
                  iVmDirLogMask);
    return dwError;
}

DWORD
VmDirSetState(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    UINT32      dwState)
{
    DWORD dwError = 0;
    PCSTR pszServerName = "localhost";
    PCSTR pszServerEndpoint = NULL;
    handle_t hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSetState(
                          hBinding,
                          dwState
                          );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetState failed. Error[%d]\n", dwError );
    goto cleanup;
}

DWORD
VmDirGetState(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    UINT32*                  pdwState)
{
    DWORD dwError = 0;
    PCSTR pszServerName = "localhost";
    PCSTR pszServerEndpoint = NULL;
    handle_t hBinding = NULL;
    UINT32  dwState = 0;

    if (!pdwState)
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding
                        );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirGetState(
                          hBinding,
                          &dwState
                          );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwState = dwState;

cleanup:
    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetState failed. Error[%d]\n", dwError );
    goto cleanup;
}

DWORD
VmDirSuperLogQueryServerData(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    PVMDIR_SUPERLOG_SERVER_DATA *ppServerData
    )
{
    DWORD       dwError = 0;
    PCSTR       pszServerName = "localhost";
    PCSTR       pszServerEndpoint = NULL;
    handle_t    hBinding = NULL;

    PVMDIR_SUPERLOG_SERVER_DATA pRpcServerData = NULL;
    PVMDIR_SUPERLOG_SERVER_DATA pServerData = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogQueryServerData(
                          hBinding,
                          &pRpcServerData);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_SUPERLOG_SERVER_DATA),
            (PVOID*)&pServerData
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    pServerData->iServerStartupTime = pRpcServerData->iServerStartupTime;
    pServerData->iAddCount = pRpcServerData->iAddCount;
    pServerData->iBindCount = pRpcServerData->iBindCount;
    pServerData->iDeleteCount = pRpcServerData->iDeleteCount;
    pServerData->iModifyCount = pRpcServerData->iModifyCount;
    pServerData->iSearchCount = pRpcServerData->iSearchCount;
    pServerData->iUnbindCount = pRpcServerData->iUnbindCount;

    *ppServerData = pServerData;

cleanup:
    if (pRpcServerData)
    {
        VmDirRpcClientFreeMemory(pRpcServerData);
    }
    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirSuperLogQueryServerData failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogEnable(
    PVMDIR_SERVER_CONTEXT    hInBinding
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogEnable(
                          hBinding);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSuperLogEnable failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogDisable(
    PVMDIR_SERVER_CONTEXT    hInBinding
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogDisable(
                          hBinding);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSuperLogDisable failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirIsSuperLogEnabled(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    PBOOLEAN                 pbEnabled
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;
    BOOLEAN         bEnabled = 0;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirIsSuperLogEnabled(
                          hBinding,
                          &bEnabled);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pbEnabled = bEnabled;

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirIsSuperLogEnabled failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogFlush(
    PVMDIR_SERVER_CONTEXT  hInBinding
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogFlush(
                          hBinding);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSuperLogFlush failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogSetSize(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    DWORD                    dwSize
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogSetSize(
                          hBinding,
                          dwSize);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSuperLogSetSize failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogGetSize(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    PDWORD                   pdwSize
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;

    DWORD           dwSize = 0;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogGetSize(
                          hBinding,
                          &dwSize);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwSize = dwSize;

cleanup:

    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }

    return dwError;

error:

    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                    "VmDirSuperLogGetSize failed. Error[%d]\n", dwError);
    goto cleanup;
}

DWORD
VmDirSuperLogGetEntriesLdapOperation(
    PVMDIR_SERVER_CONTEXT    hInBinding,
    ULONG64 **ppEnumerationCookie,
    DWORD                    dwCount, // 0 ==> all
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppEntries
    )
{
    DWORD           dwError = 0;
    PCSTR           pszServerName = "localhost";
    PCSTR           pszServerEndpoint = NULL;
    handle_t        hBinding = NULL;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pEntries = NULL;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pRpcEntries = NULL;

    if (hInBinding)
    {
        hBinding = hInBinding->hBinding;
    }
    else
    {
        dwError = VmDirCreateBindingHandleMachineAccountA(
                        pszServerName,
                        pszServerEndpoint,
                        &hBinding);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirSuperLogGetEntriesLdapOperation(
                          hBinding,
                          (vmdir_superlog_cookie_t *)ppEnumerationCookie,
                          dwCount,
                          &pRpcEntries);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirAllocateSuperLogEntryLdapOperationArray(pRpcEntries, &pEntries);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppEntries = pEntries;

cleanup:
    VmDirRpcFreeSuperLogEntryLdapOperationArray(pRpcEntries);
    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle(&hBinding);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "VmDirSuperLogGetSize failed. Error[%d]\n", dwError);
    VmDirFreeSuperLogEntryLdapOperationArray(pEntries);
    goto cleanup;
}

static
DWORD
_CopySearchInformation(
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pSrcEntry,
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION pDstEntry
    )
{
    DWORD dwError = 0;

    if (!pSrcEntry || !pDstEntry)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringW(
            pSrcEntry->opInfo.searchInfo.pwszAttributes,
            &pDstEntry->opInfo.searchInfo.pwszAttributes);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
            pSrcEntry->opInfo.searchInfo.pwszBaseDN,
            &pDstEntry->opInfo.searchInfo.pwszBaseDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
            pSrcEntry->opInfo.searchInfo.pwszScope,
            &pDstEntry->opInfo.searchInfo.pwszScope);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringW(
            pSrcEntry->opInfo.searchInfo.pwszIndexResults,
            &pDstEntry->opInfo.searchInfo.pwszIndexResults);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDstEntry->opInfo.searchInfo.dwScanned = pSrcEntry->opInfo.searchInfo.dwScanned;
    pDstEntry->opInfo.searchInfo.dwReturned = pSrcEntry->opInfo.searchInfo.dwReturned;

cleanup:
    return dwError;

error:
    goto cleanup;
}

static
DWORD
_VmDirAllocateSuperLogEntryLdapOperationArray(
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pSrcEntries,
        PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY *ppDstEntries
        )
{
    DWORD   dwError = 0;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION srcEntries = NULL;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION dstEntries = NULL;
    PVMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY pDstEntries = NULL;
    unsigned int i;

    if (!pSrcEntries || !ppDstEntries)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
            sizeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION_ARRAY),
            (PVOID*)&pDstEntries
            );
    BAIL_ON_VMDIR_ERROR(dwError);

    pDstEntries->dwCount = 0;
    pDstEntries->entries = NULL;

    if (pSrcEntries->dwCount > 0)
    {
        dwError = VmDirAllocateMemory(
                sizeof(VMDIR_SUPERLOG_ENTRY_LDAPOPERATION)*pSrcEntries->dwCount,
                (PVOID*)&pDstEntries->entries
                );
        BAIL_ON_VMDIR_ERROR(dwError);

        pDstEntries->dwCount = pSrcEntries->dwCount;
        srcEntries = pSrcEntries->entries;
        dstEntries = pDstEntries->entries;

        for (i = 0; i < pDstEntries->dwCount; i++)
        {
            dwError = VmDirAllocateStringW(srcEntries[i].pwszLoginDN, &(dstEntries[i].pwszLoginDN));
            BAIL_ON_VMDIR_ERROR(dwError);
            dwError = VmDirAllocateStringW(srcEntries[i].pwszClientIP, &(dstEntries[i].pwszClientIP));
            BAIL_ON_VMDIR_ERROR(dwError);
            dwError = VmDirAllocateStringW(srcEntries[i].pwszServerIP, &(dstEntries[i].pwszServerIP));
            BAIL_ON_VMDIR_ERROR(dwError);
            dwError = VmDirAllocateStringW(srcEntries[i].pwszOperation, &(dstEntries[i].pwszOperation));
            BAIL_ON_VMDIR_ERROR(dwError);
            dwError = VmDirAllocateStringW(srcEntries[i].pwszString, &(dstEntries[i].pwszString));
            BAIL_ON_VMDIR_ERROR(dwError);
            dstEntries[i].dwClientPort = srcEntries[i].dwClientPort;
            dstEntries[i].dwServerPort = srcEntries[i].dwServerPort;
            dstEntries[i].dwErrorCode = srcEntries[i].dwErrorCode;
            dstEntries[i].iStartTime = srcEntries[i].iStartTime;
            dstEntries[i].iEndTime = srcEntries[i].iEndTime;
            dstEntries[i].opType = srcEntries[i].opType;

            switch (dstEntries[i].opType)
            {
            case LDAP_REQ_SEARCH:
                dwError = _CopySearchInformation(&srcEntries[i], &dstEntries[i]);
                BAIL_ON_VMDIR_ERROR(dwError);
                break;
            default:
                break;
            }
        }
    }

    *ppDstEntries = pDstEntries;

cleanup:
    return dwError;

error:
    VmDirFreeSuperLogEntryLdapOperationArray(pDstEntries);
    goto cleanup;
}


// Write UPN keys for the machine and service accounts to the keytab file.
DWORD
VmDirUpdateKeytabFile(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    BOOLEAN bIsServer
    )
{
    DWORD                   dwError = 0;
    PVMDIR_KEYTAB_HANDLE    pKeyTabHandle = NULL;
    PSTR                    pszUpperCaseDomainName = NULL;
    CHAR                    pszKeyTabFileName[VMDIR_MAX_FILE_NAME_LEN] = {0};
    PSTR                    pszSRPUPN = NULL;
    PSTR                    pszMachineAccountUPN = NULL;
    PSTR                    pszServiceAccountUPN = NULL;
    PCSTR                   pszServerServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;
    PCSTR                   pszClientServiceTable[] = VMDIR_CLIENT_SERVICE_PRINCIPAL_INITIALIZER;
    PCSTR                  *pszServiceTable = NULL;
    int                     iServiceTableLen = 0;
    int                     iCnt = 0;
    PBYTE                   pLocalByte = NULL;
    DWORD                   dwByteSize = 0;
    DWORD                   dwWriteLen = 0;
    PSTR                    pszLowerCaseHostName = NULL;

    dwError = VmDirAllocASCIILowerToUpper( pszDomainName, &pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocASCIIUpperToLower( pszHostName, &pszLowerCaseHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Locate and open keytab file
    dwError = VmDirGetRegKeyTabFile(pszKeyTabFileName);
    if (dwError)
    {
        dwError = ERROR_SUCCESS;    // For none kerberos configuration, pass through.
        goto cleanup;
    }

    dwError = VmDirKeyTabOpen(pszKeyTabFileName, "a", &pKeyTabHandle);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf( &pszMachineAccountUPN, "%s@%s", pszLowerCaseHostName, pszUpperCaseDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);


    dwError = VmDirAllocateStringPrintf( &pszSRPUPN, "%s@%s", pszUserName, pszDomainName );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = _VmDirGetKeyTabRecBlob(pszServerName,
                                     pszSRPUPN,
                                     pszPassword,
                                     pszMachineAccountUPN,
                                     &pLocalByte,
                                     &dwByteSize );
    BAIL_ON_VMDIR_ERROR( dwError );

    dwWriteLen = (DWORD) fwrite(pLocalByte, 1, dwByteSize, pKeyTabHandle->ktfp);
    if ( dwWriteLen != dwByteSize)
    {
        /* I/O Error */
        dwError = ERROR_IO;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (bIsServer)
    {
        pszServiceTable = pszServerServiceTable;
        iServiceTableLen = sizeof(pszServerServiceTable)/sizeof(pszServerServiceTable[0]);
    }
    else
    {
        pszServiceTable = pszClientServiceTable;
        iServiceTableLen = sizeof(pszClientServiceTable)/sizeof(pszClientServiceTable[0]);
    }

    // Get UPN keys for the service accounts and write to keytab file
    for (iCnt = 0; iCnt < iServiceTableLen; iCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pszServiceAccountUPN);
        VMDIR_SAFE_FREE_MEMORY(pLocalByte);
        dwByteSize = 0;

        dwError = VmDirAllocateStringPrintf( &pszServiceAccountUPN, "%s/%s@%s", pszServiceTable[iCnt], pszLowerCaseHostName, pszUpperCaseDomainName );
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = _VmDirGetKeyTabRecBlob(pszServerName,
                                         pszSRPUPN,
                                         pszPassword,
                                         pszServiceAccountUPN,
                                         &pLocalByte,
                                         &dwByteSize );
        BAIL_ON_VMDIR_ERROR( dwError );

        dwWriteLen = (DWORD) fwrite(pLocalByte, 1, dwByteSize, pKeyTabHandle->ktfp);
        if ( dwWriteLen != dwByteSize)
        {
            /* I/O Error */
            dwError = ERROR_IO;
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Keytab file (%s) created", VDIR_SAFE_STRING(pszKeyTabFileName));

cleanup:
    if (pKeyTabHandle)
    {
        VmDirKeyTabClose(pKeyTabHandle);
    }

    VMDIR_SAFE_FREE_MEMORY(pszMachineAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszServiceAccountUPN);
    VMDIR_SAFE_FREE_MEMORY(pszUpperCaseDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLowerCaseHostName);
    VMDIR_SAFE_FREE_MEMORY(pLocalByte);
    VMDIR_SAFE_FREE_MEMORY(pszSRPUPN);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirUpdateKeytabFile (%s) for host(%s) domain(%s) failed (%u)",
                                         VDIR_SAFE_STRING(pszKeyTabFileName),
                                         VDIR_SAFE_STRING(pszLowerCaseHostName),
                                         VDIR_SAFE_STRING(pszUpperCaseDomainName),
                                         dwError);

    // for 2013 release, we should continue even if keytab setup fail
    // as naming and DNS config may not meet our requirements.
    // (i.e. at least forward lookup must be there.  For krb to work,
    //       we need reverse lookup as well.)
    dwError = 0;

    goto cleanup;
}

static
DWORD
_VmDirLdapCheckVmDirStatus(
    PCSTR pszPartnerHostName
    )
{
    DWORD       dwError = 0;

    PSTR        pszLocalServerReplURI = NULL;
    LDAP *      pLd = NULL;
    DWORD       i = 0;
    BOOLEAN     bFirst = TRUE;
    DWORD       dwTimeout = 15; //wait 2.5 minutes for 1st Ldu
    VDIR_SERVER_STATE vmdirState = VMDIRD_STATE_UNDEFINED;

    if (!IsNullOrEmptyString(pszPartnerHostName))
    {
        bFirst = FALSE;
        dwTimeout = -1; //infinite minutes for 2nd Ldu, because we could be copying really big DB from partner.
    }

    dwError = VmDirAllocateStringPrintf( &pszLocalServerReplURI, "%s://localhost:%d",
                                             VMDIR_LDAP_PROTOCOL, DEFAULT_LDAP_PORT_NUM );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (bFirst)
    {
        printf("Wait for local instance LDAP service ");
    }
    else
    {
        printf("Wait for local replica to finish replication ");
    }
    fflush(stdout);

    while ((dwTimeout == -1) || (i < dwTimeout))
    {
        dwError = VmDirAnonymousLDAPBind( &pLd, pszLocalServerReplURI );

        if (dwError == 0)
        {
            break;
        }

        printf(".");
        fflush(stdout);

        VmDirSleep(SLEEP_INTERVAL_IN_SECS*1000);

        i++;
        VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "LDAP connect (%s) failed (%u), %d seconds passed",
                           VDIR_SAFE_STRING(pszLocalServerReplURI), dwError, i * SLEEP_INTERVAL_IN_SECS);

        if( !bFirst )
        {
            dwError = VmDirLocalGetServerState( (UINT32*)&vmdirState );
            BAIL_ON_VMDIR_ERROR(dwError);

            if(vmdirState == VMDIRD_STATE_FAILURE)
            {
                dwError = VMDIR_ERROR_SERVER_DOWN;
                VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                                 "VmDirLdapCheckVmDirStatus: Server in unrecoverable state");
                BAIL_ON_VMDIR_ERROR(dwError);
            }
        }
    }
    printf("\n");
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalServerReplURI);
    VmDirLdapUnbind(&pLd);

    return dwError;

error:
    printf("Vmdir LDAP connectivity check failed or timed out");
    fflush(stdout);

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirLdapCheckVmDirStatus (%s) failed with error (%u)",
                     VDIR_SAFE_STRING(pszLocalServerReplURI), dwError);
    goto cleanup;
}

// Create machine and krb service account
// 1. machine account: machineFQDN@REALM
// 2. ldap service account: ldap/machineFQDN@REALM
// 3. host service account: host/machineFQDN@REALM
// 4. vmca service account: vmca/machineFQDN@REALM
DWORD
VmDirSetupDefaultAccount(
    PCSTR pszDomainName,
    PCSTR pszPartnerServerName,
    PCSTR pszLdapHostName,
    PCSTR pszBindUserName,
    PCSTR pszBindPassword
    )
{
    DWORD   dwError = 0;
    PCSTR   pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;
    int     iCnt = 0;

    dwError = VmDirLdapSetupDCAccountOnPartner(
                                    pszDomainName,
                                    pszPartnerServerName,
                                    pszBindUserName,
                                    pszBindPassword,
                                    pszLdapHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (iCnt = 0; iCnt < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); iCnt++)
    {
        dwError = VmDirLdapSetupServiceAccount(
                                        NULL, // TBD, should share pLd as does in VmDirLdapSetupComputerAccount.
                                        pszDomainName,
                                        pszPartnerServerName,
                                        pszBindUserName,
                                        pszBindPassword,
                                        pszServiceTable[iCnt],
                                        pszLdapHostName );
        if (dwError == LDAP_ALREADY_EXISTS)
        {
            dwError = LDAP_SUCCESS; // ignore if entry already exists (maybe due to prior merge/join..etc)
            VMDIR_LOG_WARNING( VMDIR_LOG_MASK_ALL, "_VmDirSetupKrbAccount SetupServiceAccount (%s) return LDAP_ALREADY_EXISTS",
                                                   pszServiceTable[iCnt] );
        }
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirSetupKrbAccount (%s)(%s) passed",
                                        VDIR_SAFE_STRING(pszDomainName),
                                        VDIR_SAFE_STRING(pszPartnerServerName) );
cleanup:

    return dwError;

error:

    goto cleanup;
}

/*
 * find hosts that has pszHostName as repliaction partner
 */
static
DWORD
_VmDirFindAllReplPartnerHost(
    PCSTR    pszHostName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PSTR**   pppszPartnerHost,
    DWORD*   pdwSize
    )
{
    DWORD       dwError=0;
    PSTR*       ppszLocal=NULL;
    DWORD       dwNumReplPartner=0;
    DWORD       dwCnt=0;

    PVMDIR_REPL_PARTNER_INFO    pReplPartnerInfo = NULL;

    dwError = VmDirGetReplicationPartners( pszHostName,
                                           pszUserName,
                                           pszPassword,
                                           &pReplPartnerInfo,
                                           &dwNumReplPartner);
    BAIL_ON_VMDIR_ERROR(dwError);

    if ( dwNumReplPartner > 0 )
    {
        dwError = VmDirAllocateMemory( dwNumReplPartner * sizeof(PSTR), (PVOID)&ppszLocal );
        BAIL_ON_VMDIR_ERROR(dwError);

        for ( dwCnt=0; dwCnt < dwNumReplPartner; dwCnt++ )
        {
            dwError = VmDirReplURIToHostname( pReplPartnerInfo[dwCnt].pszURI, &(ppszLocal[dwCnt]) );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    *pppszPartnerHost = ppszLocal;
    *pdwSize = dwNumReplPartner;
    ppszLocal = NULL;

cleanup:
    for (dwCnt=0; dwCnt < dwNumReplPartner; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo[dwCnt].pszURI);
    }
    VMDIR_SAFE_FREE_MEMORY(pReplPartnerInfo);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirFindAllReplPartnerHost failed (%u)", dwError);
    for (dwCnt=0; dwCnt < dwNumReplPartner; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY(ppszLocal[dwCnt]);
    }
    VMDIR_SAFE_FREE_MEMORY(ppszLocal);

    goto cleanup;
}

/*
 *  Delete all replication agreements that has pszHost as partner
 */
static
DWORD
_VmDirDeleteReplAgreementToHost(
   LDAP*    pLD,
   PCSTR    pszHost
   )
{
    DWORD       dwError=0;
    DWORD       dwSize=0;
    DWORD       dwCnt=0;
    PSTR*       ppszRADNs=NULL;

    dwError = VmDirGetAllRAToHost( pLD, pszHost, &ppszRADNs, &dwSize );
    BAIL_ON_VMDIR_ERROR(dwError);

    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        dwError = ldap_delete_ext_s( pLD, ppszRADNs[dwCnt], NULL, NULL );
        switch (dwError)
        {
            case LDAP_NO_SUCH_OBJECT:
                dwError = LDAP_SUCCESS;
                break;

            case LDAP_SUCCESS:
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "(%s) deleted successfully.", ppszRADNs[dwCnt]);
                break;

            default:
                VMDIR_LOG_VERBOSE( VMDIR_LOG_MASK_ALL, "(%s) deletion failed, error (%d).", ppszRADNs[dwCnt], dwError);
                break;
        }
    }

cleanup:
    for (dwCnt=0; dwCnt < dwSize; dwCnt++)
    {
        VMDIR_SAFE_FREE_MEMORY( ppszRADNs[dwCnt] );
    }
    VMDIR_SAFE_FREE_MEMORY(ppszRADNs);

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirDeleteReplAgreementToHost failed (%u)", dwError);

    goto cleanup;
}

VOID
VmDirFreeMetadata(
    PVMDIR_METADATA pMetadata
    )
{
    VmDirFreeMetadataInternal(pMetadata);
}

VOID
VmDirFreeMetadataList(
    PVMDIR_METADATA_LIST pMetadataList
    )
{
    VmDirFreeMetadataListInternal(pMetadataList);
}

DWORD
VmDirGetAttributeMetadata(
    PVMDIR_CONNECTION   pConnection,
    PCSTR               pszEntryDn,
    PCSTR               pszAttribute,
    PVMDIR_METADATA_LIST*    ppMetadataList
    )
{
    return VmDirGetAttributeMetadataInternal(pConnection, pszEntryDn, pszAttribute, ppMetadataList);
}

DWORD
VmDirGetServerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PSTR*             ppszGuid
    )
{
    DWORD dwError = 0;
    PSTR  pszGuid = NULL;

    if (!pConnection || !ppszGuid || IsNullOrEmptyString(pszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetServerGuidInternal(
                pConnection->pLd,
                pConnection->pszDomain,
                pszMachineName,
                &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;

cleanup:

    return dwError;

error:

    if (ppszGuid)
    {
        *ppszGuid = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGuid);

    goto cleanup;
}

DWORD
VmDirSetServerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PCSTR             pszGuid
    )
{
    DWORD dwError = 0;

    if (!pConnection ||
        IsNullOrEmptyString(pszGuid) ||
        IsNullOrEmptyString(pszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSetServerGuidInternal(
                pConnection->pLd,
                pConnection->pszDomain,
                pszMachineName,
                pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGetComputerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR              pszMachineName,
    PSTR*             ppszGuid
    )
{
    DWORD dwError = 0;
    PSTR  pszGuid = NULL;

    if (!pConnection || !ppszGuid || IsNullOrEmptyString(pszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirGetComputerGuidInternal(
                pConnection->pLd,
                pConnection->pszDomain,
                pszMachineName,
                &pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszGuid = pszGuid;

cleanup:

    return dwError;

error:

    if (ppszGuid)
    {
        *ppszGuid = NULL;
    }

    VMDIR_SAFE_FREE_MEMORY(pszGuid);

    goto cleanup;
}

DWORD
VmDirSetComputerID(
    PVMDIR_CONNECTION pConnection,
    PCSTR             pszMachineName,
    PCSTR             pszGuid
    )
{
    DWORD dwError = 0;

    if (!pConnection ||
        IsNullOrEmptyString(pszGuid) ||
        IsNullOrEmptyString(pszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSetComputerGuidInternal(
                pConnection->pLd,
                pConnection->pszDomain,
                pszMachineName,
                pszGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirLeaveFederation(
    PSTR pszServerName,
    PSTR pszUserName,
    PSTR pszPassword
    )
{
    DWORD dwError = 0;

    if (pszServerName)
    {
        dwError = _VmDirLeaveFederationOffline(pszServerName, pszUserName, pszPassword);
    } else
    {
        dwError = _VmDirLeaveFederationSelf(pszUserName, pszPassword);
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

/*
 * Make server (pszServerName) leave Federation Offline.
 * The server must be down to proceed.
 * The cleanup is performed on the local server
 * This function is tring to do cleanup in an idempotence way,
 * i.e, if the entry to be removed doesn't exist,
 * then it proceeds to clean up remaining entries associcated
 * with the pszServerName.
 */
static
DWORD
_VmDirLeaveFederationOffline(
    PCSTR pszServerName,
    PCSTR pszUserName,
    PCSTR pszPassword
    )
{
    DWORD dwError=0;
    PSTR  pszDomain = NULL;
    LDAP* pLD = NULL;
    PCSTR pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;
    DWORD i = 0;
    PSTR  pszLocalErrMsg = NULL;
    PSTR  pszDCAccount = NULL;
    PSTR  pszServerDN = NULL;

    dwError = VmDirRegReadDCAccount( &pszDCAccount );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainName( pszDCAccount, &pszDomain );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!_VmDirIsRemoteServerDown(pszServerName, pszUserName, pszPassword, pszDomain))
    {
         dwError = LDAP_OPERATIONS_ERROR;
         BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
             "You must shutdown domain controller/client %s before it can be removed from federation",
             pszServerName);
    }

    // Connect to local server
    dwError = _VmDirCreateServerPLD( pszDCAccount, pszDomain, pszUserName, pszPassword, &pLD);
    BAIL_ON_VMDIR_ERROR(dwError);
    if (pLD == NULL)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline: fail to bind to local server");
        dwError = LDAP_OPERATIONS_ERROR;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = _VmDirRemoveComputer(pLD, pszDomain, pszServerName);
    if (dwError == LDAP_SUCCESS)
    {
        //The server is a management node, done if if the computer is removed successfully.
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline: passed for domain client %s", pszServerName);
        goto cleanup;
    }
    else if (dwError != LDAP_NO_SUCH_OBJECT && dwError != VMDIR_ERROR_ENTRY_NOT_FOUND)
    {
        //Failed to remove the computer
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
             "fail to remove domain clinet %s", pszServerName);
    }
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
         "_VmDirLeaveFederationOffline: proceed to cleanup entries associated with domain controller %s", pszServerName);

    //The server is not a management node, then assume it is a domain controller

    // Remove entries associated with the server under Domain Controllers.
    // Bail if entry is not found since it is the first entry to be created on partner
    // during join.
    dwError = VmDirLdapDeleteDCAccount( pLD, pszDomain, pszServerName, FALSE, TRUE);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "fail to VmDirLdapDeleteDCAccount for domain controller %s", pszServerName)
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline: complete deleting DC account for domain controller %s", pszServerName);

    // Remove the server from the server tree.
    dwError = VmDirLdapCreateReplHostNameDN(&pszServerDN, pLD, pszServerName);
    if (dwError == 0 && pszServerDN)
    {
        //An example of the subtree to be deleted (cn=sea2-office-dhcp-97-183.eng.vmware.com,cn=Servers,...):
        //labeledURI=ldap://sea2-office-dhcp-97-124.eng.vmware.com,cn=Replication Agreements,cn=sea2-office-dhcp-97-183.eng.vmware.com,cn=Servers,...
        //cn=Replication Agreements,cn=sea2-office-dhcp-97-183.eng.vmware.com,cn=Servers,...
        //cn=sea2-office-dhcp-97-183.eng.vmware.com,cn=Servers,...
        dwError = VmDirDeleteDITSubtree( pLD, pszServerDN );
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
             "fail to delete subtree under %s", pszServerDN);
    }
    else
    {
        // DC does not have an entry under servers. Skip its delete and continue with cleanup.
        VMDIR_LOG_WARNING(VMDIR_LOG_MASK_ALL,
                          "%s: Failed to get server DN for domain controller %s with error (%u).",
                          __FUNCTION__,
                          pszServerName,
                          dwError);
        dwError = 0;
    }

    //Remove RAs that lead to the server (pszServerName).
    dwError = _VmDirDeleteReplAgreementToHost( pLD, pszServerName);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg, "fail to delete RA(s) to domain controller %s", pszServerName);
    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline: deleted RA(s) to domain controller %s", pszServerName);

    //Remove Service Accounts
    for (i = 0; i < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); i++)
    {
        dwError = VmDirLdapDeleteServiceAccount( pLD, pszDomain, pszServiceTable[i], pszServerName, TRUE);
        BAIL_ON_VMDIR_ERROR_WITH_MSG(dwError, pszLocalErrMsg,
                         "fail to delete Service Account %s associated with domain controller %s",
                         pszServiceTable[i], pszServerName);
        VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline: complete deleting Service Account %s associated with domain controller %s",
                         pszServiceTable[i], pszServerName);
    }

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline passed");

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VMDIR_SAFE_FREE_MEMORY(pszDCAccount);
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrMsg);
    VMDIR_SAFE_FREE_MEMORY(pszServerDN);
    if ( pLD )
    {
        ldap_unbind_ext_s(pLD, NULL, NULL);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirLeaveFederationOffline failed, %s (%u)", pszLocalErrMsg, dwError );
    goto cleanup;
}

/*
 * Create LDAP handle (binding to pszServerName)
 * that can be used for later ldap operations
 */
static
DWORD
_VmDirCreateServerPLD(
    PCSTR    pszServerName,
    PCSTR    pszDomain,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    LDAP**   ppLD
    )
{
    DWORD       dwError=0;
    PSTR        pszUPN=NULL;
    LDAP*       pLD = NULL;

    if ( !pszServerName || !ppLD || !pszPassword || !pszDomain )
    {
        dwError = VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringPrintf( &pszUPN, "%s@%s", pszUserName, pszDomain );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLD,
                                 pszServerName,
                                 pszUPN,
                                 pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppLD = pLD;

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszUPN );
    return dwError;

error:
    if ( pLD )
    {
        ldap_unbind_ext_s( pLD, NULL, NULL);
    }
    goto cleanup;
}

/*
 * Test if erver pszServerName is down
 */
static
BOOLEAN
_VmDirIsRemoteServerDown(
    PCSTR    pszServerName,
    PCSTR    pszUserName,
    PCSTR    pszPassword,
    PCSTR    pszDomain
    )
{
    DWORD       dwError=0;
    LDAP*       pLD = NULL;

    dwError = _VmDirCreateServerPLD(pszServerName, pszDomain, pszUserName, pszPassword, &pLD);
    VmDirLdapUnbind(&pLD);
    return dwError == VMDIR_ERROR_SERVER_DOWN;
}

/*
 * remove computer with name pszComputerHostName
 * used for management node cleanup
 */
static
DWORD
_VmDirRemoveComputer(
    LDAP *pLd,
    PCSTR pszDomainName,
    PCSTR pszComputerHostName
    )
{
    DWORD       dwError = 0;
    PSTR        pszComputerDN = NULL;
    PSTR        pszComputerUPN = NULL;

    dwError = VmDirAllocateStringPrintf(
                &pszComputerUPN,
                "%s@%s",
                pszComputerHostName,
                pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirConvertUPNToDN(
                pLd,
                pszComputerUPN,
                &pszComputerDN);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = ldap_delete_ext_s(pLd, pszComputerDN, NULL, NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszComputerDN);
    VMDIR_SAFE_FREE_MEMORY(pszComputerUPN);

    return dwError;

error:
    VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "_VmDirRemoveComputer (%s) failed with error (%u)",
                    VDIR_SAFE_STRING(pszComputerUPN), dwError);

    goto cleanup;
}

static
DWORD
_VmDirModDcPassword(
    PCSTR pszHostName,
    PCSTR pszUPN,
    PCSTR pszPassword,
    PCSTR pszMachineActDn,
    PBYTE pszNewPassword
    )
{
    DWORD    dwError = 0;
    LDAP*    pLD     = NULL;

    dwError = VmDirSafeLDAPBind(&pLD,
                                pszHostName,
                                pszUPN,
                                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapModReplaceAttribute(pLD,
                                           pszMachineActDn,
                                           ATTR_USER_PASSWORD,
                                           pszNewPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "Update account (%s) password on node (%s)", pszMachineActDn, pszHostName);

cleanup:
    if (pLD)
    {
        ldap_unbind_ext_s(pLD, NULL, NULL);
    }

    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "_VmDirModDcPassword failed (%u)", dwError);

    goto cleanup;
}

DWORD
VmDirGetDomainFunctionalLevel(
    PCSTR       pszHostName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PCSTR       pszDomainName,
    PDWORD      pdwFuncLvl
    )
{
    DWORD    dwError = 0;
    DWORD    dwFuncLvl = 0;
    char     bufUPN[VMDIR_MAX_UPN_LEN] = {0};
    LDAP*    pLd = NULL;

    if (!pszUserName || !pszPassword ||
        !pdwFuncLvl || !pszDomainName || !pszHostName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s",
                pszUserName, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(
                &pLd,
                pszHostName,
                bufUPN,
                pszPassword );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDomainFuncLvlInternal(
                pLd,
                pszDomainName,
                &dwFuncLvl);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!dwFuncLvl)
    {
        dwError = ERROR_NO_FUNC_LVL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwFuncLvl = dwFuncLvl;

cleanup:

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:

    if (pdwFuncLvl)
    {
        *pdwFuncLvl = 0;
    }

    goto cleanup;
}

DWORD
VmDirSetDomainFunctionalLevel(
    PCSTR       pszHostName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PCSTR       pszDomainName,
    PDWORD      pdwFuncLvl,
    BOOLEAN     bUseDefault
    )
{
    DWORD   dwError = 0;
    char    bufUPN[VMDIR_MAX_UPN_LEN] = {0};
    LDAP    *pLd = NULL;
    DWORD   dwCurrentDfl;
    PVMDIR_DC_VERSION_INFO    pDCVerInfo = NULL;

    if (!pszUserName || !pszPassword ||
        !pszDomainName || !pszHostName || !pdwFuncLvl )
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s",
                  pszUserName, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDCNodesVersion (
                pszHostName,
                pszUserName,
                pszPassword,
                pszDomainName,
                &pDCVerInfo );
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pDCVerInfo)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // default to max DFL.
    if (bUseDefault)
    {
        *pdwFuncLvl = pDCVerInfo->dwMaxDomainFuncLvl;
    }


    dwError = VmDirSafeLDAPBind(
                &pLd,
                pszHostName,
                bufUPN,
                pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Do not allow DFL downgrade.
    dwError = VmDirGetDomainFuncLvlInternal(
                pLd,
                pszDomainName,
                &dwCurrentDfl);
    BAIL_ON_VMDIR_ERROR(dwError);

    // verify that DFL is valid
    if ( *pdwFuncLvl <  dwCurrentDfl || *pdwFuncLvl > pDCVerInfo->dwMaxDomainFuncLvl)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL,
                "%s Invalid level (%d). Min (%d), Max (%d)",
                __FUNCTION__,
                *pdwFuncLvl,
                dwCurrentDfl,
                pDCVerInfo->dwMaxDomainFuncLvl);

        dwError = VMDIR_ERROR_INVALID_FUNC_LVL;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSetDomainFuncLvlInternal(
                pLd,
                pszDomainName,
                *pdwFuncLvl);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    if (pDCVerInfo)
    {
        VmDirFreeDCVersionInfo(pDCVerInfo);
    }

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:

    goto cleanup;
}

/*
 * Get node version from domain controller object.
 * This value does not exist in 5.5 and 6.0.  In such case, it tries to contact individual node to collect
 *   version information from DSEROOT entry.
 *
 * Both 5.5 and 6.0 map to DFL "1".
 */
DWORD
VmDirGetDCNodesVersion (
    PCSTR        pszHostName,
    PCSTR        pszUserName,
    PCSTR        pszPassword,
    PCSTR        pszDomainName,
    PVMDIR_DC_VERSION_INFO  *ppDCVerInfo
    )
{
    PVMDIR_DC_VERSION_INFO  pDCVerInfo = NULL;
    int     iCnt = 0, iIdx = 0;
    DWORD   dwError = 0;
    DWORD   dwCurDfl = 1;
    PSTR    pszDCContainerDN = NULL;
    PSTR    pszRemotePSCVerion = NULL;
    PCSTR   pszAttrCN = ATTR_CN;
    PCSTR   pszAttrPSCVer = ATTR_PSC_VERSION;
    PCSTR   pszAttrMaxDfl = ATTR_MAX_DOMAIN_FUNCTIONAL_LEVEL;
    PCSTR   ppszAttrs[] = { pszAttrPSCVer, pszAttrCN, pszAttrMaxDfl, NULL };

    PVMDIR_CONNECTION   pConnection = NULL;
    LDAPMessage*        pResult = NULL;
    LDAPMessage*        pEntry = NULL;
    struct berval**     ppPSCVerValues = NULL;
    struct berval**     ppCNValues = NULL;
    struct berval**     ppMaxDflValues = NULL;

    dwError = VmDirConnectionOpenByHost(
                pszHostName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDCContainerDN( pszDomainName, &pszDCContainerDN );
    BAIL_ON_VMDIR_ERROR(dwError);

    // query DC account objects
    dwError = ldap_search_ext_s(
                 pConnection->pLd,
                 pszDCContainerDN,
                 LDAP_SCOPE_ONE,
                 "objectclass=computer", /* filter */
                 (PSTR*)ppszAttrs,           /* attrs[]*/
                 FALSE,
                 NULL,              /* serverctrls */
                 NULL,              /* clientctrls */
                 NULL,              /* timeout */
                 0,
                 &pResult);
    BAIL_ON_VMDIR_ERROR(dwError);

    iCnt = ldap_count_entries( pConnection->pLd, pResult);
    if ( iCnt == 0 )
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateMemory(
                sizeof(VMDIR_DC_VERSION_INFO),
                (PVOID*)&pDCVerInfo);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                sizeof(PSTR)*iCnt,
                (PVOID*)&pDCVerInfo->ppszServer);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateMemory(
                sizeof(PSTR)*iCnt,
                (PVOID*)&pDCVerInfo->ppszVersion);
    BAIL_ON_VMDIR_ERROR(dwError);

    pDCVerInfo->dwSize = (DWORD)iCnt;
    pDCVerInfo->dwMaxDomainFuncLvl = 0;

    for ( pEntry = ldap_first_entry(pConnection->pLd, pResult), iIdx = 0;
          pEntry;
          pEntry = ldap_next_entry(pConnection->pLd, pEntry), iIdx++
        )
    {
        PSTR    pszThisVer = VMDIR_DFL_UNKNOWN;

        // handle CN (domain controller name in this context).
        if (ppCNValues)
        {
            ldap_value_free_len(ppCNValues);
        }

        ppCNValues = ldap_get_values_len(pConnection->pLd, pEntry, pszAttrCN);
        if (ppCNValues && ldap_count_values_len(ppCNValues) != 1)
        {
            dwError = ERROR_INVALID_STATE;
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        dwError = VmDirAllocateStringA(
                    ppCNValues[0]->bv_val,
                    &pDCVerInfo->ppszServer[iIdx] );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Retrieve Max DFL
        if (ppMaxDflValues)
        {
            ldap_value_free_len(ppMaxDflValues);
        }

        ppMaxDflValues = ldap_get_values_len(pConnection->pLd, pEntry, pszAttrMaxDfl);

        // handle PSC Version
        if (ppPSCVerValues)
        {
            ldap_value_free_len(ppPSCVerValues);
        }

        ppPSCVerValues = ldap_get_values_len(pConnection->pLd, pEntry, pszAttrPSCVer);
        if (ppPSCVerValues && ldap_count_values_len(ppPSCVerValues) == 1)
        {   // We have PSCVersion in domain controller,use it.
            pszThisVer = ppPSCVerValues[0]->bv_val;
        }
        else
        {
            VMDIR_SAFE_FREE_MEMORY(pszRemotePSCVerion);

            // best effort to get the version from individual node.
            // in 5.5. case, it returns "5.5".
            if ( VmDirPSCVersion( pDCVerInfo->ppszServer[iIdx],
                                       pszUserName,
                                       pszPassword,
                                       pszDomainName,
                                       &pszRemotePSCVerion) == 0 )
            {
                pszThisVer = pszRemotePSCVerion;
            }
        }

        dwError = VmDirAllocateStringA(
                    pszThisVer,
                    &pDCVerInfo->ppszVersion[iIdx] );
        BAIL_ON_VMDIR_ERROR(dwError);

        // Update max DFL to lower value
        // Use max dfl attr starting lightwave 1.1/vsphere 6.6
        if (ppMaxDflValues)
        {
            dwCurDfl = atoi(ppMaxDflValues[0]->bv_val);
        }
        else
        {
            dwError = VmDirMapVersionToMaxDFL(pszThisVer, &dwCurDfl );
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        // MaxDFL of zero has not been set.
        if ( pDCVerInfo->dwMaxDomainFuncLvl > dwCurDfl ||
             pDCVerInfo->dwMaxDomainFuncLvl == 0)
        {
            pDCVerInfo->dwMaxDomainFuncLvl = dwCurDfl;
        }
    }

    *ppDCVerInfo = pDCVerInfo;

cleanup:

    if (ppPSCVerValues)
    {
        ldap_value_free_len(ppPSCVerValues);
    }
    if (ppCNValues)
    {
        ldap_value_free_len(ppCNValues);
    }
    if (ppMaxDflValues)
    {
        ldap_value_free_len(ppMaxDflValues);
    }
    if (pResult)
    {
        ldap_msgfree(pResult);
    }

    VmDirConnectionClose(pConnection);
    VMDIR_SAFE_FREE_MEMORY(pszDCContainerDN);
    VMDIR_SAFE_FREE_MEMORY(pszRemotePSCVerion);

    return dwError;

error:

    if (pDCVerInfo)
    {
        VmDirFreeDCVersionInfo(pDCVerInfo);
    }

    goto cleanup;
}

DWORD
VmDirPSCVersion(
    PCSTR   pszHostName,
    PCSTR   pszUserName,
    PCSTR   pszPassword,
    PCSTR   pszDomainName,
    PSTR*   ppszVersion
    )
{
    DWORD dwError = 0;
    PSTR  pszVersion = NULL;
    char  bufUPN[VMDIR_MAX_UPN_LEN] = {0};
    LDAP* pLd = NULL;

    if (!pszUserName || !pszPassword || !ppszVersion ||
        !pszDomainName || !pszHostName)
    {
        BAIL_WITH_VMDIR_ERROR(dwError, ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirStringPrintFA( bufUPN, sizeof(bufUPN)-1,  "%s@%s",
                  pszUserName, pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind( &pLd,
                                 pszHostName,
                                 bufUPN,
                                 pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetPSCVersionInternal(
                pLd,
                &pszVersion);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszVersion = pszVersion;

cleanup:

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirOpenDatabaseFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    PCSTR       pszDBFileName,
    FILE **     ppFileHandle)
{
    PWSTR       pwszDBFileName = NULL;
    DWORD       dwError = 0;

    // parameter check
    if ( hBinding == NULL || IsNullOrEmptyString (pszDBFileName) || ppFileHandle == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringWFromA( pszDBFileName, &pwszDBFileName );
    BAIL_ON_VMDIR_ERROR(dwError);

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirOpenDatabaseFile(
                          hBinding->hBinding,
                          pwszDBFileName,
                          (vmdir_ftp_handle_t *) ppFileHandle );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pwszDBFileName);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirOpenDatabaseFile failed. Error[%d]\n", dwError );
    goto cleanup;
}

DWORD
VmDirReadDatabaseFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    FILE *      pFileHandle,
    UINT32 *    pdwCount,
    PBYTE       pReadBuffer,
    UINT32      bufferSize)
{
    DWORD   dwError = 0;
    VMDIR_DBCP_DATA_CONTAINER  readBufferContainer = {0};

    // parameter check
    if ( hBinding == NULL || pFileHandle == NULL || pdwCount == NULL || pReadBuffer == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError =  RpcVmDirReadDatabaseFile(
                           hBinding->hBinding,
                           pFileHandle,
                           bufferSize,
                           &readBufferContainer );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    if (readBufferContainer.dwCount > 0)
    {
        dwError = VmDirCopyMemory( pReadBuffer,
                               readBufferContainer.dwCount,
                               readBufferContainer.data,
                               readBufferContainer.dwCount);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwCount     = readBufferContainer.dwCount;

cleanup:

    VMDIR_RPC_FREE_MEMORY(readBufferContainer.data);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirReadDatabaseFile failed. Error[%d]\n", dwError );

    goto cleanup;
}

DWORD
VmDirCloseDatabaseFile(
    PVMDIR_SERVER_CONTEXT    hBinding,
    FILE **      ppFileHandle)
{
    DWORD dwError = 0;

    // parameter check
    if ( hBinding == NULL || ppFileHandle == NULL || *ppFileHandle == NULL )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    VMDIR_RPC_TRY
    {
        dwError = RpcVmDirCloseDatabaseFile(
                          hBinding->hBinding,
                          (vmdir_ftp_handle_t *) ppFileHandle );
        *ppFileHandle = NULL;
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirCloseDatabaseFile failed. Error[%d]\n", dwError );
    goto cleanup;
}

/* dwFileTransferState [in]:
 *   1 - set partner's backend to READONLY state
 *   2 - set  partner's backend to keep xlogs state (used for hot database file copy only)
 *   0 - clear partner's backend MDB_KEEPXLOGS flag or READONLY state
 * pdwLogNum [out]:
 *   The starting transaction log number if dwFileTransferState is 2 and partner support WAL;
 *   if dwFileTransferState is 2 but partner doesn't support WAL, then the partner would be put at READONLY mode, and pdwLogNum set to 0.
 * pdwDbSizeMb [out]: assigned size of the partner's backend database file in MB.
 * pDbPath [out]: the partner's database home path
 * dwDbPathSize [in]: the memory size of pDbPath.
 * Return:  0 success
 *   If the RPC call went through, the remote function may return non-zero indicating the following errors:
 *          1 function failed - invalid parameter
 *          2 function failed - invalid state, i.e.
 *            already in read-only when trying to set to read-only or not in read-only while trying to clear read-only state.
 *          3 function failed - db_path buffer too small
 */
DWORD
VmDirSetBackendState(
    PVMDIR_SERVER_CONTEXT    hBinding,
    UINT32     dwFileTransferState,
    UINT32     *pdwLogNum,
    UINT32     *pdwDbSizeMb,
    UINT32     *pdwDbMapSizeMb,
    PBYTE      pDbPath,
    UINT32     dwDbPathSize)
{
    DWORD    dwError = 0;
    UINT32   xlognum = 0;
    UINT32   dbSizeMb = 0;
    UINT32   dbMapSizeMb = 0;
    VMDIR_DBCP_DATA_CONTAINER readBufferContainer = {0};

    // parameter check
    if ( hBinding == NULL || pdwLogNum == NULL || pDbPath == NULL || dwDbPathSize == 0)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    *pdwLogNum = 0;
    *pdwDbSizeMb = 0;
    *pdwDbMapSizeMb = 0;
    VMDIR_RPC_TRY
    {
        dwError =  RpcVmDirSetBackendState( hBinding->hBinding, dwFileTransferState, &xlognum, &dbSizeMb,
                                    &dbMapSizeMb, &readBufferContainer, dwDbPathSize);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCopyMemory( pDbPath, readBufferContainer.dwCount,
                               readBufferContainer.data, readBufferContainer.dwCount);
    BAIL_ON_VMDIR_ERROR(dwError);
    *pdwLogNum = xlognum;
    *pdwDbSizeMb = dbSizeMb;
    *pdwDbMapSizeMb = dbMapSizeMb;

cleanup:
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetBackendState failed (%u)", dwError);
    goto cleanup;
}

VOID
VmDirFreeStringArray(
    PSTR* ppszStr,
    DWORD size)
{
    if (ppszStr)
    {
        DWORD idx = 0;
        for (; idx < size; ++idx)
        {
            VMDIR_SAFE_FREE_STRINGA(ppszStr[idx]);
        }
        VMDIR_SAFE_FREE_MEMORY(ppszStr);
    }
}

/*
 * Schema requirements:
 *     Update existing schema to my expected definition if necessary.
 *
 * DFL requirements:
 * TBD
 */
static
DWORD
_VmDirJoinPreCondition(
    PCSTR       pszHostName,
    PCSTR       pszDomainName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PSTR*       ppszErrMsg
    )
{
    DWORD   dwError = 0;
    PSTR    pszVersion = NULL;
    PSTR    pszSchemaFile = NULL;
    PVMDIR_CONNECTION   pConnection = NULL;
    PVDIR_LDAP_SCHEMA   pFileSchema = NULL;
    PSTR    pszErrMsg = NULL;
    DWORD   dwDfl = 0;
#ifndef LIGHTWAVE_BUILD
    int     iVerCmp65 = 0;
#endif

    // open connection to remote node
    dwError = VmDirConnectionOpenByHost(
                pszHostName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get domain functional level from remote node
    dwError = VmDirGetDomainFuncLvlInternal(pConnection->pLd, pszDomainName, &dwDfl);
    BAIL_ON_VMDIR_ERROR(dwError);

    // Make sure local node can support DFL
    if (VMDIR_MAX_DFL < dwDfl)
    {
        dwError = VmDirAllocateStringPrintf(&pszErrMsg,
                        "Maximum functional level (%u) is less than "
                        "domain functional level (%u)",
                        VMDIR_MAX_DFL,
                        dwDfl);
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_FUNC_LVL);
    }

    // get file schema
    dwError = VmDirGetDefaultSchemaFile(&pszSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaInit(&pFileSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadFile(pFileSchema, pszSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get PSC version of remote node
    dwError = VmDirGetPSCVersionInternal(pConnection->pLd, &pszVersion);
    BAIL_ON_VMDIR_ERROR(dwError);

#ifndef LIGHTWAVE_BUILD
    // For PSC build 6.5 and before.
    // patch remote node so its schema is union of itself and file
    iVerCmp65 = VmDirCompareVersion(pszVersion, "6.5");
    if (iVerCmp65 < 0)
    {
        dwError = VmDirAllocateStringPrintf(&pszErrMsg,
                "Partner version %s < 6.5.0. "
                "Join time schema upgrade is not supported",
                pszVersion);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VMDIR_ERROR_SCHEMA_NOT_COMPATIBLE;
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else if (iVerCmp65 == 0)
    {
        dwError = VmDirPatchRemoteSubSchemaSubEntry(
                pConnection->pLd, pFileSchema);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
#endif
    {   // Lightwave or PSC(>=6.6) with new schema object model to support down an up version join
        dwError = VmDirPatchRemoteSchemaObjects(
                pConnection->pLd, pFileSchema);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:
    VmDirConnectionClose(pConnection);
    VMDIR_SAFE_FREE_MEMORY(pszSchemaFile);
    VMDIR_SAFE_FREE_MEMORY(pszVersion);
    VmDirFreeLdapSchema(pFileSchema);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "[%s] error (%d), msg (%s)",
            __FUNCTION__, dwError, VDIR_SAFE_STRING(pszErrMsg) );

    if (ppszErrMsg && pszErrMsg)
    {
        *ppszErrMsg = pszErrMsg;
        pszErrMsg = NULL;
    }
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    goto    cleanup;
}

DWORD
VmDirUrgentReplicationRequest(
    PCSTR pszRemoteServerName
    )
{
    return VMDIR_ERROR_DEPRECATED_FUNCTION;
}

DWORD
VmDirUrgentReplicationResponse(
    PCSTR    pszRemoteServerName,
    PCSTR    pszUtdVector,
    PCSTR    pszInvocationId,
    PCSTR    pszHostName
    )
{
   return VMDIR_ERROR_DEPRECATED_FUNCTION;
}
