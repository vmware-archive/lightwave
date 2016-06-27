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
 *        gssapi-sspi.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        GSSAPI SSPI
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"

#define IDM_MAX_HOSTNAME_LEN 256
#define IDM_GSS_SERVICE_NAME "host"
#define IDM_SID_LOOKUP_BATCH_SIZE 1000

static
int
server_acquire_creds(
    char *service_name,
    gss_cred_id_t *server_creds,
    OM_uint32 *ret_min_stat)
{
    gss_buffer_desc name_buf;
    gss_name_t server_name;
    OM_uint32 maj_stat = 0, min_stat = 0;

    name_buf.value = service_name;
    name_buf.length = strlen(name_buf.value) + 1;
    maj_stat = gss_import_name(&min_stat, &name_buf,
                               (gss_OID) gss_nt_service_name, &server_name);
    if (maj_stat != GSS_S_COMPLETE) {
        *ret_min_stat = min_stat;
        return maj_stat;
    }

    maj_stat = gss_acquire_cred(&min_stat, server_name, 0,
                                GSS_C_NULL_OID_SET, GSS_C_ACCEPT,
                                server_creds, NULL, NULL);
    if (maj_stat != GSS_S_COMPLETE) {
        *ret_min_stat = min_stat;
        return maj_stat;
    }

    (void) gss_release_name(&min_stat, &server_name);

    return maj_stat;
}

static
krb5_error_code
idm_krb5_get_in_tkt(
    char *user_name,
    char *domain_name,
    char *pwd,
    char **ret_princ_name)
{
    char *princ_name = NULL;  // convert pszUserName @ pszDomainName to UPN
    char *service_name = NULL; // Not used
    krb5_error_code krb_err = 0;
    krb5_get_init_creds_opt *krb_options = NULL;
    krb5_context krb_ctx = NULL;
    krb5_principal princ = NULL;
    krb5_ccache krb_cc = NULL;
    krb5_creds as_tkt = {0};
    krb5_deltat starttime = 0;
    int krb5_flags = 0; // KRB5_PRINCIPAL_PARSE_ENTERPRISE is valid value
    int princ_name_len = 0;

    princ_name_len = strlen(user_name) + strlen(domain_name) + 2;
    princ_name = calloc(princ_name_len, sizeof(char));
    if (!princ_name)
    {
        krb_err = ENOMEM;
        goto error;
    }

    snprintf(princ_name, princ_name_len, "%s@%s", user_name, domain_name);
    krb_err = krb5_init_context(&krb_ctx);
    if (krb_err)
    {
        goto error;
    }

    krb_err = krb5_cc_default(krb_ctx, &krb_cc);
    if (krb_err)
    {
        goto error;
    }

    krb_err = krb5_parse_name_flags(krb_ctx, princ_name, krb5_flags, &princ);
    if (krb_err)
    {
        goto error;
    }
    free(princ_name);
    princ_name = NULL;

    krb_err = krb5_unparse_name(krb_ctx, princ, &princ_name);
    if (krb_err)
    {
        goto error;
    }

    krb_err = krb5_get_init_creds_opt_alloc(krb_ctx, &krb_options);
    if (krb_err)
    {
        goto error;
    }

    krb_err = krb5_get_init_creds_password(
                  krb_ctx,
                  &as_tkt,
                  princ,
                  pwd,
                  NULL,   // prompter func; N/A
                  NULL,   // prompter data; N/A
                  starttime,
                  service_name,
                  krb_options);
    if (krb_err)
    {
        goto error;
    }

    krb_err = krb5_cc_initialize(krb_ctx, krb_cc, as_tkt.client);
    if (krb_err)
    {
        goto error;
    }
    krb_err = krb5_cc_store_cred(krb_ctx, krb_cc, &as_tkt);
    if (krb_err)
    {
        goto error;
    }

    *ret_princ_name = princ_name;

error:
    if (krb_options)
    {
        krb5_get_init_creds_opt_free(krb_ctx, krb_options);
    }
    if (krb_err)
    {
        IDM_SAFE_FREE_MEMORY(princ_name);
    }
    krb5_free_principal(krb_ctx, princ);
    krb5_free_cred_contents(krb_ctx, &as_tkt);
    krb5_cc_close(krb_ctx, krb_cc);
    krb5_free_context(krb_ctx);

    return krb_err;
}

