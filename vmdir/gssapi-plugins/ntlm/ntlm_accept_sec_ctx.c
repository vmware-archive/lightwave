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
#include "gssapiP_ntlm.h"
#include "gssapi_ntlm.h"
#include "gssapi_alloc.h"
#include "ntlm_util.h"
#include <lber.h>

#ifndef _WIN32
#include <config.h>
#endif

#ifdef _WIN32

#include <Winsock2.h>
#ifndef snprintf
#define snprintf _snprintf

#endif
#else

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

extern
DWORD
VmDirGetSRPSecret(
    PCSTR       pszUPN,
    PBYTE*      ppSecretBlob,
    DWORD*      pSize
    );


#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#include <csrp/srp.h>
#include "ntlm_encrypt.h"

static
OM_uint32
ntlm_gss_validate_oid_header(
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

    if (len < oid_len || len < (int) GSS_NTLM_MECH_OID_LEN)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }

    if (memcmp(ptr, NTLM_OID, GSS_NTLM_MECH_OID_LEN) != 0)
    {
        maj = GSS_S_BAD_MECH;
        goto error;
    }
    token_len += GSS_NTLM_MECH_OID_LEN;

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
_ntlm_gss_auth_init(
    OM_uint32 *minor_status,
    ntlm_gss_ctx_id_t ntlm_context_handle,
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
    char *ntlm_upn_name = NULL;
    char *ntlm_secret = NULL;
    unsigned int ntlm_secret_len = 0;
    unsigned int ntlm_secret_len_max = 0;
    unsigned char *ntlm_secret_str = NULL;
    unsigned int ntlm_secret_str_len = 0;
    uint32_t ntlm_decode_buf_len = 0;
    uint16_t ntlm_decode_mda_len = 0;
    uint16_t ntlm_decode_v_len = 0;
    uint8_t ntlm_decode_salt_len = 0;
    char *ntlm_decode_ptr = NULL;
    char *ntlm_mda = NULL;
    char *ntlm_v = NULL;
    char *ntlm_salt = NULL;
    SRP_HashAlgorithm hash_alg = SRP_SHA1;
    SRP_NGType ng_type = SRP_NG_2048;
    struct SRPVerifier *ver = NULL;
    const unsigned char *ntlm_bytes_B = NULL;
    int ntlm_bytes_B_len = 0;
    const unsigned char *ntlm_session_key = NULL;
    int ntlm_session_key_len = 0;

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    ntlm_debug_printf("_ntlm_gss_auth_init(): state=NTLM_AUTH_INIT\n");

    /*
     * ptr points to ASN.1 encoded data which is dependent on the authentication
     * state. The appropriate decoder format string is applied for each state
     */
    berror = ber_scanf(ber, "t{OO}", &ber_state, &ber_upn, &ber_bytes_A);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

ntlm_print_hex(ber_bytes_A->bv_val, (int) ber_bytes_A->bv_len, "_ntlm_gss_auth_init(accept_sec_context): bytes_A");
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
                          &ntlm_context_handle->gss_upn_name);
    if (maj)
    {
        goto error;
    }

    maj = gss_display_name(&min,
                           ntlm_context_handle->gss_upn_name,
                           &disp_name_buf,
                           &disp_name_OID);
    if (maj)
    {
        goto error;
    }
    disp_name = &disp_name_buf;
    ntlm_debug_printf("ntlm_gss_accept_sec_context: UPN name=%.*s\n",
                     (int) disp_name_buf.length, (char *) disp_name_buf.value);

    ntlm_upn_name = calloc(disp_name_buf.length + 1, sizeof(char));
    if (!ntlm_upn_name)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }
    snprintf(ntlm_upn_name,
             disp_name_buf.length+1,
             "%.*s",
             (int) disp_name_buf.length,
             (char *) disp_name_buf.value);

    /* Used in generating Kerberos keyblock salt value */
    ntlm_context_handle->upn_name = ntlm_upn_name;
    ntlm_upn_name = NULL;

    // TODO: place holder to get NTLM secret from Lotus
    maj = GSS_S_UNAVAILABLE;
    if (maj)
    {
        goto error;
    }

    ntlm_debug_printf("base64 encoded secret: <%.*s>\n",
                     ntlm_secret_str_len, ntlm_secret_str);
    ntlm_secret = calloc(ntlm_secret_str_len, sizeof(char));
    ntlm_secret_len_max = ntlm_secret_str_len;
    sts = sasl_decode64(ntlm_secret_str,
                        ntlm_secret_str_len,
                        ntlm_secret,
                        ntlm_secret_len_max,
                        &ntlm_secret_len);
    if (sts == SASL_OK)
    {
        /*
         * Encoding of data blob (from common/ntlm.c):
         * calculate buffer size
         * mda: Message Digest Algorithm
         * v: NTLM private "hash" value
         * salt: random salt generated at "hash" creation time
         *
         * 0. 4 byte length
         * 1. utf8(mda) : 2 bytes + string
         * 2. mpi(v)    : 2 bytes + verifier
         * 3. os(salt)  : 1 bytes + salt
         */
        ntlm_decode_ptr = ntlm_secret;
        memcpy(&ntlm_decode_buf_len, ntlm_decode_ptr, sizeof(uint32_t));
        ntlm_decode_ptr += sizeof(uint32_t);
        ntlm_decode_buf_len = ntohl(ntlm_decode_buf_len);

        memcpy(&ntlm_decode_mda_len, ntlm_decode_ptr, sizeof(uint16_t));
        ntlm_decode_ptr += sizeof(uint16_t);
        ntlm_decode_mda_len = ntohs(ntlm_decode_mda_len);
        ntlm_mda = ntlm_decode_ptr;
        ntlm_decode_ptr += ntlm_decode_mda_len;

        memcpy(&ntlm_decode_v_len, ntlm_decode_ptr, sizeof(uint16_t));
        ntlm_decode_ptr += sizeof(uint16_t);
        ntlm_decode_v_len = ntohs(ntlm_decode_v_len);
        ntlm_v = ntlm_decode_ptr;
        ntlm_decode_ptr += ntlm_decode_v_len;

        memcpy(&ntlm_decode_salt_len, ntlm_decode_ptr, sizeof(uint8_t));
        ntlm_decode_ptr += sizeof(uint8_t);
        ntlm_salt = ntlm_decode_ptr;


        // What is the length? Is this binary/string data?
        ntlm_debug_printf("decoded buffer len=%d\n", (int) ntlm_secret_len);
        ntlm_debug_printf("ntlm_decode_buf_len=%d\n", (int) ntlm_decode_buf_len);
        ntlm_debug_printf("ntlm_decode_mda_len=%d\n", (int) ntlm_decode_mda_len);
        ntlm_debug_printf("ntlm_decode_v_len=%d\n", (int) ntlm_decode_v_len);
        ntlm_debug_printf("ntlm_decode_salt_len=%d\n", (int) ntlm_decode_salt_len);
    }

    /*
     * Create response token. This contains (s, B) for I
     */
    ver = srp_verifier_new(hash_alg,
                           ng_type,
                           ntlm_context_handle->upn_name,
                           ntlm_salt, (int) ntlm_decode_salt_len,
                           ntlm_v, (int) ntlm_decode_v_len,
                           ber_bytes_A->bv_val, (int) ber_bytes_A->bv_len,
                           &ntlm_bytes_B, &ntlm_bytes_B_len,
                           NULL, NULL /* n_hex, g_hex */ );
    if (!ntlm_bytes_B)
    {
        ntlm_debug_printf("srp_verifier_new: failed!\n");
        maj = GSS_S_FAILURE;
        goto error;
    }

    ntlm_print_hex(ntlm_salt, ntlm_decode_salt_len,
                  "_ntlm_gss_auth_init(accept_sec_context): ntlm_salt value");
    ntlm_print_hex(ntlm_v, ntlm_decode_v_len,
                  "_ntlm_gss_auth_init(accept_sec_context): ntlm_v value");
    ntlm_print_hex(ntlm_bytes_B, ntlm_bytes_B_len,
                  "_ntlm_gss_auth_init(accept_sec_context): ntlm_B value");
    ber_mda.bv_val = ntlm_mda;
    ber_mda.bv_len = ntlm_decode_mda_len;

    ber_salt.bv_val = ntlm_salt;
    ber_salt.bv_len = ntlm_decode_salt_len;
    /*
     * TBD: B is computed: (kv + g**b) % N
     * char *ntlm_v = NULL;
     */
    ber_B.bv_val = (void *) ntlm_bytes_B;
    ber_B.bv_len = ntlm_bytes_B_len;

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
                 NTLM_AUTH_SALT_RESP,
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
    ntlm_context_handle->ntlm_ver = ver;

    ntlm_session_key = srp_verifier_get_session_key(ver, &ntlm_session_key_len);
    if (ntlm_session_key && ntlm_session_key_len > 0)
    {
        ntlm_context_handle->ntlm_session_key =
            calloc(ntlm_session_key_len, sizeof(unsigned char));
        if (!ntlm_context_handle->ntlm_session_key)
        {
            maj = GSS_S_FAILURE;
            min = ENOMEM;
            goto error;
        }
        memcpy(ntlm_context_handle->ntlm_session_key,
               ntlm_session_key,
               ntlm_session_key_len);
        ntlm_context_handle->ntlm_session_key_len = ntlm_session_key_len;

#if  1 /* TBD: adam  Debug */
        ntlm_print_hex(ntlm_session_key, ntlm_session_key_len,
                      "_ntlm_gss_auth_init(accept_sec_ctx) got session key");
#endif
    }

    maj = GSS_S_CONTINUE_NEEDED;

