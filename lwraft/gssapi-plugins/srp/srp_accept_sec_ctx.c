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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <krb5.h>
#include "gssapiP_srp.h"
#include "gssapi_srp.h"
#include "gssapi_alloc.h"
#include "srp_util.h"
#include <lber.h>

#include <vmdirdefines.h>
#include "includes.h"
#include "srprpc.h"
#include <client/structs.h>

#include <config.h>

#ifdef _WIN32

#include <Winsock2.h>
#ifndef snprintf
#define snprintf _snprintf

#endif
#else /* Linux */

#include <arpa/inet.h>

#endif

/*
 * Win32/Likewise data types defined here, vs pulling in
 * Likewise headers, which pulls in undesired library dependencies.
 */
#include <errno.h>
#ifndef DWORD
#define DWORD unsigned int
#endif
#ifndef PBYTE
#define PBYTE unsigned char *
#endif
#ifndef PSTR
#define PSTR char *
#endif
#ifndef PCSTR
#define PCSTR const char *
#endif

#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#include <csrp/srp.h>
#include <dce/rpc.h>
#include "srp_encrypt.h"

ULONG
VmDirCreateBindingHandleAuthA(
    PCSTR     pszNetworkAddress,
    PCSTR     pszNetworkEndpoint,
    PCSTR     pszUserName,
    PCSTR     pszDomain,
    PCSTR     pszPassword,
    handle_t  *ppBinding);

static
OM_uint32
srp_gss_validate_oid_header(
    OM_uint32 *minor_status,
    gss_buffer_t in_tok,
    int *object_len)
{
    unsigned char *ptr = NULL;
    OM_uint32 maj = 0;
    int len = 0;
    int oid_len = 0;
    int enc_token_len = 0;
    int token_len = 0;

    *minor_status = 0;
    if (!in_tok || in_tok->length == 0 || !in_tok->value)
    {
        maj = GSS_S_NO_CONTEXT;
        goto error;
    }

    /*
     * tag for APPLICATION 0, Sequence[constructed, definite length]
     * length of remainder of token
     * tag of OBJECT IDENTIFIER
     * length of mechanism OID
     * encoding of mechanism OID
     * <the rest of the token>
     *
     * Numerically, this looks like :
     *
     * 0x60
     * <length> - could be multiple bytes
     * 0x06
     * <length> - assume only one byte, hence OID length < 127
     * <mech OID bytes>
     *
     */
    ptr = in_tok->value;
    len = (int) in_tok->length;
    if (*ptr != 0x60)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }
    len--, ptr++;

    enc_token_len = (int) *ptr;
    token_len = 0;
    len--, ptr++;

    if (*ptr != 0x06)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }
    len--, ptr++;
    token_len++;

    if (len == 0)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }
    oid_len = *ptr;
    len--, ptr++;
    token_len++;

    if (len < oid_len ||
        (len < (int) GSS_SRP_MECH_OID_LEN_ST) ||
        (len < (int) GSSAPI_SRP_MECH_OID_LEN_ST))
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }

    if ((oid_len != GSS_SRP_MECH_OID_LEN_ST && oid_len != GSSAPI_SRP_MECH_OID_LEN_ST) ||
        (memcmp(ptr, GSS_SRP_MECH_OID_ST, oid_len) != 0 &&
         memcmp(ptr, GSSAPI_SRP_MECH_OID_ST, oid_len) != 0))
    {
        maj = GSS_S_BAD_MECH;
        goto error;
    }
    token_len += oid_len;

    if (token_len != enc_token_len)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }

    len -= oid_len, ptr += oid_len;

    *object_len = (int) (ptr - (unsigned char *) in_tok->value);
error:
    return maj;
}