static
DWORD
idm_get_upn_name_by_sid(
    PSTR pszUpnSid,
    PWSTR *ppwszUpnName)
{
    DWORD dwError = 0;
    DWORD i = 0;
    HANDLE hLsa = NULL;
    LSA_QUERY_LIST queryList = {0};
    PLSA_SECURITY_OBJECT *ppObjects = NULL;
    PWSTR pwszUpnName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszUpnName = NULL;

    dwError = LsaOpenServer(&hLsa);
    BAIL_ON_ERROR(dwError);

    queryList.ppszStrings = (PCSTR*) &pszUpnSid;

    dwError = LsaFindObjects(
        hLsa,
        NULL,
        0,
        LSA_OBJECT_TYPE_UNDEFINED,
        LSA_QUERY_TYPE_BY_SID,
        1,
        queryList,
        &ppObjects);
    BAIL_ON_ERROR(dwError);

    if (ppObjects[0] == NULL)
    {
        dwError = LW_ERROR_NO_SUCH_OBJECT;
        BAIL_ON_ERROR(dwError);
    }

    /*
     * Create UPN name from SID lookup. When both the DN for the user
     * and the samAccountName are both present, create the name from these
     * elements. Otherwise, just use the provided UPN.
     */

    if (ppObjects[0]->pszDN && ppObjects[0]->pszSamAccountName)
    {
        dwError = LwLdapConvertDNToDomain(
                      ppObjects[0]->pszDN,
                      &pszDomainName);
        BAIL_ON_ERROR(dwError);

        /* FQDN in UPN name must be uppercased */
        for (i=0; pszDomainName[i]; i++)
        {
            pszDomainName[i] = toupper((int) pszDomainName[i]);
        }

        dwError = IDMAllocateStringPrintf(
                      &pszUpnName,
                      "%s@%s",
                      ppObjects[0]->pszSamAccountName,
                      pszDomainName);
        BAIL_ON_ERROR(dwError);
    }
    else
    {
        dwError = IDMAllocateStringPrintf(
                      &pszUpnName,
                      "%s",
                      ppObjects[0]->userInfo.pszUPN);
        BAIL_ON_ERROR(dwError);
    }

    dwError = LwRtlWC16StringAllocateFromCString(
                  (PVOID) &pwszUpnName,
                  pszUpnName);
    BAIL_ON_ERROR(dwError);

    *ppwszUpnName = pwszUpnName;

error:
    if (dwError)
    {
        IDM_SAFE_FREE_MEMORY(pwszUpnName);
    }
    IDM_SAFE_FREE_MEMORY(pszDomainName);
    IDM_SAFE_FREE_MEMORY(pszUpnName);
    if (hLsa)
    {
        LsaCloseServer(hLsa);
    }
    if (ppObjects)
    {
        LsaFreeSecurityObjectList(1, ppObjects);
    }

    return dwError;
}

