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

DWORD
_VmDirUpdateKeytabFile(
    PCSTR pszServerName,
    PCSTR pszDomainName,
    PCSTR pszHostName,
    PCSTR pszUserName,
    PCSTR pszPassword,
    BOOLEAN bIsServer);

static
DWORD
_VmDirLdapCheckVmDirStatus(
    PCSTR pszPartnerHostName
    );

static
DWORD
_VmDirSetupDefaultAccount(
    PCSTR pszDomainName,
    PCSTR pszPartnerServerName,
    PCSTR pszLdapHostName,
    PCSTR pszBindUserName,
    PCSTR pszBindPassword
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
_VmDirJoinPreCondition(
    PCSTR       pszHostName,
    PCSTR       pszDomainName,
    PCSTR       pszUserName,
    PCSTR       pszPassword,
    PSTR*       ppszErrMsg
    );

DWORD
VmDirRaftRequestVote(
    PVMDIR_SERVER_CONTEXT    hBinding,
    /* [in] */ UINT32 term,
    /* [in] */ char *candidateId,
    /* [in] */ unsigned long long lastLogIndex,
    /* [in] */ UINT32 lastLogTerm,
    /* [out] */ UINT32 *currentTerm,
    /* [out] */ UINT32 *voteGranted
    );

DWORD
VmDirRaftAppendEntries(
    PVMDIR_SERVER_CONTEXT    hBinding,
    /* [in] */               UINT32 term,
    /* [in, string] */       char * leader,
    /* [in] */               unsigned long long preLogIndex,
    /* [in] */               UINT32 prevLogTerm,
    /* [in] */               unsigned long long leaderCommit,
                             int entriesSize,
    /* [in] */               unsigned char *entries,
    /* [out] */              UINT32 * currentTerm,
    /* [out] */              unsigned long long *status
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

    dwError = VmDirDomainNameToDN( pszDomain, &pszDomainDN);
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
    if ( dwError == VMDIR_ERROR_NO_SUCH_ATTRIBUTE)
    {
        dwError = 0;
    }
    else
    {
        iExpInDays = atoi(pszExpInDays);
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
        dwError = VmDirDomainNameToDN(pszDomainName, &pszDomainDN);
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
    PVM_DIR_CONNECTION pIPCConnection = NULL;

    if (VmDirOpenClientConnection(&pIPCConnection) != 0)
    {   // POST is not listen on IPC port
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
    }

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
    dwError = _VmDirSetupDefaultAccount(
                        pszDomainName,
                        pszLotusServerNameCanon,    // Partner is self in this 1st instance case.
                        pszLotusServerNameCanon,
                        pszUserName,
                        pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

	VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "VmDirSetupHostInstance (%s)(%s)(%s) passed",
                                        VDIR_SAFE_STRING(pszDomainName),
                                        VDIR_SAFE_STRING(pszSiteName),
                                        VDIR_SAFE_STRING(pszLotusServerNameCanon) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY( pszLotusServerNameCanon );
    VmDirCloseClientConnection(pIPCConnection);
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirSetupHostInstance failed. Error(%u)", dwError);
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
    PSTR    pszErrMsg = NULL;
    LDAP*   pLd = NULL;
    PVMDIR_REPL_STATE pReplState = NULL;
    PVM_DIR_CONNECTION pIPCConnection = NULL;

    if (IsNullOrEmptyString(pszUserName) ||
        IsNullOrEmptyString(pszPassword) ||
        IsNullOrEmptyString(pszPartnerHostName))
    {
        dwError =  VMDIR_ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirOpenClientConnection(&pIPCConnection) != 0)
    {   // POST is not listen on IPC port
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_UNAVAILABLE);
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

    pszPartnerServerName = (PSTR)pszPartnerHostName;
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

    if (VmDirRaftServerExists(
                pszPartnerServerName,
                pszDomainName,
                pszUserName,
                pszPassword,
                pszLotusServerNameCanon) == TRUE)
    {
        VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s, raft server (%s) exists already.",
                                             __FUNCTION__,
                                             VDIR_SAFE_STRING(pszLotusServerNameCanon));
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_ALREADY_PROMOTED);
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

    dwError = _VmDirSetupDefaultAccount(
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

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL,
                    "VmDirJoin (%s)(%s)(%s) passed",
                    VDIR_SAFE_STRING(pszPartnerHostName),
                    VDIR_SAFE_STRING(pszSiteName),
                    VDIR_SAFE_STRING(pszLotusServerNameCanon) );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(pszLotusServerNameCanon);
    VMDIR_SAFE_FREE_MEMORY(pszErrMsg);
    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }
    VmDirFreeReplicationStateInternal(pReplState);
    VmDirCloseClientConnection(pIPCConnection);
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
        dwError =  ERROR_INVALID_PARAMETER;
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
    //VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "VmDirGetState failed. Error[%d]\n", dwError );
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
    DWORD       dwLdapPort = DEFAULT_LDAP_PORT_NUM;
    DWORD       dwTmpLdapPort = 0;

    if (!IsNullOrEmptyString(pszPartnerHostName))
    {
        bFirst = FALSE;
        dwTimeout = -1; //infinite minutes for 2nd Ldu, because we could be copying really big DB from partner.
    }

    if (VmDirGetRegKeyValueDword(
                VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                VMDIR_REG_KEY_LDAP_PORT,
                &dwTmpLdapPort,
                DEFAULT_LDAP_PORT_NUM) == ERROR_SUCCESS)
    {
        dwLdapPort = dwTmpLdapPort;
    }

    dwError = VmDirAllocateStringPrintf( &pszLocalServerReplURI, "%s://localhost:%d",
                                             VMDIR_LDAP_PROTOCOL, dwLdapPort );
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
            VmDirSleep(2000);
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
static
DWORD
_VmDirSetupDefaultAccount(
    PCSTR pszDomainName,
    PCSTR pszPartnerServerName,
    PCSTR pszLdapHostName,
    PCSTR pszBindUserName,
    PCSTR pszBindPassword
    )
{
    DWORD   dwError = 0;
//    PCSTR   pszServiceTable[] = VMDIR_DEFAULT_SERVICE_PRINCIPAL_INITIALIZER;
//    int     iCnt = 0;

    dwError = VmDirLdapSetupDCAccountOnPartner(
                                    pszDomainName,
                                    pszPartnerServerName,
                                    pszBindUserName,
                                    pszBindPassword,
                                    pszLdapHostName );
    BAIL_ON_VMDIR_ERROR(dwError);

/*
    for (iCnt = 0; iCnt < sizeof(pszServiceTable)/sizeof(pszServiceTable[0]); iCnt++)
    {
        dwError = VmDirLdapSetupServiceAccount(
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
*/

    VMDIR_LOG_INFO( VMDIR_LOG_MASK_ALL, "_VmDirSetupKrbAccount (%s)(%s) passed",
                                        VDIR_SAFE_STRING(pszDomainName),
                                        VDIR_SAFE_STRING(pszPartnerServerName) );
cleanup:

    return dwError;

error:

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
                          (vmdir_dbcp_handle_t *) ppFileHandle );
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
                          (vmdir_dbcp_handle_t *) ppFileHandle );
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

/* @brief See lmdb.h for the parameters
 */
DWORD
VmDirSetBackendState(
    PVMDIR_SERVER_CONTEXT    hBinding,
    MDB_state_op op,
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
        dwError =  RpcVmDirSetBackendState( hBinding->hBinding, op, &xlognum, &dbSizeMb,
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
    PSTR    pszSchemaFile = NULL;
    PVMDIR_CONNECTION   pConnection = NULL;
    PVDIR_LDAP_SCHEMA   pCurSchema = NULL;
    PVDIR_LDAP_SCHEMA   pNewSchema = NULL;
    PVDIR_LDAP_SCHEMA_DIFF  pSchemaDiff = NULL;
    PSTR    pszErrMsg = NULL;

    // open connection to remote node
    dwError = VmDirConnectionOpenByHost(
                pszHostName,
                pszDomainName,
                pszUserName,
                pszPassword,
                &pConnection);
    BAIL_ON_VMDIR_ERROR(dwError);

    // get remote schema (tree)
    dwError = VmDirLdapSchemaInit(&pCurSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadRemoteSchema(pCurSchema, pConnection->pLd);
    BAIL_ON_VMDIR_ERROR(dwError);

    // try loading file
    dwError = VmDirLdapSchemaCopy(pCurSchema, &pNewSchema);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetDefaultSchemaFile(&pszSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirLdapSchemaLoadFile(pNewSchema, pszSchemaFile);
    BAIL_ON_VMDIR_ERROR(dwError);

    // compute diff
    dwError = VmDirLdapSchemaGetDiff(pCurSchema, pNewSchema, &pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

    // perform patch
    dwError = VmDirPatchRemoteSchemaObjects(pConnection->pLd, pSchemaDiff);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VmDirConnectionClose(pConnection);
    VMDIR_SAFE_FREE_MEMORY(pszSchemaFile);
    VmDirFreeLdapSchema(pCurSchema);
    VmDirFreeLdapSchema(pNewSchema);
    VmDirFreeLdapSchemaDiff(pSchemaDiff);
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
VmDirGetMode(
    PVMDIR_SERVER_CONTEXT hInBinding,
    UINT32*               pdwMode)
{
    DWORD dwError = 0;
    PCSTR pszServerName = "localhost";
    PCSTR pszServerEndpoint = NULL;
    handle_t hBinding = NULL;
    UINT32  dwMode = 0;

    if (!pdwMode)
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
        dwError = RpcVmDirGetMode(
                          hBinding,
                          &dwMode
                          );
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *pdwMode = dwMode;

cleanup:
    if (!hInBinding && hBinding)
    {
        VmDirFreeBindingHandle( &hBinding);
    }
    return dwError;

error:
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s failed. Error[%d]\n", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirSetMode(
    PVMDIR_SERVER_CONTEXT hInBinding,
    UINT32                dwMode)
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
        dwError = RpcVmDirSetMode(
                          hBinding,
                          dwMode
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
    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL, "%s Error[%d]\n", __FUNCTION__, dwError );
    goto cleanup;
}

DWORD
VmDirRaftAppendEntries(
    PVMDIR_SERVER_CONTEXT    hBinding,
    /* [in] */               UINT32 term,
    /* [in, string] */       char * leader,
    /* [in] */               unsigned long long preLogIndex,
    /* [in] */               UINT32 prevLogTerm,
    /* [in] */               unsigned long long leaderCommit,
                             int entriesSize,
    /* [in] */               unsigned char *entries,
    /* [out] */              UINT32 * currentTerm,
    /* [out] */              unsigned long long *status
    )
{
    DWORD dwError = 0;
    UINT32 iCurrentTerm = 0;
    idl_uhyper_int iStatus = 0;
    chglog_container chglogEntries = {0};
    chglogEntries.chglog_size = entriesSize;
    chglogEntries.chglog_bytes = entries;

    *currentTerm = 0;
    *status = 0;

    VMDIR_RPC_TRY
    {
        dwError =  RpcVmDirRaftAppendEntries(hBinding->hBinding, term, (idl_char *)leader, preLogIndex,
                                             prevLogTerm, leaderCommit, &chglogEntries, &iCurrentTerm, &iStatus);
    }
    VMDIR_RPC_CATCH
    {
        VMDIR_RPC_GETERROR_CODE(dwError);
    }
    VMDIR_RPC_ENDTRY;
    BAIL_ON_VMDIR_ERROR(dwError);

    *currentTerm = iCurrentTerm;
    *status = iStatus;

cleanup:
    return dwError;

error:
    goto cleanup;
}


DWORD
VmDirRaftRequestVote(
    PVMDIR_SERVER_CONTEXT    hBinding,
    /* [in] */ UINT32 term,
    /* [in] */ char candidateId[],
    /* [in] */ unsigned long long lastLogIndex,
    /* [in] */ UINT32 lastLogTerm,
    /* [out] */ UINT32 *currentTerm,
    /* [out] */ UINT32 *voteGranted
)
{
    DWORD   dwError = 0;

    VMDIR_RPC_TRY
    {
        dwError =  RpcVmDirRaftRequestVote( hBinding->hBinding, term, (idl_char *)candidateId,
                                            lastLogIndex, lastLogTerm, currentTerm, voteGranted);
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
    goto cleanup;
}
