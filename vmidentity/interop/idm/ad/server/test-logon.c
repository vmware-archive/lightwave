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
 *        test-logon.c
 *
 * Abstract:
 *                      
 *        Identity Manager - Active Directory Integration
 *
 *        Logon API test
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"
#include "idmcommon.h"
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include <locale.h>
#include <stdio.h>
#include "../structs.h"
#include "../prototypes.h"
#include "../defines.h"

#include <string.h>
#include <stdlib.h>

#ifndef _WIN32
/* getpass() prototype */
#include <unistd.h>
#endif

static char *argv0;
static int argv0len;

#ifdef _WIN32

#define snprintf _snprintf

DWORD
LwRtlWC16StringAllocateFromCString(PWSTR * outwstr, PSTR str)
{
    int len = 0;
    PWSTR tmpwstr = NULL;

    len = MultiByteToWideChar(
          CP_ACP,
          0,
          str,
          -1,
          NULL,
          0);
    if (len == 0)
    {
        return GetLastError();
    }
    tmpwstr = calloc(len+1, sizeof(WCHAR));
    if (!tmpwstr)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    len = MultiByteToWideChar(
          CP_ACP,
          0,
          str,
          -1,
          tmpwstr,
          len);
    if (len)
    {
        *outwstr = tmpwstr;
        return 0;
    }
    return GetLastError();
}

DWORD
LwRtlCStringAllocateFromWC16String(
        PSTR *outcstr,
        PWSTR inwstr)
{
    int len = 0;
    PSTR tmpcstr = NULL;

    len = WideCharToMultiByte(
          CP_ACP,
          0,
          inwstr,
          -1,
          NULL,
          0,
          NULL,
          NULL);

    if (len == 0)
    {
        return GetLastError();
    }
    tmpcstr = calloc(len+1, sizeof(CHAR));
    if (!tmpcstr)
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    len = WideCharToMultiByte(
          CP_ACP,
          0,
          inwstr,
          -1,
          tmpcstr,
          len,
          NULL,
          NULL);
    if (len)
    {
        *outcstr = tmpcstr;
        return 0;
    }
    return GetLastError();
}

char *getpass(char *prompt)
{
    static char passbuf[128];
    int i = 0;

    printf("%s (insecure) ", prompt);
    fflush(stdout);
    fgets(passbuf, (int) sizeof(passbuf), stdin);
    for (i=0; passbuf[i]; i++)
    {
        if (passbuf[i] == '\r' || passbuf[i] == '\n')
        {
            passbuf[i] = '\0';
        }
    }
    return passbuf;
}

#endif


/*
 * 1.3.6.1.4.1.6876.11711.2.1.1.1
 *
 * {iso(1) identified-organization(3) dod(6) internet(1) private(4)
 *   enterprise(1) 6876 vmwSecurity(11711) vmwAuthentication(2) vmwGSSAPI(1)
 *   vmwSRP(1) vmwSrpCredOptPwd(1)}
 * Official registered GSSAPI_SRP password cred option OID
 */
#ifndef GSSAPI_SRP_CRED_OPT_PW
#define GSSAPI_SRP_CRED_OPT_PW  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01\x01"
#define GSSAPI_SRP_USERNAME  \
    "\x2b\x06\x01\x04\x01\xb5\x5c\xdb\x3f\x02\x01\x01\x02"
#endif

#ifndef GSSAPI_SRP_CRED_OPT_PW_LEN
#define GSSAPI_SRP_CRED_OPT_PW_LEN  13
#endif

/*
 * 1.2.840.113554.1.2.10
 *
 * {iso(1) member-body(2) US(840) mit(113554) infosys(1) gssapi(2) srp(10)}
 * "Made up" SRP OID,
 * "Made up" SRP OID, which is actually in MIT GSSAPI OID namespace,
 *  based on existing GSSAPI mech OIDs.
 * This is being depricated in future releases.
 */
