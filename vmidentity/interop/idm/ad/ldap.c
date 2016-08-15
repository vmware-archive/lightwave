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
 * Module Name:
 *
 *        ldap.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        LDAP support
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *          Jonathan Brown  (brownj@vmware.com)
 *
 */

#include "includes.h"

static
IDM_KRB_CONTEXT_STATE
IDMKrbGetState(
    PIDM_KRB_CONTEXT pKrbContext
    );

static
DWORD
IDMKrbSetState(
    PIDM_KRB_CONTEXT pKrbContext,
    IDM_KRB_CONTEXT_STATE state
    );

static
int
IDMSASLInteraction(
    LDAP* pLD,
    DWORD dwFlags,
    PVOID pDefaults,
    PVOID pIn
    );

static
DWORD
IDMKrbDetermineJoinState(
    PIDM_KRB_CONTEXT pKrbContext
    );

static
DWORD
IDMKrbRenewCredentials(
    PIDM_KRB_CONTEXT pKrbContext,
    PWSTR pszPassword
    );

static
DWORD
IDMKrbGetPrincipal(
    PCSTR pszAccount,
    PCSTR pszDomain,
    PSTR* ppszPrincipal
    );

DWORD
IDMLdapSaslBind(
    LDAP* pLd,                       /* IN     */
    PWSTR pszUser,                   /* IN     */
    PWSTR pszDomain,                 /* IN     */
    PWSTR pszPassword                /* IN     */
    )
{
    DWORD dwError = 0;
    DWORD dwCleanupError = 0;
    BOOLEAN bLocked = FALSE;
    PSTR  pszCachePath = NULL;
    PSTR  user = NULL;
    PSTR  domain = NULL;

    if (!pLd)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    switch (IDMKrbGetState(pgIdmKrbContext))
    {
        case IDM_KRB_CONTEXT_STATE_INITIAL:

            dwError = IDMKrbDetermineJoinState(pgIdmKrbContext);
            BAIL_ON_ERROR(dwError);

            // If the user is provided, use the user's credential.
            if(pszUser) {
                dwError = LwRtlCStringAllocateFromWC16String(&user, pszUser);
                BAIL_ON_ERROR(dwError);

                dwError = LwRtlCStringAllocateFromWC16String(&domain, pszDomain);
                BAIL_ON_ERROR(dwError);

                IDM_RWMUTEX_LOCK_EXCLUSIVE(&pgIdmKrbContext->mutex_rw, bLocked, dwError);
                BAIL_ON_ERROR(dwError);

                IDM_SAFE_FREE_MEMORY(pgIdmKrbContext->pszAccount);
                pgIdmKrbContext->pszAccount = user;
                user = NULL;

                IDM_SAFE_FREE_MEMORY(pgIdmKrbContext->pszDomain);
                pgIdmKrbContext->pszDomain = domain;
                domain = NULL;

                IDM_RWMUTEX_UNLOCK(&pgIdmKrbContext->mutex_rw, bLocked, dwError);
                BAIL_ON_ERROR(dwError);
            }

        case IDM_KRB_CONTEXT_STATE_JOINED:

            dwError = IDMKrbRenewCredentials(pgIdmKrbContext, pszUser? pszPassword: NULL); // If the user is provided, use the user's credential.
            if (dwError)
            {
               // Refreshing credentials might fail if the system left
               // the domain or there were issues reaching the domain
               // controller.
               IDMKrbSetState(pgIdmKrbContext, IDM_KRB_CONTEXT_STATE_INITIAL);
            }
            BAIL_ON_ERROR(dwError);

            IDM_RWMUTEX_LOCK_SHARED(
                &pgIdmKrbContext->mutex_rw,
                bLocked,
                dwError);
            BAIL_ON_ERROR(dwError);

            dwError = LwKrb5SetThreadDefaultCachePath(
                            pgIdmKrbContext->pszCachePath,
                            &pszCachePath);
            BAIL_ON_ERROR(dwError);

            dwError = LwMapLdapErrorToLwError(
                        ldap_sasl_interactive_bind_s(
                            pLd,
                            NULL,
                            "GSSAPI",
                            NULL,
                            NULL,
                            LDAP_SASL_QUIET,
                            &IDMSASLInteraction,
                            NULL));
            BAIL_ON_ERROR(dwError);

            break;

        default:

            dwError = ERROR_INVALID_STATE;
            BAIL_ON_ERROR(dwError);

            break;
    }

cleanup:
    if (pszCachePath)
    {
        LwKrb5SetThreadDefaultCachePath(pszCachePath, NULL);

        LwFreeMemory(pszCachePath);
    }

    IDM_RWMUTEX_UNLOCK(&pgIdmKrbContext->mutex_rw, bLocked, dwCleanupError);
    if(!dwError)
    {
        dwError = dwCleanupError;
    }

    IDM_SAFE_FREE_MEMORY(user);

    IDM_SAFE_FREE_MEMORY(domain);

    return dwError;

error:

    goto cleanup;
}