static
DWORD
idm_get_names_by_sids(
    PSTR *ppszSidStrings,
    DWORD sidCount,
    PWSTR **pppwszGroupNames,
    PWSTR **pppwszSidStrings,
    PDWORD dwSidNameCount)
{
    DWORD dwError = 0;
    DWORD i = 0;
    DWORD j = 0;

    // Used to call LsaGetNamesBySidList
    HANDLE hLsa = NULL;
    PLSA_SID_INFO pSIDInfoList = NULL;
    CHAR sepChar = '\0';

    // Used to fill in the returned GroupNames and SidStrings
    DWORD iResolvedCount = 0;
    PWSTR *ppwszGroupNames = NULL;
    PWSTR *ppwszSidStrings = NULL;

    // Used to construct the GroupName string
    PSTR upnString = NULL;

    // Used to batching the Lsa query
    DWORD dwMaxBatchSize = IDM_SID_LOOKUP_BATCH_SIZE;
    DWORD dwCurrentBatchSize = 0;
    DWORD dwLastBatchSize = 0;
    DWORD dwTotalBatchNumber = 0;
    DWORD dwCurrentBatchStartIndex = 0;

    dwError = LsaOpenServer(&hLsa);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                  sizeof(*ppwszGroupNames) * sidCount,
                  (PVOID) &ppwszGroupNames);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                  sizeof(*ppwszSidStrings) * sidCount,
                  (PVOID) &ppwszSidStrings);
    BAIL_ON_ERROR(dwError);

    // Calculate the total batch numbers and the last batch size
    dwLastBatchSize = sidCount % dwMaxBatchSize;
    dwTotalBatchNumber = sidCount / dwMaxBatchSize;
    if(dwLastBatchSize)
    {
        ++ dwTotalBatchNumber;
    }
    else
    {
        dwLastBatchSize = dwMaxBatchSize;
    }

    // Outer loop for all batches
    for (i=0, iResolvedCount =0; i < dwTotalBatchNumber; i++, dwError = 0)
    {
        dwCurrentBatchSize = dwMaxBatchSize;
        if(i == (dwTotalBatchNumber - 1)) // am I the last batch
        {
            dwCurrentBatchSize = dwLastBatchSize;
        }

        dwCurrentBatchStartIndex = i * dwMaxBatchSize;
        dwError = LsaGetNamesBySidList(
                      hLsa,
                      dwCurrentBatchSize,
                      &ppszSidStrings[dwCurrentBatchStartIndex],
                      &pSIDInfoList,
                      &sepChar);
        BAIL_ON_ERROR(dwError);

        // Inner loop for the current batch
        for(j=0; j < dwCurrentBatchSize; j++)
        {
            if(pSIDInfoList[j].accountType == AccountType_NotFound)
            {
                continue;
            }

            dwError = LwRtlWC16StringAllocateFromCString(
                          &ppwszSidStrings[iResolvedCount],
                          ppszSidStrings[dwCurrentBatchStartIndex + j]);
            BAIL_ON_ERROR(dwError);

            dwError = IDMAllocateStringPrintf(
                          &upnString,
                          "%s%c%s",
                          pSIDInfoList[j].pszDomainName,
                          sepChar,
                          pSIDInfoList[j].pszSamAccountName);
            BAIL_ON_ERROR(dwError);

            dwError = LwRtlWC16StringAllocateFromCString(
                          &ppwszGroupNames[iResolvedCount],
                          upnString);
            BAIL_ON_ERROR(dwError);
            IDM_SAFE_FREE_MEMORY(upnString);
            iResolvedCount ++;
        }

        if(pSIDInfoList)
        {
            LsaFreeSIDInfoList(pSIDInfoList, dwCurrentBatchSize);
            pSIDInfoList = NULL;
        }
    }

    *pppwszGroupNames = ppwszGroupNames;
    *pppwszSidStrings = ppwszSidStrings;
    *dwSidNameCount = iResolvedCount;

error:
    if (dwError)
    {
         for (i=0; i<iResolvedCount; i++)
         {
             IDM_SAFE_FREE_MEMORY(ppwszGroupNames[i]);
             IDM_SAFE_FREE_MEMORY(ppwszSidStrings[i]);
         }
         IDM_SAFE_FREE_MEMORY(ppwszGroupNames);
         IDM_SAFE_FREE_MEMORY(ppwszSidStrings);
    }

    IDM_SAFE_FREE_MEMORY(upnString);

    if (pSIDInfoList)
    {
        LsaFreeSIDInfo(pSIDInfoList);
    }

    if(hLsa)
    {
        LsaCloseServer(hLsa);
    }

    return dwError;
}