static
OM_uint32
_srp_gss_auth_create_machine_acct_binding(
    OM_uint32 *minor_status,
    PVMDIR_SERVER_CONTEXT *hRetServer)
{
    DWORD dwError = 0;
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    int domainState = 0;
    char *machine_acct_upn = NULL;
    char *machine_acct_pwd = NULL;
    char *hostname = NULL;
    void *hRegistry = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    dwError = srp_reg_get_handle((void **) &hRegistry);
    if (dwError)
    {
        maj = GSS_S_FAILURE;
        min = dwError;
        goto error;
    }

    /* Determine if this system is a management node */
    dwError = srp_reg_get_domain_state(hRegistry, &domainState);
    if (dwError)
    {
        domainState = 1;
    }

    /* Value "2" is a management node: Perform SRP pass-through */
    if (domainState == 2)
    {
        dwError = srp_reg_get_machine_acct_upn(
                      hRegistry,
                      &machine_acct_upn);
        if (dwError)
        {
            maj = GSS_S_FAILURE;
            min = dwError;
            goto error;
        }

        dwError = srp_reg_get_machine_acct_password(
                      hRegistry,
                      &machine_acct_pwd);
        if (dwError)
        {
            maj = GSS_S_FAILURE;
            min = dwError;
            goto error;
        }

        dwError = srp_reg_get_dc_name(
                      hRegistry,
                      &hostname);
        if (dwError)
        {
            maj = GSS_S_FAILURE;
            min = dwError;
            goto error;
        }
    }


    /*
     * This will create a remote binding handle when credentials are
     * provided, or if local, will use ncalrpc.
     */
    dwError = VmDirOpenServerA(
                  hostname,
                  machine_acct_upn, /* UPN doesn't need domain name */
                  NULL,
                  machine_acct_pwd,
                  0,
                  NULL,
                  &hServer);
    BAIL_ON_VMDIR_ERROR(dwError);

    *hRetServer = hServer;

error:
    if (machine_acct_upn)
    {
        free(machine_acct_upn);
    }
    if (machine_acct_pwd)
    {
        free(machine_acct_pwd);
    }
    if (hostname)
    {
        free(hostname);
    }
    if (hRegistry)
    {
        srp_reg_close_handle(hRegistry);
    }
    if (maj)
    {
        if (min)
        {
            *minor_status = min;
        }
    }
    return maj;
}