static
IDM_KRB_CONTEXT_STATE
IDMKrbGetState(
    PIDM_KRB_CONTEXT pKrbContext
    )
{
    DWORD dwError = 0;
    BOOLEAN bLocked = FALSE;
    IDM_KRB_CONTEXT_STATE state = IDM_KRB_CONTEXT_STATE_UNKNOWN;

    IDM_RWMUTEX_LOCK_SHARED(&pKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    state = pKrbContext->state;

cleanup:

    IDM_RWMUTEX_UNLOCK(&pKrbContext->mutex_rw, bLocked, dwError);

    return state;

error:

    goto cleanup;
}

static
DWORD
IDMKrbSetState(
    PIDM_KRB_CONTEXT      pKrbContext,
    IDM_KRB_CONTEXT_STATE state
    )
{
    DWORD dwError = 0;
    DWORD dwCleanupError = 0;
    BOOLEAN bLocked = FALSE;

    IDM_RWMUTEX_LOCK_EXCLUSIVE(&pKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    pKrbContext->state = state;

cleanup:

    IDM_RWMUTEX_UNLOCK(&pKrbContext->mutex_rw, bLocked, dwCleanupError);
    if(!dwError)
    {
        dwError = dwCleanupError;
    }
    return dwError;

error:

    goto cleanup;
}

static
int
IDMSASLInteraction(
    LDAP* pLD,
    DWORD dwFlags,
    PVOID pDefaults,
    PVOID pIn
    )
{
    return LDAP_SUCCESS;
}

static
DWORD
IDMKrbDetermineJoinState(
    PIDM_KRB_CONTEXT pKrbContext
    )
{
    DWORD dwError = 0;
    DWORD dwCleanupError = 0;
    HANDLE hLsa = NULL;
    PLSA_MACHINE_ACCOUNT_INFO_A pAcctInfo = NULL;
    PSTR pszAccount = NULL;
    PSTR pszDomain = NULL;
    BOOLEAN bLocked = FALSE;

    dwError = LsaOpenServer(&hLsa);
    BAIL_ON_ERROR(dwError);

    dwError = LsaAdGetMachineAccountInfo(hLsa, NULL, &pAcctInfo);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateStringA(
                    pAcctInfo->DnsDomainName,
                    &pszDomain);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateStringA(
                    pAcctInfo->SamAccountName,
                    &pszAccount);
    BAIL_ON_ERROR(dwError);

    IDM_RWMUTEX_LOCK_EXCLUSIVE(&pKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    if (pKrbContext->state == IDM_KRB_CONTEXT_STATE_INITIAL)
    {
        IDM_SAFE_FREE_MEMORY(pKrbContext->pszAccount);
        pKrbContext->pszAccount = pszAccount;
        pszAccount = NULL;

        IDM_SAFE_FREE_MEMORY(pKrbContext->pszDomain);
        pKrbContext->pszDomain = pszDomain;
        pszDomain = NULL;

        pKrbContext->expiryTime = 0;
        pKrbContext->state = IDM_KRB_CONTEXT_STATE_JOINED;
    }

cleanup:

    IDM_RWMUTEX_UNLOCK(&pKrbContext->mutex_rw, bLocked, dwCleanupError);

    IDM_SAFE_FREE_MEMORY(pszAccount);
    IDM_SAFE_FREE_MEMORY(pszDomain);

    if (pAcctInfo)
    {
        LsaAdFreeMachineAccountInfo(pAcctInfo);
    }
    if (hLsa)
    {
        LsaCloseServer(hLsa);
    }

    if(!dwError)
    {
        dwError = dwCleanupError;
    }
    return dwError;

error:

    dwError = ERROR_NOT_JOINED;

    goto cleanup;
}

static
DWORD
IDMKrbRenewCredentials(
    PIDM_KRB_CONTEXT pKrbContext,
    PWSTR password
    )
{
    DWORD   dwError = 0;
    DWORD   dwCleanupError = 0;
    BOOLEAN bLocked = FALSE;
    BOOLEAN bRefreshCreds = FALSE;
    krb5_context   pCtx   = NULL;
    PSTR           pszPrincipal = NULL;
    krb5_principal pPrincipal = NULL;
    krb5_keytab    ktid   = 0;
    krb5_ccache    pCache = NULL;
    krb5_creds     creds = {0};
    PSTR pszPassword = NULL;

    IDM_RWMUTEX_LOCK_SHARED(&pKrbContext->mutex_rw, bLocked, dwError);
    BAIL_ON_ERROR(dwError);

    if (!pKrbContext->expiryTime)
    {
        bRefreshCreds = TRUE;
    }
    else
    {
        krb5_timestamp now = time(NULL);

        if (now >= pKrbContext->expiryTime)
        {
            bRefreshCreds = TRUE;
        }
        else
        {
            double threshold = (30 * 60); // 30 minutes
            double interval = difftime(pKrbContext->expiryTime, now);

            if (interval <= threshold)
            {
                bRefreshCreds = TRUE;
            }
        }
    }

    if (bRefreshCreds)
    {
        krb5_error_code errKrb = 0;
        krb5_deltat    startTime = 0;
        krb5_get_init_creds_opt options = {0};
        krb5_timestamp origExpiryTime = pKrbContext->expiryTime;

        IDM_RWMUTEX_UNLOCK(&pKrbContext->mutex_rw, bLocked, dwError);
        BAIL_ON_ERROR(dwError);

        dwError = IDMKrbGetPrincipal(
                      pKrbContext->pszAccount,
                      pKrbContext->pszDomain,
                      &pszPrincipal);
        BAIL_ON_ERROR(dwError);

        errKrb = krb5_init_context(&pCtx);
        BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

        errKrb = krb5_parse_name(pCtx, pszPrincipal, &pPrincipal);
        BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

        errKrb = krb5_kt_default(pCtx, &ktid);
        BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

        krb5_get_init_creds_opt_init(&options);
        krb5_get_init_creds_opt_set_forwardable(&options, TRUE);

        if (password)
        {
            dwError = LwRtlCStringAllocateFromWC16String(
                        &pszPassword,
                        password);
            BAIL_ON_ERROR(dwError);

            errKrb = krb5_get_init_creds_password(
                    pCtx,
                    &creds,
                    pPrincipal,
                    pszPassword,
                    NULL,
                    NULL,
                    startTime,
                    NULL, // the default host/ service principal.
                    &options);
        }
        else
        {
            errKrb = krb5_get_init_creds_keytab(
                    pCtx,
                    &creds,
                    pPrincipal,
                    ktid,
                    startTime,
                    NULL, // the default host/ service principal.
                    &options);
        }
        BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

        IDM_RWMUTEX_LOCK_EXCLUSIVE(&pKrbContext->mutex_rw, bLocked, dwError);
        BAIL_ON_ERROR(dwError);

        if (pKrbContext->expiryTime <= origExpiryTime)
        {
            errKrb = krb5_cc_resolve(pCtx, pKrbContext->pszCachePath, &pCache);
            BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

            errKrb = krb5_cc_initialize(pCtx, pCache, pPrincipal);
            BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

            errKrb = krb5_cc_store_cred(pCtx, pCache, &creds);
            BAIL_ON_KERBEROS_ERROR(NULL, errKrb, dwError);

            pKrbContext->expiryTime = creds.times.endtime;
        }
    }

cleanup:

    IDM_RWMUTEX_UNLOCK(&pKrbContext->mutex_rw, bLocked, dwCleanupError);

    if (pCtx)
    {
        if (ktid)
        {
            krb5_kt_close(pCtx, ktid);
            ktid = 0;
        }
        if (pPrincipal)
        {
            if (creds.client == pPrincipal)
            {
                creds.client = NULL;
            }

            krb5_free_principal(pCtx, pPrincipal);
        }

        krb5_free_cred_contents(pCtx, &creds);

        if (pCache)
        {
            krb5_cc_close(pCtx, pCache);
        }

        krb5_free_context(pCtx);
    }

    IDM_SAFE_FREE_MEMORY(pszPrincipal);

    if(!dwError)
    {
        dwError = dwCleanupError;
    }

    IDM_SAFE_FREE_MEMORY(pszPassword);
    return dwError;

error:

    goto cleanup;
}

static
DWORD
IDMKrbGetPrincipal(
    PCSTR pszAccount,
    PCSTR pszDomain,
    PSTR* ppszPrincipal
    )
{
    DWORD dwError = 0;
    PSTR  pszPrincipal = NULL;
    PSTR  pszCursor = NULL;

    dwError = IDMAllocateStringPrintf(
                    &pszPrincipal,
                    "%s@%s",
                    pszAccount,
                    pszDomain);
    BAIL_ON_ERROR(dwError);

    pszCursor = pszPrincipal + strlen(pszAccount);
    while (pszCursor && *pszCursor)
    {
        *pszCursor = toupper((int)*pszCursor);
        pszCursor++;
    }

    *ppszPrincipal = pszPrincipal;

cleanup:

    return dwError;

error:

    *ppszPrincipal = NULL;

    IDM_SAFE_FREE_MEMORY(pszPrincipal);

    goto cleanup;
}