static
DWORD
idm_logon_gssapi(PIDM_AUTH_CONTEXT pAuthContext)
{
    OM_uint32 maj_stat = 0;
    OM_uint32 min_stat = 0;
    OM_uint32 init_sec_min_stat = 0;
    gss_name_t target_name = {0};
    gss_ctx_id_t init_context = {0};
    gss_buffer_desc send_tok = {0};
    gss_buffer_desc recv_tok = {0};
    gss_buffer_desc *token_ptr = NULL;
    gss_OID oid = GSS_C_NULL_OID;
    OM_uint32 gss_flags = 0;;
    OM_uint32 ret_flags = 0;
    char *service_name = IDM_GSS_SERVICE_NAME;

    token_ptr = GSS_C_NO_BUFFER;
    init_context = GSS_C_NO_CONTEXT;

    /*
     * Import the name into target_name.  Use send_tok to save
     * local variable space.
     */
    send_tok.value = service_name;
    send_tok.length = strlen(service_name);
    maj_stat = gss_import_name(&min_stat, &send_tok,
                               (gss_OID) gss_nt_service_name,
                               &target_name);
    if (maj_stat != GSS_S_COMPLETE) {
        goto error;
    }

    maj_stat = gss_init_sec_context(&init_sec_min_stat,
                   GSS_C_NO_CREDENTIAL,
                   &init_context,
                   target_name,
                   oid,
                   gss_flags,
                   0,
                   NULL, /* no channel bindings */
                   token_ptr,
                   NULL, /* ignore mech type */
                   &recv_tok,
                   &ret_flags,
                   NULL);        /* ignore time_rec */
    if (maj_stat)
    {
        goto error;
    }

    maj_stat = gss_accept_sec_context(
        &init_sec_min_stat,
        (gss_ctx_id_t *) &pAuthContext->hCtxtHandle,
        (gss_cred_id_t) pAuthContext->hCredsHandle,
        &recv_tok,
        GSS_C_NO_CHANNEL_BINDINGS,
        NULL,
        NULL,
        &send_tok,
        &ret_flags,
        NULL,       /* ignore time_rec */
        NULL);    /* ignore del_cred_handle */
    if (maj_stat)
    {
        goto error;
    }

error:
    if (recv_tok.length && recv_tok.value)
    {
        gss_release_buffer(&min_stat, &recv_tok);
    }
    gss_release_name(&min_stat, &target_name);
    if (init_context != GSS_C_NO_CONTEXT)
    {
        gss_delete_sec_context(&min_stat,
                               &init_context,
                               GSS_C_NO_BUFFER);
    }
    return maj_stat;
}