static
OM_uint32
_srp_gss_auth_init(
    OM_uint32 *minor_status,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    ber_tag_t ber_state = 0;
    struct berval ber_ctx = {0};
    struct berval *ber_upn = NULL;
    struct berval *ber_bytes_A = NULL;
    struct berval ber_salt = {0};
    struct berval ber_mda = {0};
    struct berval ber_B = {0};
    struct berval *flatten = NULL;
    BerElement *ber = NULL;
    BerElement *ber_resp = NULL;
    int berror = 0;
    int sts = 0;
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    OM_uint32 min_tmp = 0;
    gss_buffer_desc tmp_in_tok = {0};
    gss_buffer_desc disp_name_buf = {0};
    gss_buffer_t disp_name = NULL;
    gss_OID disp_name_OID = NULL;
    char *srp_upn_name = NULL;
    int srp_decode_mda_len = 0;
    int srp_decode_salt_len = 0;
    const unsigned char *srp_mda = NULL;
    const unsigned char *srp_salt = NULL;
    SRP_HashAlgorithm hash_alg = SRP_SHA1;
    SRP_NGType ng_type = SRP_NG_2048;
    struct SRPVerifier *ver = NULL;
    const unsigned char *srp_bytes_B = NULL;
    int srp_bytes_B_len = 0;
    const unsigned char *srp_session_key = NULL;
    unsigned char *ret_srp_session_key = NULL;
    int srp_session_key_len = 0;
    ber_int_t gss_srp_version_maj = 0;
    ber_int_t gss_srp_version_min = 0;
    PVMDIR_SERVER_CONTEXT hServer = NULL;
    srp_verifier_handle_t hSrp = NULL; /* aliased / cast to "ver" variable */

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_srp_gss_auth_init(): state=SRP_AUTH_INIT\n");

    /*
     * ptr points to ASN.1 encoded data which is dependent on the authentication
     * state. The appropriate decoder format string is applied for each state
     */
    berror = ber_scanf(ber, "t{ii",
                       &ber_state, &gss_srp_version_maj, &gss_srp_version_min);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    berror = ber_scanf(ber, "OO}", &ber_upn, &ber_bytes_A);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_srp_gss_auth_init(accept_sec_context): protocol version %d.%d\n",
                     gss_srp_version_maj, gss_srp_version_min);
    srp_print_hex(ber_bytes_A->bv_val,
                  (int) ber_bytes_A->bv_len,
                  "_srp_gss_auth_init(accept_sec_context): bytes_A");
    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    tmp_in_tok.value = ber_upn->bv_val;
    tmp_in_tok.length = ber_upn->bv_len;
    maj = gss_import_name(&min,
                          &tmp_in_tok,
                          NULL,
                          &srp_context_handle->gss_upn_name);
    if (maj)
    {
        goto error;
    }

    maj = gss_display_name(&min,
                           srp_context_handle->gss_upn_name,
                           &disp_name_buf,
                           &disp_name_OID);
    if (maj)
    {
        goto error;
    }

    disp_name = &disp_name_buf;
    srp_debug_printf("srp_gss_accept_sec_context: UPN name=%.*s\n",
                     (int) disp_name_buf.length, (char *) disp_name_buf.value);

    srp_upn_name = calloc(disp_name_buf.length + 1, sizeof(char));
    if (!srp_upn_name)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }
    snprintf(srp_upn_name,
             disp_name_buf.length+1,
             "%.*s",
             (int) disp_name_buf.length,
             (char *) disp_name_buf.value);


    maj = _srp_gss_auth_create_machine_acct_binding(
              &min,
              &hServer);
    if (maj)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    sts = cli_rpc_srp_verifier_new(
            hServer ? hServer->hBinding : NULL,
            hash_alg,
            ng_type,
            srp_upn_name,
            ber_bytes_A->bv_val, (int) ber_bytes_A->bv_len,
            &srp_bytes_B, &srp_bytes_B_len,
            &srp_salt, &srp_decode_salt_len,
            &srp_mda, &srp_decode_mda_len,
            NULL, NULL, /* n_hex, g_hex */
            &hSrp);
    if (sts)
    {
        maj = GSS_S_FAILURE;
        min = sts;
        goto error;
    }
    ver = (struct SRPVerifier *) hSrp, hSrp = NULL;

    if (!srp_bytes_B)
    {
        srp_debug_printf("srp_verifier_new: failed!\n");
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_print_hex(srp_salt, srp_decode_salt_len,
                  "_srp_gss_auth_init(accept_sec_context): srp_salt value");
    srp_print_hex(srp_bytes_B, srp_bytes_B_len,
                  "_srp_gss_auth_init(accept_sec_context): srp_B value");
    ber_mda.bv_val = (unsigned char *) srp_mda;
    ber_mda.bv_len = srp_decode_mda_len;

    ber_salt.bv_val = (unsigned char *) srp_salt;
    ber_salt.bv_len = srp_decode_salt_len;
    /*
     * B is computed: (kv + g**b) % N
     */
    ber_B.bv_val = (void *) srp_bytes_B;
    ber_B.bv_len = srp_bytes_B_len;

    ber_resp = ber_alloc_t(LBER_USE_DER);
    if (!ber_resp)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }

    /*
     * Response format:
     * tag | MDA | salt | B
     */
    berror = ber_printf(ber_resp, "t{OOO}",
                 SRP_AUTH_SALT_RESP,
                 &ber_mda,
                 &ber_salt,
                 &ber_B);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    berror = ber_flatten(ber_resp, &flatten);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    output_token->value = gssalloc_calloc(1, flatten->bv_len);
    if (!output_token->value)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    output_token->length = flatten->bv_len;
    memcpy(output_token->value, flatten->bv_val, flatten->bv_len);

    sts = cli_rpc_srp_verifier_get_session_key(
        hServer ? hServer->hBinding : NULL,
        ver,
        &srp_session_key,
        &srp_session_key_len);
    if (sts)
    {
        min = sts;
        maj = GSS_S_FAILURE;
        goto error;
    }

    if (srp_session_key && srp_session_key_len > 0)
    {
        ret_srp_session_key =
            calloc(srp_session_key_len, sizeof(unsigned char));
        if (!ret_srp_session_key)
        {
            maj = GSS_S_FAILURE;
            min = ENOMEM;
            goto error;
        }
    }
    memcpy(ret_srp_session_key,
           srp_session_key,
           srp_session_key_len);

    /* Set context handle/return values here; all previous calls succeeded */
    maj = GSS_S_CONTINUE_NEEDED;
    srp_context_handle->hServer = hServer, hServer = NULL;

    /* Used in generating Kerberos keyblock salt value */
    srp_context_handle->upn_name = srp_upn_name, srp_upn_name = NULL;
    srp_context_handle->srp_ver = ver, ver = NULL;

    /* Return the SRP session key in the context handle */
    srp_context_handle->srp_session_key_len = srp_session_key_len;
    srp_context_handle->srp_session_key = ret_srp_session_key, ret_srp_session_key = NULL;

    srp_print_hex(srp_session_key, srp_session_key_len,
                  "_srp_gss_auth_init(accept_sec_ctx) got session key");