#ifndef GSS_SRP_MECH_OID
#define GSS_SRP_MECH_OID_LENGTH 9
#define GSS_SRP_MECH_OID "\x2a\x86\x48\x86\xf7\x12\x01\x02\x0a"
#endif

#ifndef SPNEGO_OID
#define SPNEGO_OID_LENGTH 6
#define SPNEGO_OID "\x2b\x06\x01\x05\x05\x02"
#endif

DWORD
IDMAuthenticateUserSrp(
    PWSTR pwszUserName,
    PWSTR pwszDomainName,
    PWSTR pwszPassword,
    PIDM_USER_INFO *ppIdmUserInformation
    )
{
    DWORD dwError = 0;
    OM_uint32 min = 0;
    OM_uint32 maj = 0;
    gss_buffer_desc name_buf = {0};
    gss_name_t gss_name_buf = NULL;
    gss_name_t cred_name_buf = NULL;
    gss_buffer_desc disp_name_buf = {0};
    gss_buffer_desc gss_pwd = {0};
    gss_OID_desc gss_srp_password_oid =
        {GSSAPI_SRP_CRED_OPT_PW_LEN, (void *) GSSAPI_SRP_CRED_OPT_PW};
    size_t len = 0;
    gss_OID_desc oid_spnego_desc = {SPNEGO_OID_LENGTH, SPNEGO_OID};
    gss_OID oid_spnego = &oid_spnego_desc;
    gss_OID disp_name_OID = NULL;
    gss_cred_id_t cred_handle_cli = NULL;
    gss_cred_id_t cred_handle_svr = NULL;
    gss_OID_set_desc desired_mech = {0};
    OM_uint32 req_flags = 0;
    OM_uint32 time_req = 0; /* default is 2 hrs */
    gss_buffer_desc recv_tok = {0};
    gss_buffer_desc send_tok = {0};
    OM_uint32 ret_flags = 0;
    gss_OID actual_mech = NULL;
    NTSTATUS nterr = 0;
    PSTR pszUserName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUpnName = NULL;
    gss_ctx_id_t gss_ctx_cli = NULL;
    gss_ctx_id_t gss_ctx_svr = NULL;
    gss_OID_desc mech_srp_desc = {GSS_SRP_MECH_OID_LENGTH, (void *) GSS_SRP_MECH_OID};
    gss_OID mech_srp = &mech_srp_desc;

    nterr = LwRtlCStringAllocateFromWC16String(
                &pszUserName,
                pwszUserName);
    BAIL_ON_ERROR(nterr);

    nterr = LwRtlCStringAllocateFromWC16String(
                &pszPassword,
                pwszPassword);
    BAIL_ON_ERROR(nterr);

    if (pwszDomainName)
    {
        nterr = LwRtlCStringAllocateFromWC16String(
                    &pszDomainName,
                    pwszDomainName);
        BAIL_ON_ERROR(nterr);
    }

    if (!pszDomainName || !*pszDomainName)
    {
        len = strlen(pszUserName) + 1;
        pszUpnName = calloc(len, sizeof(char));
        snprintf(pszUpnName, len, "%s", pszUserName);
    }
    else
    {
        /* user@domin */
        len = strlen(pszUserName) +  1 + strlen(pszDomainName) + 1;
        pszUpnName = calloc(len, sizeof(char));
        snprintf(pszUpnName, len, "%s@%s", pszUserName, pszDomainName);
    }

    name_buf.value = pszUpnName;
    name_buf.length = strlen(pszUpnName);
    maj = gss_import_name(&min, &name_buf, GSS_C_NT_USER_NAME, &gss_name_buf);
    if (maj)
    {
        dwError = min;
        BAIL_ON_ERROR(dwError);
    }

    /*
     * Hard code desired mech OID to SRP
     */
    desired_mech.elements = (gss_OID) mech_srp;
    desired_mech.count = 1;
    maj = gss_acquire_cred(
              &min,
              gss_name_buf,
              0,
              &desired_mech,
              GSS_C_INITIATE,
              &cred_handle_cli,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    maj = gss_inquire_cred(
              &min,
              cred_handle_cli,
              &cred_name_buf,
              NULL,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }
    maj = gss_display_name(&min, cred_name_buf, &disp_name_buf, &disp_name_OID);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

printf("UPN: %.*s\n", (int) disp_name_buf.length, (char *) disp_name_buf.value);
    gss_release_buffer(&min, &disp_name_buf);
    gss_pwd.value = pszPassword;
    gss_pwd.length = strlen(pszPassword);
    maj = gss_set_cred_option(
              &min,
              &cred_handle_cli,
              &gss_srp_password_oid,
              &gss_pwd);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    cred_handle_svr = NULL;
    maj = gss_acquire_cred(
              &min,
              GSS_C_NO_NAME,
              0,
              &desired_mech,
              GSS_C_ACCEPT,
              &cred_handle_svr,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    do
    {
        maj = gss_init_sec_context(
                    &min,
                    cred_handle_cli,
                    &gss_ctx_cli,
                    cred_name_buf,
                    oid_spnego,
                    req_flags,
                    time_req,
                    GSS_C_NO_CHANNEL_BINDINGS,
                    &send_tok,
                    &actual_mech,
                    &recv_tok,
                    &ret_flags,
                    NULL /* time_rec */);
        if (recv_tok.length > 0)
        {
            maj = GSS_S_CONTINUE_NEEDED;
        }
        if (maj == GSS_S_COMPLETE)
        {
            break;
        }
        else if (maj != GSS_S_CONTINUE_NEEDED)
        {
            dwError = min ? min : maj;
            BAIL_ON_ERROR(dwError);
        }
        if (send_tok.value)
        {
            gss_release_buffer(&min, &send_tok);
        }

        maj = gss_accept_sec_context(
            &min,
            &gss_ctx_svr,
            cred_handle_svr,
            &recv_tok,
            GSS_C_NO_CHANNEL_BINDINGS,
            NULL,
            NULL,
            &send_tok,
            &ret_flags,
            NULL,       /* ignore time_rec */
            NULL);    /* ignore del_cred_handle */
        if (recv_tok.value)
        {
            gss_release_buffer(&min, &recv_tok);
        }
        if (send_tok.length > 0)
        {
            maj = GSS_S_CONTINUE_NEEDED;
        }
    } while (maj == GSS_S_CONTINUE_NEEDED);

    if (maj != GSS_S_COMPLETE)
    {
        dwError = min ? min : maj;
    }

error:
    if (recv_tok.value)
    {
        gss_release_buffer(&min, &recv_tok);
    }
    if (send_tok.value)
    {
        gss_release_buffer(&min, &send_tok);
    }
    if (gss_ctx_cli != GSS_C_NO_CONTEXT)
    {
        gss_delete_sec_context(&min, &gss_ctx_cli, GSS_C_NO_BUFFER);
    }
    if (gss_ctx_svr != GSS_C_NO_CONTEXT)
    {
        gss_delete_sec_context(&min, &gss_ctx_svr , GSS_C_NO_BUFFER);
    }
    if (gss_name_buf)
    {
        gss_release_name(&min, &gss_name_buf);
    }
    if (cred_name_buf)
    {
        gss_release_name(&min, &cred_name_buf);
    }
    if (disp_name_buf.value)
    {
        gss_release_buffer(&min, &disp_name_buf);
    }
    if (cred_handle_cli)
    {
        gss_release_cred(&min, &cred_handle_cli);
    }
    if (cred_handle_svr)
    {
        gss_release_cred(&min, &cred_handle_svr);
    }

    IDM_SAFE_FREE_MEMORY(pszUserName);
    IDM_SAFE_FREE_MEMORY(pszPassword);
    IDM_SAFE_FREE_MEMORY(pszDomainName);
    IDM_SAFE_FREE_MEMORY(pszUpnName);
    return dwError;

}

#ifndef NTLM_OID
#define NTLM_OID_LENGTH 10
#define NTLM_OID "\x2b\x06\x01\x04\x01\x82\x37\x02\x02\x0a"
#endif

#ifndef GSS_NTLM_PASSWORD_OID
#define GSS_NTLM_PASSWORD_OID "\x2b\x06\x01\x04\x01\x81\xd6\x29\x03\x01"
#define GSS_NTLM_PASSWORD_LEN 10
#endif

DWORD
IDMAuthenticateUserNtlm(
    PWSTR pwszUserName,
    PWSTR pwszDomainName,
    PWSTR pwszPassword,
    PIDM_USER_INFO *ppIdmUserInformation
    )
{
    DWORD dwError = 0;
    OM_uint32 min = 0;
    OM_uint32 maj = 0;
    gss_buffer_desc name_buf = {0};
    gss_name_t gss_name_buf = NULL;
    gss_name_t cred_name_buf = NULL;
    gss_buffer_desc disp_name_buf = {0};
    gss_buffer_desc gss_pwd = {0};
    gss_OID_desc gss_ntlm_password_oid =
        {GSS_NTLM_PASSWORD_LEN, (void *) GSS_NTLM_PASSWORD_OID};
#if 1
    gss_OID_desc oid_spnego_desc = {SPNEGO_OID_LENGTH, SPNEGO_OID};
    gss_OID oid_spnego = &oid_spnego_desc;
#endif
    gss_OID_desc mech_ntlm_desc = {NTLM_OID_LENGTH, (void *) NTLM_OID};
    gss_OID mech_ntlm = &mech_ntlm_desc;
    size_t len = 0;

    gss_OID disp_name_OID = NULL;
    gss_cred_id_t cred_handle_cli = NULL;
    gss_cred_id_t cred_handle_svr = NULL;
    gss_OID_set_desc desired_mech = {0};
    OM_uint32 req_flags = 0;
    OM_uint32 time_req = 0; /* default is 2 hrs */
    gss_buffer_desc recv_tok = {0};
    gss_buffer_desc send_tok = {0};
    OM_uint32 ret_flags = 0;
    gss_OID actual_mech = NULL;
    NTSTATUS nterr = 0;
    PSTR pszUserName = NULL;
    PSTR pszDomainName = NULL;
    PSTR pszPassword = NULL;
    PSTR pszUpnName = NULL;
    gss_ctx_id_t gss_ctx_cli = NULL;
    gss_ctx_id_t gss_ctx_svr = NULL;

    nterr = LwRtlCStringAllocateFromWC16String(
                &pszUserName,
                pwszUserName);
    BAIL_ON_ERROR(nterr);

    nterr = LwRtlCStringAllocateFromWC16String(
                &pszPassword,
                pwszPassword);
    BAIL_ON_ERROR(nterr);

    if (pwszDomainName)
    {
        nterr = LwRtlCStringAllocateFromWC16String(
                    &pszDomainName,
                    pwszDomainName);
        BAIL_ON_ERROR(nterr);
    }

    if (!pszDomainName || !*pszDomainName)
    {
        len = strlen(pszUserName) + 1;
        pszUpnName = calloc(len, sizeof(char));
        snprintf(pszUpnName, len, "%s", pszUserName);
    }
    else
    {
        /* user@domin */
        len = strlen(pszUserName) +  1 + strlen(pszDomainName) + 1;
        pszUpnName = calloc(len, sizeof(char));
        snprintf(pszUpnName, len, "%s@%s", pszUserName, pszDomainName);
    }

    name_buf.value = pszUpnName;
    name_buf.length = strlen(pszUpnName);
    maj = gss_import_name(&min, &name_buf, GSS_C_NT_USER_NAME, &gss_name_buf);
    if (maj)
    {
        dwError = min;
        BAIL_ON_ERROR(dwError);
    }

    /*
     * Hard code desired mech OID to NTLM
     */
    desired_mech.elements = (gss_OID) mech_ntlm;
    desired_mech.count = 1;
    maj = gss_acquire_cred(
              &min,
              gss_name_buf,
              0,
              &desired_mech,
              GSS_C_INITIATE,
              &cred_handle_cli,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    maj = gss_inquire_cred(
              &min,
              cred_handle_cli,
              &cred_name_buf,
              NULL,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }
    maj = gss_display_name(&min, cred_name_buf, &disp_name_buf, &disp_name_OID);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

printf("UPN: %.*s\n", (int) disp_name_buf.length, (char *) disp_name_buf.value);
    gss_release_buffer(&min, &disp_name_buf);
    gss_pwd.value = pszPassword;
    gss_pwd.length = strlen(pszPassword);
    maj = gss_set_cred_option(
              &min,
              &cred_handle_cli,
              &gss_ntlm_password_oid,
              &gss_pwd);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    cred_handle_svr = NULL;
    maj = gss_acquire_cred(
              &min,
              GSS_C_NO_NAME,
              0,
              &desired_mech,
              GSS_C_ACCEPT,
              &cred_handle_svr,
              NULL,
              NULL);
    if (maj)
    {
        dwError = min ? min : maj;
        BAIL_ON_ERROR(dwError);
    }

    do
    {
        maj = gss_init_sec_context(
                    &min,
                    cred_handle_cli,
                    &gss_ctx_cli,
                    cred_name_buf,
#if 1
                    oid_spnego,
#else
                    (gss_OID) mech_ntlm,
#endif
                    req_flags,
                    time_req,
                    GSS_C_NO_CHANNEL_BINDINGS,
                    &send_tok,
                    &actual_mech,
                    &recv_tok,
                    &ret_flags,
                    NULL /* time_rec */);
        if (recv_tok.length > 0)
        {
            maj = GSS_S_CONTINUE_NEEDED;
        }
        if (maj == GSS_S_COMPLETE)
        {
            break;
        }
        else if (maj != GSS_S_CONTINUE_NEEDED)
        {
            dwError = min ? min : maj;
            BAIL_ON_ERROR(dwError);
        }
        if (send_tok.value)
        {
            gss_release_buffer(&min, &send_tok);
        }

        maj = gss_accept_sec_context(
            &min,
            &gss_ctx_svr,
            cred_handle_svr,
            &recv_tok,
            GSS_C_NO_CHANNEL_BINDINGS,
            NULL,
            NULL,
            &send_tok,
            &ret_flags,
            NULL,       /* ignore time_rec */
            NULL);    /* ignore del_cred_handle */
        if (recv_tok.value)
        {
            gss_release_buffer(&min, &recv_tok);
        }
        if (send_tok.length > 0)
        {
            maj = GSS_S_CONTINUE_NEEDED;
        }
    } while (maj == GSS_S_CONTINUE_NEEDED);

    if (maj != GSS_S_COMPLETE)
    {
        dwError = min ? min : maj;
    }

error:
    if (recv_tok.value)
    {
        gss_release_buffer(&min, &recv_tok);
    }
    if (send_tok.value)
    {
        gss_release_buffer(&min, &send_tok);
    }
    if (gss_ctx_cli != GSS_C_NO_CONTEXT)
    {
        gss_delete_sec_context(&min, &gss_ctx_cli, GSS_C_NO_BUFFER);
    }
    if (gss_ctx_svr != GSS_C_NO_CONTEXT)
    {
        gss_delete_sec_context(&min, &gss_ctx_svr , GSS_C_NO_BUFFER);
    }
    if (gss_name_buf)
    {
        gss_release_name(&min, &gss_name_buf);
    }
    if (cred_name_buf)
    {
        gss_release_name(&min, &cred_name_buf);
    }
    if (disp_name_buf.value)
    {
        gss_release_buffer(&min, &disp_name_buf);
    }
    if (cred_handle_cli)
    {
        gss_release_cred(&min, &cred_handle_cli);
    }
    if (cred_handle_svr)
    {
        gss_release_cred(&min, &cred_handle_svr);
    }

    IDM_SAFE_FREE_MEMORY(pszUserName);
    IDM_SAFE_FREE_MEMORY(pszPassword);
    IDM_SAFE_FREE_MEMORY(pszDomainName);
    IDM_SAFE_FREE_MEMORY(pszUpnName);
    return dwError;

}

void
PrintUserInfo(PIDM_USER_INFO pIdmUserInformation)
{
    PSTR pszStr = NULL;
    PSTR pszSid = NULL;
    DWORD groupCount = 0;

    if (!pIdmUserInformation)
    {
        return;
    }

    LwRtlCStringAllocateFromWC16String(
        &pszStr,
        pIdmUserInformation->pszUserName);
    LwRtlCStringAllocateFromWC16String(
        &pszSid,
        pIdmUserInformation->pszUserSid);
    printf("User Name is: %s\n", pszStr);
    printf("User SID is: %s\n\n", pszSid);
    IDM_SAFE_FREE_MEMORY(pszStr);
    IDM_SAFE_FREE_MEMORY(pszSid);
    printf("Groups: %d\n", pIdmUserInformation->dwNumGroups);
    for (groupCount=0; groupCount < pIdmUserInformation->dwNumGroups; groupCount++)
    {
        LwRtlCStringAllocateFromWC16String(
            &pszSid,
            pIdmUserInformation->ppszSids[groupCount]);
        LwRtlCStringAllocateFromWC16String(
            &pszStr,
            pIdmUserInformation->ppszGroupNames[groupCount]);
        printf("Group[%d] = %-20s: \t%s\n", groupCount, pszSid, pszStr);
        IDM_SAFE_FREE_MEMORY(pszSid);
        IDM_SAFE_FREE_MEMORY(pszStr);
    }
}

void usage(char *msg)
{
    if (msg)
    {
        printf("%s: unknown option '%s'\n", argv0, msg);
    }
    printf("usage: %s [--options ...] user domain [passwd]\n", argv0);
    printf("       %*s --srp: Try GSS_SRP authentication\n", argv0len, " ");
    printf("       %*s --ntlm: Try GSS_NTLM authentication\n", argv0len, " ");
    exit(1);
}

#ifndef _WIN32
struct AuthData {
    PWSTR user;
    PWSTR domain;
    PWSTR pwd;
};

void *auth(void* authData) {
    PIDM_USER_INFO pIdmUserInformation = NULL;
    DWORD dwError = 0;
    struct AuthData *ptr = (struct AuthData *)authData;
    char * user = NULL;
    DWORD ptid = (DWORD)pthread_self();

    LwRtlCStringAllocateFromWC16String(&user, ptr->user);
    printf("TID: %d Auth user: %s\n", ptid, user);

    dwError = IDMAuthenticateUser(
                      ptr->user,
                      ptr->domain,
                      ptr->pwd,
                      &pIdmUserInformation);
    if (dwError)
    {
        printf("TID: %d IDMAuthenticateUser: Failed 0x%x\n", ptid, dwError);
        return NULL;
    }
    else
    {
        LwRtlCStringAllocateFromWC16String(
            &user,
            pIdmUserInformation->pszUserName);
        printf("TID: %d Successfully logged on, User Name is: %s\n", ptid, user);
        IDMFreeUserInfo(pIdmUserInformation);
    }

    return NULL;
}

void testAuthWithThreads(int threadCount) {
    struct AuthData inputs[threadCount];
    pthread_t auth_thread[threadCount];
    char * user;
    char * pwd;
    int i=0;
    for(i=0; i<threadCount; i++) {
        LwRtlWC16StringAllocateFromCString(&inputs[i].domain, "ssolabs.com");
        IDMAllocateStringA("aduser_0000", &user);
        user[strlen(user) - 1] = '0' + i;
        LwRtlWC16StringAllocateFromCString(&inputs[i].user, user);
        IDMAllocateStringA("pwd_0000", &pwd);
        pwd[strlen(pwd) - 1] = '0' + i;
        LwRtlWC16StringAllocateFromCString(&inputs[i].pwd, pwd);

        if(pthread_create(&auth_thread[i], NULL, auth, (void *) &(inputs[i]))) {
            printf("Error creating thread\n");
            return;
        }
    }

    for(i=0; i<threadCount; i++) {
        if(pthread_join(auth_thread[i], NULL)) {
            printf("Error joining thread\n");
            return;
        }
    }
}
#endif

int main(int argc, char *argv[])
{
    DWORD krb_err = 0;
    DWORD dwError = 0;
    PIDM_USER_INFO pIdmUserInformation = NULL;
    PWSTR user = NULL;
    PWSTR domain = NULL;
    PWSTR pwd = NULL;
    char *pass = NULL;
    char passbuf[80];
    int optcnt = 1; /* Ignore argv[0] initially */
    int do_srp_auth = FALSE;
    int do_ntlm_auth = FALSE;
    int do_threads = FALSE;

    setlocale(LC_ALL, "");
    argv0 = (argv0=strrchr(argv[0], '/')) ? argv0+1 : argv[0];
    argv0len = (int) strlen(argv0);

    do
    {
        if (optcnt < argc && strncmp(argv[optcnt], "--", 2) != 0)
        {
            break;
        }
        else if (optcnt < argc && strcmp(argv[optcnt], "--srp") == 0)
        {
            do_srp_auth = TRUE;
            optcnt++;
        }
        else if (optcnt < argc && strcmp(argv[optcnt], "--ntlm") == 0)
        {
            do_ntlm_auth = TRUE;
            optcnt++;
        }
        else if (optcnt < argc && strcmp(argv[optcnt], "--threads") == 0)
        {
            do_threads = TRUE;
            optcnt++;
        }
        /* else if (!strcmp(argv[optcnt], "--other-switches-here)) */
        else
        {
            usage(argv[optcnt]);
        }
    } while (optcnt < argc);
    optcnt--; /* Remove argv[0] assumption */

    if (!do_threads && (argc-optcnt) < 3)
    {
        usage(NULL);
    }
    dwError = LwRtlWC16StringAllocateFromCString(&user, argv[1+optcnt]);
    if (dwError) goto error;

    dwError = LwRtlWC16StringAllocateFromCString(&domain, argv[2+optcnt]);
    if (dwError) goto error;

    if ((argc-optcnt) == 3)
    {
        pass = getpass("password: ");
        passbuf[0] = '\0';
        strncat(passbuf, pass, sizeof(passbuf)-1);
        pass = passbuf;
    }
    else
    {
        pass = argv[3+optcnt];
    }
    LwRtlWC16StringAllocateFromCString(&pwd, pass);

    if (do_srp_auth)
    {
        dwError = IDMAuthenticateUserSrp(
                      user,
                      domain,
                      pwd,
                      &pIdmUserInformation);
        krb_err = dwError;
    }
    else if (do_ntlm_auth)
    {
        dwError = IDMAuthenticateUserNtlm(
                      user,
                      domain,
                      pwd,
                      &pIdmUserInformation);
        krb_err = dwError;
    }
    else if(do_threads)
    {
#ifndef _WIN32
        testAuthWithThreads(9);
        IDMDestroySidCache();
#endif
    }
    else
    {
        krb_err = IDMAuthenticateUser(
                      user,
                      domain,
                      pwd,
                      &pIdmUserInformation);

        if (krb_err)
        {
            printf("IDMAuthenticateUser: Failed 0x%x\n", krb_err);
            return 1;
        }
        else
        {
            printf("Successfully logged on\n");
            PrintUserInfo(pIdmUserInformation);
            IDMFreeUserInfo(pIdmUserInformation);
        }
        IDMDestroySidCache();
    }

error:
    IDM_SAFE_FREE_MEMORY(user);
    IDM_SAFE_FREE_MEMORY(domain);
    IDM_SAFE_FREE_MEMORY(pwd);

    return 0;
}