DWORD
IDMAuthenticateUser(
    PWSTR pszUserName,
    PWSTR pszDomainName,
    PWSTR pszPassword,
    PIDM_USER_INFO *ppIdmUserInformation
    )
{
    DWORD dwError = 0;
    krb5_error_code krb_err = 0;
    PIDM_USER_INFO pIdmUserInformation = NULL;
    NTSTATUS nterr = 0;
    char *domain_name = NULL;
    char *domain_name_uppercase = NULL;
    char *user_name = NULL;
    char *princ_name = NULL;  // convert pszUserName @ pszDomainName to UPN
    char *pwd = NULL;
    PIDM_AUTH_CONTEXT pAuthContext = NULL;
    int i = 0;
    BOOL bMutexObtained = FALSE;

    nterr = LwRtlCStringAllocateFromWC16String(
                &domain_name,
                pszDomainName);
    if (nterr)
    {
        krb_err = LwNtStatusToErrno(nterr);
        goto error;
    }

    nterr = LwRtlCStringAllocateFromWC16String(
                &user_name,
                pszUserName);
    if (nterr)
    {
        krb_err = LwNtStatusToErrno(nterr);
        goto error;
    }

    nterr = LwRtlCStringAllocateFromWC16String(
                &pwd,
                pszPassword);
    if (nterr)
    {
        krb_err = LwNtStatusToErrno(nterr);
        goto error;
    }

    /*
     * Uppercase domain name for free
    */
    dwError = IDMAllocateStringA(
                    domain_name,
                    &domain_name_uppercase);
    BAIL_ON_ERROR(dwError);

    for (i=0; domain_name_uppercase[i]; i++)
    {
        domain_name_uppercase[i] = toupper((int) domain_name_uppercase[i]);
    }

    IDM_MUTEX_LOCK(&pgIdmAuthMutex->mutex, bMutexObtained, dwError);

    BAIL_ON_ERROR(dwError);

    nterr = idm_krb5_get_in_tkt(
                user_name,
                domain_name_uppercase,
                pwd,
                &princ_name);
    /*
     * Try case exact domain name after KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN failure with upper case domain name
     */
    if (nterr == KRB5KDC_ERR_C_PRINCIPAL_UNKNOWN)
    {
        nterr = idm_krb5_get_in_tkt(
                user_name,
                domain_name,
                pwd,
                &princ_name);
    }

    if (nterr)
    {
        krb_err = nterr;
        goto error;
    }

    dwError = IDMCreateAuthContext(NULL, &pAuthContext);
    BAIL_ON_ERROR(dwError);

    dwError = idm_logon_gssapi(pAuthContext);
    BAIL_ON_ERROR(dwError);

    IDM_MUTEX_UNLOCK(&pgIdmAuthMutex->mutex, bMutexObtained, dwError);
    BAIL_ON_ERROR(dwError);

    dwError = IDMGetUserInformationFromAuthContext(
                  pAuthContext,
                  &pIdmUserInformation);
    BAIL_ON_ERROR(dwError);
    *ppIdmUserInformation = pIdmUserInformation;

error:
    if (dwError)
    {
        krb_err = dwError;
    }
    IDM_SAFE_FREE_MEMORY(domain_name);
    IDM_SAFE_FREE_MEMORY(domain_name_uppercase);
    IDM_SAFE_FREE_MEMORY(user_name);
    IDM_SAFE_FREE_MEMORY(princ_name);
    IDM_SAFE_FREE_MEMORY(pwd);
    IDMFreeAuthContext(pAuthContext);
    IDM_MUTEX_UNLOCK(&pgIdmAuthMutex->mutex, bMutexObtained, dwError);

    return krb_err;
}


DWORD
IDMCreateAuthContext(
    PWSTR pszPackageName,  // Not used
    PIDM_AUTH_CONTEXT *ppAuthContext
    )
{
    DWORD dwError = 0;
    PIDM_AUTH_CONTEXT pAuthContext = NULL;
    int serr = 0;
    char *service_name = IDM_GSS_SERVICE_NAME;
    OM_uint32 min_stat = 0;

    dwError = IDMAllocateMemory(sizeof(*pAuthContext), (PVOID) &pAuthContext);
    BAIL_ON_ERROR(dwError);

    serr = server_acquire_creds(service_name, (gss_cred_id_t *) &pAuthContext->hCredsHandle, &min_stat);
    if (serr)
    {
        dwError = ERROR_INVALID_SERVICENAME;
        BAIL_ON_ERROR(dwError);
    }

    *ppAuthContext = pAuthContext;

error:
    if (dwError)
    {
        IDM_SAFE_FREE_MEMORY(pAuthContext);
    }
    return dwError;
}