error:
    if (ver)
    {
        cli_rpc_srp_verifier_delete(
            hServer ? hServer->hBinding : NULL,
            (void **) &ver);
    }
    VmDirCloseServer(hServer);
    if (srp_upn_name)
    {
        free(srp_upn_name);
    }
    if (ber_upn)
    {
        ber_bvfree(ber_upn);
    }
    if (ber_bytes_A)
    {
        ber_bvfree(ber_bytes_A);
    }
    ber_bvfree(flatten);
    ber_free(ber, 1);
    ber_free(ber_resp, 1);

    if (disp_name)
    {
        gss_release_buffer(&min_tmp, disp_name);
    }
    if (srp_bytes_B)
    {
        free((void *) srp_bytes_B);
    }
    if (srp_salt)
    {
        free((void *) srp_salt);
    }
    if (srp_mda)
    {
        free((void *) srp_mda);
    }
    if (srp_session_key)
    {
        free((void *) srp_session_key);
    }
    if (ret_srp_session_key)
    {
        free((void *) ret_srp_session_key);
    }

    if (maj)
    {
        if (min)
        {
            *minor_status = min;
        }
    }
    return maj;
}

static
OM_uint32
_srp_gss_validate_client(
    OM_uint32 *minor_status,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    int berror = 0;
    ber_tag_t ber_state = 0;
    BerElement *ber = NULL;
    BerElement *ber_resp = NULL;
    struct berval ber_HAMK = {0};
    struct berval *ber_srp_bytes_M = NULL;
    struct berval ber_ctx = {0};
    const unsigned char *bytes_HAMK = NULL;
    int bytes_HAMK_len = 0;
    struct berval *flatten = NULL;
    PVMDIR_SERVER_CONTEXT hServer = NULL;

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_srp_gss_validate_client(): "
                     "state=SRP_AUTH_CLIENT_VALIDATE\n");

    /*
     * ptr points to ASN.1 encoded data which is dependent on the authentication
     * state. The appropriate decoder format string is applied for each state
     */
    berror = ber_scanf(ber, "t{O}", &ber_state, &ber_srp_bytes_M);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        min = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }

    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state || ber_srp_bytes_M->bv_len == 0)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_print_hex(ber_srp_bytes_M->bv_val, (int) ber_srp_bytes_M->bv_len,
                  "_srp_gss_validate_client(accept_sec_ctx) received bytes_M");

    hServer = srp_context_handle->hServer;
    min = cli_rpc_srp_verifier_verify_session(
              hServer->hBinding,
              srp_context_handle->srp_ver,
              ber_srp_bytes_M->bv_val, (int) ber_srp_bytes_M->bv_len,
              &bytes_HAMK, &bytes_HAMK_len);
    if (min || !bytes_HAMK)
    {
        /*
         * Bad password will cause this to fail. Do not bail on error here.
         * Merely generate a NULL HAMK response below, to complete the
         * SRP protocol exchange with the client. The client tests for an
         * empty HAMK token, and formulates the proper error.
         */
        srp_debug_printf("_srp_gss_validate_client: "
                         "srp_verifier_verify_session() failed!!!\n");
    }

    /*
     * ASN.1 encode the bytes_HAMK value, sending it back to the client
     * for validation. That will complete the authentication process if that
     * succeeds.
     */

    ber_resp = ber_alloc_t(LBER_USE_DER);
    if (!ber_resp)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }
    if (min == 0)
    {
        /*
         * Generate HAMK response. When min is an error code,
         * an empty HAMK response (zero length) is created.
         */
        min = cli_rpc_srp_verifier_get_session_key_length(
                  hServer->hBinding,
                  srp_context_handle->srp_ver,
                  (long *) &ber_HAMK.bv_len);
        if (min)
        {
            maj = GSS_S_FAILURE;
            goto error;
        }
    }

    ber_HAMK.bv_val = (void *) bytes_HAMK;
    berror = ber_printf(ber_resp, "t{O}",
                  (int) SRP_AUTH_SERVER_VALIDATE,
                  &ber_HAMK);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    berror = ber_flatten(ber_resp, &flatten);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    output_token->value = gssalloc_calloc(1, flatten->bv_len);
    if (!output_token->value)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    output_token->length = flatten->bv_len;
    memcpy(output_token->value, flatten->bv_val, flatten->bv_len);

    /*
     * From server's perspective, authentication is done. However,
     * there is a final output_token to process by gss_init_sec_context().
     */
    maj = GSS_S_COMPLETE;