error:
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
    if (ntlm_secret)
    {
        free(ntlm_secret);
    }
    if (ntlm_secret_str)
    {
        free(ntlm_secret_str);
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
_ntlm_gss_validate_client(
    OM_uint32 *minor_status,
    ntlm_gss_ctx_id_t ntlm_context_handle,
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
    struct berval *ber_ntlm_bytes_M = NULL;
    struct berval ber_ctx = {0};
    const unsigned char *bytes_HAMK = 0;
    struct berval *flatten = NULL;

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    ntlm_debug_printf("_ntlm_gss_validate_client(): "
                     "state=NTLM_AUTH_CLIENT_VALIDATE\n");

    /*
     * ptr points to ASN.1 encoded data which is dependent on the authentication
     * state. The appropriate decoder format string is applied for each state
     */
    berror = ber_scanf(ber, "t{O}", &ber_state, &ber_ntlm_bytes_M);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        min = EINVAL; /* TBD: Adam, return a real error code here */
        goto error;
    }

    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state || ber_ntlm_bytes_M->bv_len == 0)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    ntlm_print_hex(ber_ntlm_bytes_M->bv_val, (int) ber_ntlm_bytes_M->bv_len,
                  "_ntlm_gss_validate_client(accept_sec_ctx) received bytes_M");

    srp_verifier_verify_session(ntlm_context_handle->ntlm_ver,
                                ber_ntlm_bytes_M->bv_val, &bytes_HAMK);
    if (!bytes_HAMK)
    {
    ntlm_debug_printf("_ntlm_gss_validate_client: "
                     "srp_verifier_verify_session() failed!!!\n");
        maj = GSS_S_FAILURE;
        goto error;
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
    ber_HAMK.bv_len = srp_verifier_get_session_key_length(ntlm_context_handle->ntlm_ver);
    ber_HAMK.bv_val = (void *) bytes_HAMK;
    berror = ber_printf(ber_resp, "t{O}",
                  (int) NTLM_AUTH_SERVER_VALIDATE,
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
    if (ber_ntlm_bytes_M)
    {
        ber_bvfree(ber_ntlm_bytes_M);
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
_ntlm_gss_accept_sec_ctx_error_resp(
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
                  (int) NTLM_AUTH_FAILED,
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

/*ARGSUSED*/
OM_uint32
ntlm_gss_accept_sec_context(
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
    ntlm_gss_cred_id_t ntlm_cred = NULL;
    unsigned char *ptr = NULL;
    int ptr_len = 0;
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    OM_uint32 tmp_maj = 0;
    OM_uint32 tmp_min = 0;
    gss_buffer_desc input_token_ntlm = {0};
    ntlm_gss_ctx_id_t ntlm_context_handle = NULL;
    krb5_error_code krb5_err = 0;
    gss_cred_id_t ntlm_cred_handle = NULL;
    gss_OID_set_desc desired_mech;
    gss_OID_desc mech_ntlm_desc = {NTLM_OID_LENGTH, (void *) NTLM_OID};
    gss_OID mech_ntlm = &mech_ntlm_desc;
    int iv_len = 0;

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
        ntlm_context_handle = (ntlm_gss_ctx_id_t) *context_handle;
    }
    else
    {
        /* First call, allocate context handle */
        ntlm_context_handle =
            (ntlm_gss_ctx_id_t) xmalloc(sizeof(ntlm_gss_ctx_id_rec));
        if (!ntlm_context_handle)
        {
            min = ENOMEM;
            maj = GSS_S_FAILURE;
            goto error;
        }
        memset(ntlm_context_handle, 0, sizeof(ntlm_gss_ctx_id_rec));

#if 1
        /*
         * Hard code desired mech OID to NTLM
         */
        desired_mech.elements = (gss_OID) mech_ntlm;
        desired_mech.count = 1;

        maj = ntlm_gss_acquire_cred(
                  &min,
                  GSS_C_NO_NAME,
                  0,
                  &desired_mech,
                  GSS_C_ACCEPT,
                  &ntlm_cred_handle,
                  NULL,
                  NULL);
        if (maj)
        {
            goto error;
        }
        ntlm_cred = (ntlm_gss_cred_id_t) ntlm_cred_handle;

#else
        if (!verifier_cred_handle || !context_handle)
        {
            maj = GSS_S_FAILURE;
            goto error;
        }
        ntlm_cred = (ntlm_gss_cred_id_t) verifier_cred_handle;
#endif
        ntlm_context_handle->magic_num = NTLM_MAGIC_ID;

        maj = ntlm_gss_duplicate_oid(&min,
                                    ntlm_cred->ntlm_mech_oid,
                                    &ntlm_context_handle->mech);
        if (maj)
        {
            goto error;
        }

        ntlm_context_handle->state = NTLM_AUTH_INIT;
        ntlm_context_handle->cred = (ntlm_gss_cred_id_t) verifier_cred_handle;
        *context_handle = (gss_ctx_id_t) ntlm_context_handle;
    }

    ptr = (unsigned char*) input_token->value;
    ptr_len = (int) input_token->length;
    maj = ntlm_gss_validate_oid_header(
              &min,
              input_token,
              &oid_len);
    if (maj)
    {
        goto error;
    }

    ptr += oid_len;
    ptr_len -= oid_len;
    input_token_ntlm.value = ptr;
    input_token_ntlm.length = ptr_len;

    /* This is the "t" field of ber_scanf() */
    state = NTLM_AUTH_STATE_VALUE(ptr[0]);

    /* Verify state machine is consistent with expected state */
    state = NTLM_AUTH_STATE_VALUE(ptr[0]);

#if 0 /* TBD: FIXME, need spengo to fix this */
    if (state != ntlm_context_handle->state)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
#endif

    switch(state)
    {
      case NTLM_AUTH_INIT:
        ntlm_debug_printf("ntlm_gss_accept_sec_context: state=NTLM_AUTH_INIT\n");
        maj = _ntlm_gss_auth_init(minor_status,
                                 ntlm_context_handle,
                                 state,
                                 &input_token_ntlm,
                                 output_token);
        if (maj)
        {
            goto error;
        }
        ntlm_context_handle->state = NTLM_AUTH_CLIENT_VALIDATE;
        break;

      case NTLM_AUTH_CLIENT_VALIDATE:
        ntlm_debug_printf("ntlm_gss_accept_sec_context: "
                         "state=NTLM_AUTH_CLIENT_VALIDATE\n");
        maj = _ntlm_gss_validate_client(minor_status,
                                       ntlm_context_handle,
                                       state,
                                       &input_token_ntlm,
                                       output_token);
        if (maj != GSS_S_CONTINUE_NEEDED && maj != GSS_S_COMPLETE)
        {
            /* Hard error occurred */
            goto error;
        }

        ntlm_context_handle->state = NTLM_AUTH_COMPLETE;
        if (mech_type)
        {
            tmp_maj = ntlm_gss_duplicate_oid(
                          &tmp_min,
                          (gss_OID) gss_mech_ntlm_oid,
                          mech_type);
            if (tmp_maj)
            {
                maj = tmp_maj;
                *minor_status = tmp_min;
                goto error;
            }
        }

        if (src_name)
        {
            /* Optional: Return UPN name to caller */
            tmp_maj = gss_duplicate_name(
                      &tmp_min,
                      ntlm_context_handle->gss_upn_name,
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
      case NTLM_AUTH_COMPLETE:
        ntlm_debug_printf("ntlm_gss_accept_sec_context: "
                         "state=NTLM_AUTH_COMPLETE\n");
        maj = GSS_S_COMPLETE;
        break;

      default:
        ntlm_debug_printf("ntlm_gss_accept_sec_context: state=UNKNOWN!!!\n");
        maj = GSS_S_FAILURE;
        goto error;
        break;
    }

    if (ntlm_context_handle->state == NTLM_AUTH_COMPLETE)
    {
        krb5_err = ntlm_make_enc_keyblock(ntlm_context_handle);
        if (krb5_err)
        {
            maj = GSS_S_FAILURE;
            min = krb5_err;
            goto error;
        }
        AES_set_encrypt_key(
            ntlm_context_handle->keyblock->contents,
            ntlm_context_handle->keyblock->length * 8,
            &ntlm_context_handle->aes_encrypt_key);
        AES_set_decrypt_key(
            ntlm_context_handle->keyblock->contents,
            ntlm_context_handle->keyblock->length * 8,
            &ntlm_context_handle->aes_decrypt_key);

        iv_len = (AES_BLOCK_SIZE < ntlm_context_handle->ntlm_session_key_len) ?
                     AES_BLOCK_SIZE : ntlm_context_handle->ntlm_session_key_len;
        memset(ntlm_context_handle->aes_encrypt_iv, 0, iv_len);
        memcpy(ntlm_context_handle->aes_encrypt_iv,
               ntlm_context_handle->ntlm_session_key,
               iv_len);

        memset(ntlm_context_handle->aes_decrypt_iv, 0, iv_len);
        memcpy(ntlm_context_handle->aes_decrypt_iv,
               ntlm_context_handle->ntlm_session_key,
               iv_len);
    }

error:
    if (maj != GSS_S_CONTINUE_NEEDED && maj != GSS_S_COMPLETE)
    {
        _ntlm_gss_accept_sec_ctx_error_resp(
            minor_status,
            output_token);
    }

    if (ntlm_cred_handle)
    {
        ntlm_gss_release_cred(&tmp_min, &ntlm_cred_handle);
    }
    return maj;
}