DWORD
IDMGetUserInformationFromAuthContext(
        PIDM_AUTH_CONTEXT pAuthContext,
        PIDM_USER_INFO *ppIdmUserInformation
        )
{
    DWORD dwError = 0;
    DWORD sidCount = 0;
    DWORD iSid = 0;
    DWORD reqTokenLen = 0;
    DWORD reqTokenUserLen = 0;
    PBYTE reqTokenBuf = NULL;
    PBYTE reqTokenUserBuf = NULL;
    NTSTATUS status = 0;
    PIDM_USER_INFO pIdmUserInformation = NULL;
    PACCESS_TOKEN pAccessToken = NULL;
    PLW_MAP_SECURITY_CONTEXT MapSecurityContext = NULL;
    PSTR *ppszGroupNames = NULL;
    PTOKEN_GROUPS pGroups = NULL;  // Alias, do not free
    PTOKEN_USER pUser = NULL;  // Alias, do not free
    PWSTR *ppwszSidNames = NULL; // User friendly list of SIDS
    PWSTR *ppwszSidStrings = NULL;
    PWSTR pwszUpnName = NULL;
    PWSTR pwszLookupSid = NULL;
    PSTR pszUserSid = NULL;
    PWSTR pwszRetUserSid = NULL;
    PSTR pszGroupSid = NULL;

    if (!pAuthContext || !ppIdmUserInformation)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    dwError = IDMInitializeSidCache();
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                  sizeof(*pIdmUserInformation),
                  (PVOID) &pIdmUserInformation);
    BAIL_ON_ERROR(dwError);

    dwError = LwMapSecurityCreateContext(&MapSecurityContext);
    BAIL_ON_ERROR(dwError);

    dwError = LwMapSecurityCreateAccessTokenFromGssContext(
                 MapSecurityContext,
                 &pAccessToken,
                 (LW_MAP_SECURITY_GSS_CONTEXT) pAuthContext->hCtxtHandle);
    BAIL_ON_ERROR(dwError);

    /* Query memory needed for group SIDS */
    status = RtlQueryAccessTokenInformation(
                 pAccessToken,
                 TokenGroups,
                 NULL,
                 0,
                 &reqTokenLen);
    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        dwError = LwNtStatusToWin32Error(status);
        BAIL_ON_ERROR(dwError);
    }

    /* Allocate required memory for group SIDS */
    dwError = IDMAllocateMemory(
                  reqTokenLen, // Sure hope this is size in bytes
                  (PVOID) &reqTokenBuf);
    BAIL_ON_ERROR(dwError);

    /* Get the actual group tokens */
    pGroups = (PTOKEN_GROUPS) reqTokenBuf;
    status = RtlQueryAccessTokenInformation(
                 pAccessToken,
                 TokenGroups,
                 reqTokenBuf,
                 reqTokenLen,
                 &reqTokenLen);
    dwError = LwNtStatusToWin32Error(status);
    BAIL_ON_ERROR(dwError);

    dwError = IDMAllocateMemory(
                  pGroups->GroupCount * sizeof(*ppszGroupNames),
                  (PVOID) &ppszGroupNames);
    BAIL_ON_ERROR(dwError);

    /* Query memory needed for User Sid */
    status = RtlQueryAccessTokenInformation(
                 pAccessToken,
                 TokenUser,
                 NULL,
                 0,
                 &reqTokenUserLen);
    if (status != STATUS_BUFFER_TOO_SMALL)
    {
        dwError = LwNtStatusToWin32Error(status);
        BAIL_ON_ERROR(dwError);
    }

    /* Allocate required memory for User Sid */
    dwError = IDMAllocateMemory(
                  reqTokenUserLen, // Sure hope this is size in bytes
                  (PVOID) &reqTokenUserBuf);
    BAIL_ON_ERROR(dwError);

    /* Get the actual UPN Sid */
    pUser = (PTOKEN_USER) reqTokenUserBuf;
    status = RtlQueryAccessTokenInformation(
                 pAccessToken,
                 TokenUser,
                 reqTokenUserBuf,
                 reqTokenUserLen,
                 &reqTokenUserLen);
    dwError = LwNtStatusToWin32Error(status);
    BAIL_ON_ERROR(dwError);

    dwError = RtlAllocateCStringFromSid(
                  &pszUserSid,
                  pUser->User.Sid);
    BAIL_ON_ERROR(dwError);

    for (sidCount=0, iSid=0; sidCount<pGroups->GroupCount; sidCount++)
    {
/* Useful for debugging */
#if 0
{
PSTR pszStringSid = NULL;
RtlAllocateCStringFromSid(&pszStringSid, pGroups->Groups[sidCount].Sid);
printf("SID Value = %s\n", pszStringSid);
IDM_SAFE_FREE_MEMORY(pszStringSid);
}
#endif
        IDM_SAFE_FREE_MEMORY(pszGroupSid);
        status = RtlAllocateCStringFromSid(
                     &pszGroupSid,
                     pGroups->Groups[sidCount].Sid);
        dwError = LwNtStatusToWin32Error(status);
        BAIL_ON_ERROR(dwError);

        /* Skip group SID entries that match the user SID */
        if (strcmp(pszUserSid, pszGroupSid) == 0)
        {
            continue;
        }

        dwError = FindSidCacheEntry(
                      IDMGetBuiltinSidCache(),
                      pGroups->Groups[sidCount].Sid,
                      &pwszLookupSid);
        if (dwError && !pwszLookupSid)
        {
            /* Entry is not a well-known SID, add to list to lookup */
            ppszGroupNames[iSid] = pszGroupSid;
            pszGroupSid = NULL;
            iSid++;
        }
        IDM_SAFE_FREE_MEMORY(pwszLookupSid);
    }


    dwError = idm_get_upn_name_by_sid(
                  pszUserSid,
                  &pwszUpnName);
    BAIL_ON_ERROR(dwError);

    sidCount = 0;
    dwError = idm_get_names_by_sids(
                  ppszGroupNames,
                  iSid,
                  &ppwszSidNames,
                  &ppwszSidStrings,
                  &sidCount);
    BAIL_ON_ERROR(dwError);

    dwError = RtlAllocateWC16StringFromSid(
                  &pwszRetUserSid,
                  pUser->User.Sid);
    BAIL_ON_ERROR(dwError);

    pIdmUserInformation->ppszGroupNames = ppwszSidNames;
    pIdmUserInformation->ppszSids = ppwszSidStrings;
    pIdmUserInformation->dwNumGroups = sidCount;
    pIdmUserInformation->pszUserName = pwszUpnName;
    pIdmUserInformation->pszUserSid = pwszRetUserSid;
    *ppIdmUserInformation = pIdmUserInformation;
    ppwszSidNames = NULL;
    ppwszSidStrings = NULL;
    pwszUpnName = NULL;