error:
    if (ber_srp_bytes_M)
    {
        ber_bvfree(ber_srp_bytes_M);
    }
    if (bytes_HAMK)
    {
        free((void *) bytes_HAMK);
    }
    ber_bvfree(flatten);
    ber_free(ber, 1);
    ber_free(ber_resp, 1);
    if (maj)
    {
        if (min)
        {
            *minor_status = min;
        }
    }
    return maj;
}


/*
 * Report error status to client, and the final
 * minor status from the server.
 * This is the end, my friend...
 */
static
OM_uint32
_srp_gss_accept_sec_ctx_error_resp(
    OM_uint32 *minor_status,
    gss_buffer_t output_token)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    int berror = 0;
    BerElement *ber_resp = NULL;
    struct berval *flatten = NULL;

    ber_resp = ber_alloc_t(LBER_USE_DER);
    if (!ber_resp)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }
    berror = ber_printf(ber_resp, "t{i}",
                  (int) SRP_AUTH_FAILED,
                  *minor_status);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    berror = ber_flatten(ber_resp, &flatten);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    output_token->value = gssalloc_calloc(1, flatten->bv_len);
    if (!output_token->value)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    output_token->length = flatten->bv_len;
    memcpy(output_token->value, flatten->bv_val, flatten->bv_len);

error:
    ber_bvfree(flatten);
    ber_free(ber_resp, 1);
    if (maj)
    {
        /* Cleanup return memory stuff here */
    }

    return maj;
}