error:
    if (dwError)
    {
        IDMFreeUserInfo(pIdmUserInformation);
    }
    if (pGroups && ppszGroupNames)
    {
        for (sidCount = 0; sidCount < pGroups->GroupCount; sidCount++)
        {
            IDM_SAFE_FREE_MEMORY(ppszGroupNames[sidCount]);
        }
    }
    IDM_SAFE_FREE_MEMORY(pwszLookupSid);
    IDM_SAFE_FREE_MEMORY(ppszGroupNames);
    IDM_SAFE_FREE_MEMORY(reqTokenBuf);
    IDM_SAFE_FREE_MEMORY(reqTokenUserBuf);
    IDM_SAFE_FREE_MEMORY(ppwszSidNames);
    IDM_SAFE_FREE_MEMORY(ppwszSidStrings);
    IDM_SAFE_FREE_MEMORY(pwszUpnName);
    IDM_SAFE_FREE_MEMORY(pszUserSid);
    IDM_SAFE_FREE_MEMORY(pszGroupSid);
    RtlReleaseAccessToken(&pAccessToken);
    LwMapSecurityFreeContext(&MapSecurityContext);

    return dwError;
}

VOID
IDMFreeUserInfo(
    PIDM_USER_INFO pIdmUserInformation
    )
{
    DWORD i = 0;
    if (pIdmUserInformation)
    {
        for (i = 0; i < pIdmUserInformation->dwNumGroups; i++)
        {
            IDM_SAFE_FREE_MEMORY(pIdmUserInformation->ppszGroupNames[i]);
            IDM_SAFE_FREE_MEMORY(pIdmUserInformation->ppszSids[i]);
        }
        IDM_SAFE_FREE_MEMORY(pIdmUserInformation->ppszGroupNames);
        IDM_SAFE_FREE_MEMORY(pIdmUserInformation->ppszSids);
        IDM_SAFE_FREE_MEMORY(pIdmUserInformation->pszUserName);
        IDM_SAFE_FREE_MEMORY(pIdmUserInformation->pszUserSid);
        IDM_SAFE_FREE_MEMORY(pIdmUserInformation);
    }
}


VOID
IDMFreeAuthContext(
    PIDM_AUTH_CONTEXT pAuthContext
    )
{
    OM_uint32 min_stat = 0;

    if (pAuthContext)
    {
        if ((gss_ctx_id_t) pAuthContext->hCtxtHandle != GSS_C_NO_CONTEXT)
        {
            gss_delete_sec_context(&min_stat,
                                   (gss_ctx_id_t *) &pAuthContext->hCtxtHandle,
                                   GSS_C_NO_BUFFER);
        }
        if ((gss_cred_id_t) pAuthContext->hCredsHandle != GSS_C_NO_CREDENTIAL)
        {
            gss_release_cred(&min_stat,
                            (gss_cred_id_t *) &pAuthContext->hCredsHandle);
        }
        IDM_SAFE_FREE_MEMORY(pAuthContext);
    }
}


DWORD
IDMAuthenticate2(
    PIDM_AUTH_CONTEXT pAuthContext,
    PBYTE pInputBuffer,
    DWORD dwInputBufferSize,
    PBYTE *ppOutputBuffer,
    DWORD *pdwOutputBufferSize,
    BOOL *pfDone
    )
{
    OM_uint32 maj_stat = 0;
    OM_uint32 min_stat = 0;
    OM_uint32 acc_sec_min_stat = 0;
    gss_buffer_desc  recv_tok = {0};
    gss_buffer_desc send_tok = {0};
    DWORD dwError = 0;
    OM_uint32 ret_flags = 0;

    if (!pAuthContext ||
        !pInputBuffer ||
        !ppOutputBuffer ||
        !pdwOutputBufferSize ||
        !pfDone)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    recv_tok.value = pInputBuffer;
    recv_tok.length = dwInputBufferSize;

    maj_stat = gss_accept_sec_context(
        &acc_sec_min_stat,
        (gss_ctx_id_t *) &pAuthContext->hCtxtHandle,
        (gss_cred_id_t) pAuthContext->hCredsHandle,
        &recv_tok,
        GSS_C_NO_CHANNEL_BINDINGS,
        NULL,
        NULL,
        &send_tok,
        &ret_flags,
        NULL,       /* ignore time_rec */
        NULL);    /* ignore del_cred_handle */
    if (maj_stat == GSS_S_CONTINUE_NEEDED)
    {
        *pfDone = FALSE;
        *ppOutputBuffer = send_tok.value;
        *pdwOutputBufferSize = send_tok.length;
    }
    else
    {
        if (maj_stat == GSS_S_COMPLETE)
        {
            *pfDone = TRUE;
        }
        else
        {
            dwError = IDM_ERROR_USER_INVALID_CREDENTIAL;
            BAIL_ON_ERROR(dwError);
        }
    }

error:
    if (dwError)
    {
        if (send_tok.length && send_tok.value)
        {
            gss_release_buffer(&min_stat, &send_tok);
        }
        if ((gss_ctx_id_t) pAuthContext->hCtxtHandle != GSS_C_NO_CONTEXT)
        {
            gss_delete_sec_context(&min_stat,
                                   (gss_ctx_id_t *) &pAuthContext->hCtxtHandle,
                                   GSS_C_NO_BUFFER);
        }
    }
    return dwError;
}

DWORD
IDMGetComputerName(
    PWSTR* ppszName)
{
    DWORD dwError = 0;
    PWSTR pwszName = NULL;
    CHAR pHostName[IDM_MAX_HOSTNAME_LEN];

    if (!ppszName)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_ERROR(dwError);
    }

    if (gethostname(pHostName, sizeof(pHostName)-1) < 0)
    {
        dwError = LwErrnoToWin32Error(errno);
        BAIL_ON_ERROR(dwError);
    }

    dwError = LwRtlWC16StringAllocateFromCString(
                  &pwszName,
                  pHostName);
    BAIL_ON_ERROR(dwError);

    *ppszName = pwszName;

error:
    return dwError;
}