OM_uint32
srp_gss_accept_sec_context(
                OM_uint32 *minor_status,
                gss_ctx_id_t *context_handle,
                gss_cred_id_t verifier_cred_handle,
                gss_buffer_t input_token,
                gss_channel_bindings_t input_chan_bindings,
                gss_name_t *src_name,
                gss_OID *mech_type,
                gss_buffer_t output_token,
                OM_uint32 *ret_flags,
                OM_uint32 *time_rec,
                gss_cred_id_t *delegated_cred_handle)
{
    int oid_len = 0;
    int state = 0;
    srp_gss_cred_id_t srp_cred = NULL;
    unsigned char *ptr = NULL;
    int ptr_len = 0;
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    OM_uint32 tmp_maj = 0;
    OM_uint32 tmp_min = 0;
    gss_buffer_desc input_token_srp = {0};
    srp_gss_ctx_id_t srp_context_handle = NULL;
    krb5_error_code krb5_err = 0;
    gss_cred_id_t srp_cred_handle = NULL;

    if (minor_status == NULL ||
        output_token == GSS_C_NO_BUFFER ||
        context_handle == NULL)
    {
        return GSS_S_CALL_INACCESSIBLE_WRITE;
    }

    if (input_token == GSS_C_NO_BUFFER)
    {
        return GSS_S_CALL_INACCESSIBLE_READ;
    }

    if (minor_status)
    {
        *minor_status = 0;
    }

    if (output_token != GSS_C_NO_BUFFER)
    {
        output_token->length = 0;
        output_token->value = NULL;
    }

    if (!context_handle)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    if (*context_handle)
    {
        srp_context_handle = (srp_gss_ctx_id_t) *context_handle;
    }
    else
    {
        /* First call, allocate context handle */
        srp_context_handle =
            (srp_gss_ctx_id_t) calloc(1, sizeof(srp_gss_ctx_id_rec));
        if (!srp_context_handle)
        {
            min = ENOMEM;
            maj = GSS_S_FAILURE;
            goto error;
        }
        memset(srp_context_handle, 0, sizeof(srp_gss_ctx_id_rec));

        /* Needed for Kerberos AES256-SHA1 keyblock generation */
        krb5_err = krb5_init_context(&srp_context_handle->krb5_ctx);
        if (krb5_err)
        {
            maj = GSS_S_FAILURE;
            min = krb5_err;
            goto error;
        }
        maj = srp_gss_acquire_cred(
                  &min,
                  GSS_C_NO_NAME,
                  0,
                  NULL,
                  GSS_C_ACCEPT,
                  &srp_cred_handle,
                  NULL,
                  NULL);
        if (maj)
        {
            goto error;
        }
        srp_cred = (srp_gss_cred_id_t) srp_cred_handle;
        srp_context_handle->magic_num = SRP_MAGIC_ID;

        maj = srp_gss_duplicate_oid(&min,
                                    srp_cred->srp_mech_oid,
                                    &srp_context_handle->mech);
        if (maj)
        {
            goto error;
        }

        srp_context_handle->state = SRP_AUTH_INIT;
        srp_context_handle->cred = (srp_gss_cred_id_t) verifier_cred_handle;
        *context_handle = (gss_ctx_id_t) srp_context_handle;
    }

    ptr = (unsigned char*) input_token->value;
    ptr_len = (int) input_token->length;
    maj = srp_gss_validate_oid_header(
              &min,
              input_token,
              &oid_len);
    if (maj)
    {
        goto error;
    }

    ptr += oid_len;
    ptr_len -= oid_len;
    input_token_srp.value = ptr;
    input_token_srp.length = ptr_len;

    /* This is the "t" field of ber_scanf() */
    state = SRP_AUTH_STATE_VALUE(ptr[0]);

    /* Verify state machine is consistent with expected state */
    state = SRP_AUTH_STATE_VALUE(ptr[0]);

    if (state != srp_context_handle->state)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    switch(state)
    {
      case SRP_AUTH_INIT:
        srp_debug_printf("srp_gss_accept_sec_context: state=SRP_AUTH_INIT\n");
        maj = _srp_gss_auth_init(minor_status,
                                 srp_context_handle,
                                 state,
                                 &input_token_srp,
                                 output_token);
        if (maj)
        {
            if (maj == GSS_S_CONTINUE_NEEDED)
            {
                srp_context_handle->state = SRP_AUTH_CLIENT_VALIDATE;
            }
            goto error;
        }
        break;

      case SRP_AUTH_CLIENT_VALIDATE:
        srp_debug_printf("srp_gss_accept_sec_context: "
                         "state=SRP_AUTH_CLIENT_VALIDATE\n");
        maj = _srp_gss_validate_client(minor_status,
                                       srp_context_handle,
                                       state,
                                       &input_token_srp,
                                       output_token);
        if (maj != GSS_S_CONTINUE_NEEDED && maj != GSS_S_COMPLETE)
        {
            /* Hard error occurred */
            goto error;
        }

        srp_context_handle->state = SRP_AUTH_COMPLETE;
        if (mech_type)
        {
            /* The security mechanism with which the context was established.
             * If the security mechanism type is not required, specify NULL
             * for this parameter. The gss_OID value returned for this
             * parameter points to a read-only structure and must not be
             * released by the application.
             */
            *mech_type = srp_context_handle->mech;
        }

        if (src_name)
        {
            /* Optional: Return UPN name to caller */
            tmp_maj = gss_duplicate_name(
                      &tmp_min,
                      srp_context_handle->gss_upn_name,
                      src_name);
            if (tmp_maj)
            {
                maj = tmp_maj;
                *minor_status = tmp_min;
                goto error;
            }
        }
        break;

      /* This should never happen, but include for completeness-sake */
      case SRP_AUTH_COMPLETE:
        srp_debug_printf("srp_gss_accept_sec_context: "
                         "state=SRP_AUTH_COMPLETE\n");
        maj = GSS_S_COMPLETE;
        break;

      default:
        srp_debug_printf("srp_gss_accept_sec_context: state=UNKNOWN!!!\n");
        maj = GSS_S_FAILURE;
        goto error;
        break;
    }

    if (srp_context_handle->state == SRP_AUTH_COMPLETE)
    {
        PVMDIR_SERVER_CONTEXT hServer = srp_context_handle->hServer;

        krb5_err = srp_make_enc_keyblock(srp_context_handle);
        if (krb5_err)
        {
            maj = GSS_S_FAILURE;
            min = krb5_err;
            goto error;
        }

        /* Clean up SRP server-side memory, then close the server context */
        cli_rpc_srp_verifier_delete(
            hServer->hBinding,
            (void **) &srp_context_handle->srp_ver);

        VmDirCloseServer(hServer);
        srp_context_handle->hServer = NULL;
    }

error:
    if (maj != GSS_S_CONTINUE_NEEDED && maj != GSS_S_COMPLETE)
    {
        _srp_gss_accept_sec_ctx_error_resp(
            minor_status,
            output_token);
    }

    if (srp_cred_handle)
    {
        srp_gss_release_cred(&tmp_min, &srp_cred_handle);
    }
    return maj;
}
