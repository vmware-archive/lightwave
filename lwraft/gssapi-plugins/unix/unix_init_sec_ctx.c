/*
 * Copyright © 2014 VMware, Inc.  All Rights Reserved.
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
 * Module: srp_init_sec_ctx.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Implements SRP initialize security context
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "unix_mglueP.h"
#include "unix_encrypt.h"
#include "unix_util.h"
#include <krb5.h>
#include "gssapiP_unix.h"
#include "gssapi/gssapi_krb5.h"
#include "gssapi_alloc.h"
#include <lber.h>
#include <csrp/srp.h>
#include <ctype.h>

#ifndef _WIN32
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif
#ifndef __MACH__
#include <crypt.h>
#endif
#endif

static OM_uint32
__srp_ber_flatten_output_token(
    OM_uint32 *minor_status,
    BerElement *ber,
    int ber_len,
    gss_buffer_t asn1_oid,
    gss_buffer_t output_token)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    OM_uint32 output_token_len = 0;
    gss_buffer_desc output_token_mem = {0};
    unsigned char *ptr = NULL;
    int berror = 0;
    struct berval *flatten = NULL;

    berror = ber_flatten(ber, &flatten);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    output_token_len = (OM_uint32) (asn1_oid->length + ber_len);
    output_token_mem.value = gssalloc_malloc(output_token_len);
    if (!output_token_mem.value)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    memset(output_token_mem.value, 0, output_token_len);

    output_token_mem.length = output_token_len;
    ptr = output_token_mem.value;

    memcpy(ptr, asn1_oid->value, asn1_oid->length);
    ptr += asn1_oid->length;

    memcpy(ptr, flatten->bv_val, flatten->bv_len);
    ptr += ber_len;

    /* output_token now owns the memory in output_token_mem */
    *output_token = output_token_mem;
    memset(&output_token_mem, 0, sizeof(output_token_mem));

error:
    if (major)
    {
        *minor_status = minor;
        if (output_token_mem.value)
        {
            gssalloc_free(output_token_mem.value);
        }
    }
    if (flatten)
    {
        ber_bvfree(flatten);
    }
    return major;
}

/*
 * Format request with this data:
 *   major : minor protocol verion
 *   username
 *
 * Expected reply:
 *   UNIX salt value from remote system's /etc/shadow file
 *
 *
 * Errors:
 *   User does not exist
 */
static
OM_uint32
_unix_gss_make_auth_init_output_token(
    OM_uint32 *minor_status,
    gss_OID srp_mech_oid,
    gss_name_t auth_name,
    srp_gss_ctx_id_t srp_context_handle,
    gss_buffer_t output_token)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    BerElement *ber = NULL;
    int ber_len = 0;
    int berror = 0;
    gss_buffer_desc asn1_srp_oid = {0};
    gss_buffer_desc auth_name_buf = {0};
    char *username_str = NULL;
    ber_int_t gss_srp_version_maj = 1;
    ber_int_t gss_srp_version_min = 0;

    ber = ber_alloc_t(LBER_USE_DER);
    if (!ber)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    major = srp_asn1_encode_mech_oid_token(
                &minor,
                srp_mech_oid,
                &asn1_srp_oid);
    if (major)
    {
        goto error;
    }


    major = gss_display_name(&minor, auth_name, &auth_name_buf, NULL);
    if (major)
    {
        goto error;
    }

    username_str = calloc(auth_name_buf.length+1, sizeof(char));
    if (!username_str)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }

    /* This is a '\0' terminated string */
    memcpy(username_str, auth_name_buf.value, auth_name_buf.length);
    srp_context_handle->unix_username = username_str;
    username_str = NULL;

    /*
     * ASN.1 encode the following data:
     * |- GSS_SRP_OID -|-State TAG-|-State Data 1-|-...-|-State Data N-|
     * |- GSS_SRP_OID -|-SRP_INIT(1)-|-VerMaj-|-VerMin-|-username-
     * Note: Use octet string for upn_string; o is octet string, i is length
     *       describing string length to ASN.1 encoder.
     */
    berror = ber_printf(ber, "t{ii",
                  (ber_tag_t) SRP_AUTH_INIT,
                  gss_srp_version_maj,
                  gss_srp_version_min);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    ber_len += berror;

    berror = ber_printf(ber, "o}",
                  (char *) srp_context_handle->unix_username,
                  (ber_len_t) auth_name_buf.length);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    ber_len += berror;

    major = __srp_ber_flatten_output_token(
                &minor,
                ber,
                ber_len,
                &asn1_srp_oid,
                output_token);
    if (major)
    {
        goto error;
    }

error:
    if (major)
    {
        *minor_status = minor;
    }
    if (username_str)
    {
        free(username_str);
    }
    if (asn1_srp_oid.value)
    {
        gss_release_buffer(&minor, &asn1_srp_oid);
    }
    if (auth_name_buf.value)
    {
        gss_release_buffer(&minor, &auth_name_buf);
    }
    ber_free(ber, 1);
    return major;   
}

/*
 * Carol → Steve: I and A = g**a
 */
static
OM_uint32
_srp_gss_make_auth_init_output_token(
    OM_uint32 *minor_status,
    gss_OID srp_mech_oid,
    gss_buffer_t auth_password,
    srp_gss_ctx_id_t srp_context_handle,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    struct crypt_data *cryptbuf_ptr = NULL;
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    gss_buffer_desc asn1_srp_oid = {0};
    gss_buffer_t username = NULL;
    BerElement *ber = NULL;
    BerElement *ber_input = NULL;
    int ber_len = 0;
    int berror = 0;
    char *password = NULL;
    char *user_salt = NULL;
    char *client_sp_hash = NULL;
    struct SRPUser *usr = NULL;
    const char *srp_auth_user = NULL;
    const unsigned char *srp_bytes_A = NULL;
    int srp_bytes_A_len = 0;
    SRP_NGType ng_type = SRP_NG_2048;
    ber_tag_t ber_state = 0;
    struct berval ber_in_tok = {0};
    struct berval *ber_user_salt = NULL;

    ber_in_tok.bv_len = input_token->length;
    ber_in_tok.bv_val = input_token->value;
    ber_input = ber_init(&ber_in_tok);
    if (!ber_input)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    ber = ber_alloc_t(LBER_USE_DER);
    if (!ber)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    berror = ber_scanf(ber_input, "t{O}", &ber_state, &ber_user_salt);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    if (ber_state != SRP_UNIX_SALT_RESPONSE)
    {
        /* This is a serious protocol sequencing error */
        major = GSS_S_FAILURE;
        goto error;
    }

    user_salt = calloc(ber_user_salt->bv_len + 1, sizeof(char));
    if (!user_salt)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    memcpy(user_salt, ber_user_salt->bv_val, ber_user_salt->bv_len);

    major = srp_asn1_encode_mech_oid_token(
                &minor,
                srp_mech_oid,
                &asn1_srp_oid);
    if (major)
    {
        goto error;
    }

    /* The caller constructs this as a '\0' terminated string */
    password = auth_password->value;

    cryptbuf_ptr = calloc(1, sizeof(struct crypt_data));
    if (!cryptbuf_ptr)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("calling crypt_r: pass='%s' salt='%s'\n",
                     "CENSORED", user_salt);
    client_sp_hash = crypt_r(password, user_salt, cryptbuf_ptr);
    if (!client_sp_hash || !*client_sp_hash || strlen(client_sp_hash) < 3)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    srp_debug_printf("crypt_r: computed hash = %s\n", client_sp_hash);

    usr = srp_user_new(SRP_SHA1, ng_type,
                         srp_context_handle->unix_username,
                         (const unsigned char *) client_sp_hash,
                         (int) strlen(client_sp_hash), NULL, NULL);
    if (!usr)
    {
        srp_debug_printf("srp_user_new: failed!\n");
        major = GSS_S_FAILURE;
        return(EXIT_FAILURE);
    }

    /* Used in generating Kerberos keyblock salt value */
    srp_context_handle->upn_name = strdup(srp_context_handle->unix_username);
    if (!srp_context_handle->upn_name)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }

    /* User -> Host: (username, bytes_A) */
    srp_user_start_authentication(usr,
                                  &srp_auth_user,
                                  &srp_bytes_A,
                                  &srp_bytes_A_len);
    if (!srp_auth_user || !srp_bytes_A || srp_bytes_A_len == 0)
    {
        srp_debug_printf("srp_user_start_authentication: failed!\n");
        major = GSS_S_FAILURE;
        return(EXIT_FAILURE);
    }

    srp_print_hex(srp_bytes_A, srp_bytes_A_len,
                  "_srp_gss_make_auth_init_output_token(init_sec_context): bytes_A");
    /*
     * ASN.1 encode the following data:
     * |- GSS_SRP_OID -|-State TAG-|-State Data 1-|-...-|-State Data N-|
     * |- GSS_SRP_OID -|-SRP_INIT(1)-|-VerMaj-|-VerMin-|-UPN(octet string)-|-SRP-bytes_A-|
     * Note: Use octet string for upn_string; o is octet string, i is length
     *       describing string length to ASN.1 encoder.
     */

    berror = ber_printf(ber, "t{o}",
                  (ber_tag_t) SRP_UNIX_SALT_RESPONSE,
                  srp_bytes_A,
                  (ber_len_t) srp_bytes_A_len);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    ber_len += berror;

    major = __srp_ber_flatten_output_token(
                &minor,
                ber,
                ber_len,
                &asn1_srp_oid,
                output_token);
    if (major)
    {
        goto error;
    }

    /* Save the srp_user_new() context in the srp_gss_ctx... handle */
    srp_context_handle->srp_usr = usr;
    usr = NULL;

error:
    if (major)
    {
        *minor_status = minor;
    }
    else
    {
        major = GSS_S_CONTINUE_NEEDED;
    }
    if (username)
    {
        gss_release_buffer(&minor, username);
    }
    if (asn1_srp_oid.value)
    {
        gss_release_buffer(&minor, &asn1_srp_oid);
    }
    if (cryptbuf_ptr)
    {
        free(cryptbuf_ptr);
    }
    if (user_salt)
    {
        free(user_salt);
    }
    if (usr)
    {
        srp_user_delete(usr);
    }

    ber_bvfree(ber_user_salt);
    ber_free(ber, 1);
    ber_free(ber_input, 1);
    return major;
}

static
OM_uint32
_srp_auth_salt_resp(
    OM_uint32 *minor_status,
    gss_OID srp_mech_oid,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    ber_tag_t ber_state = 0;
    struct berval ber_in_tok = {0};
    BerElement *ber_resp = NULL;
    ber_tag_t berror = 0;
    struct berval *ber_mda = NULL;
    struct berval *ber_salt = NULL;
    struct berval *ber_B = NULL;
    const unsigned char *srp_bytes_M = NULL;
    int srp_bytes_M_len = 0;
    int srp_session_key_len = 0;
    gss_buffer_desc asn1_srp_oid = {0};
    BerElement *ber = NULL;
    int ber_len = 0;
    const unsigned char *srp_session_key = NULL;

    ber_in_tok.bv_len = input_token->length;
    ber_in_tok.bv_val = input_token->value;
    ber_resp = ber_init(&ber_in_tok);
    berror = ber_scanf(ber_resp, "t{OOO}",
                 &ber_state, &ber_mda, &ber_salt, &ber_B);
    if (berror == LBER_ERROR)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    srp_print_hex(ber_salt->bv_val, (int) ber_salt->bv_len,
                  "_srp_auth_salt_resp(init_sec_context): salt");
    srp_print_hex(ber_B->bv_val, (int) ber_B->bv_len,
                  "_srp_auth_salt_resp(init_sec_context): bytes_B");

    /* Consistency check, this must match state */
    if ((int) ber_state != state)
    {
        major = GSS_S_FAILURE;
        goto error;
    }
    srp_user_process_challenge(srp_context_handle->srp_usr,
                               ber_salt->bv_val, (int) ber_salt->bv_len,
                               ber_B->bv_val, (int) ber_B->bv_len,
                               &srp_bytes_M, &srp_bytes_M_len);

    srp_session_key = srp_user_get_session_key(
                          srp_context_handle->srp_usr,
                          &srp_session_key_len);
    if (srp_session_key && srp_session_key_len > 0)
    {
        srp_context_handle->srp_session_key =
            calloc(srp_session_key_len, sizeof(unsigned char));
        if (!srp_context_handle->srp_session_key)
        {
            minor = ENOMEM;
            major = GSS_S_FAILURE;
            goto error;
        }
        memcpy(srp_context_handle->srp_session_key,
               srp_session_key,
               srp_session_key_len);
        srp_context_handle->srp_session_key_len = srp_session_key_len;

        srp_print_hex(srp_context_handle->srp_session_key,
                      srp_context_handle->srp_session_key_len,
                      "_srp_auth_salt_resp(init_sec_ctx) got session key");
    }


    ber = ber_alloc_t(LBER_USE_DER);
    if (!ber)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    major = srp_asn1_encode_mech_oid_token(
                &minor,
                srp_mech_oid,
                &asn1_srp_oid);
    if (major)
    {
        goto error;
    }

    /* ASN.1 encode the following data:
     * |- GSS_SRP_OID -|-State TAG-|-State Data 1-|-...-|-State Data N-|
     * |- GSS_SRP_OID -|-SRP_AUTH_CLIENT_VALIDATE(1)-|-SRP-bytes_A-|
     * Note: Use octet string for upn_string; o is octet string, i is length
     *       describing string length to ASN.1 encoder.
     */
    srp_print_hex(srp_bytes_M, srp_bytes_M_len,
                  "_srp_auth_salt_resp(init_sec_ctx) sending bytes_M");

    berror = ber_printf(ber, "t{o}",
                  (ber_tag_t) SRP_AUTH_CLIENT_VALIDATE,
                  srp_bytes_M,
                  (ber_len_t) srp_bytes_M_len);

    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        return(EXIT_FAILURE);
    }
    ber_len = berror;

    major = __srp_ber_flatten_output_token(
                &minor,
                ber,
                ber_len,
                &asn1_srp_oid,
                output_token);
    if (major)
    {
        goto error;
    }

error:
    if (major)
    {
        *minor_status = minor;
    }

    if (ber_mda)
    {
        ber_bvfree(ber_mda);
    }
    if (ber_salt)
    {
        ber_bvfree(ber_salt);
    }
    if (ber_B)
    {
        ber_bvfree(ber_B);
    }
    if (asn1_srp_oid.value)
    {
        gss_release_buffer(&minor, &asn1_srp_oid);
    }
    ber_free(ber_resp, 1);
    ber_free(ber, 1);

    return major;
}


static
OM_uint32
_srp_auth_server_validate(
    OM_uint32 *minor_status,
    gss_OID srp_mech_oid,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    int berror = 0;
    ber_tag_t ber_state = 0;
    BerElement *ber = NULL;
    struct berval *ber_srp_bytes_HAMK = NULL;
    struct berval ber_ctx = {0};

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_srp_auth_server_validate(): "
                     "state=SRP_AUTH_CLIENT_VALIDATE\n");

    /*
     * ASN.1 decode the "HAMK" server mutual auth token
     */
    berror = ber_scanf(ber, "t{O}", &ber_state, &ber_srp_bytes_HAMK);
    if (berror == -1)
    {
        major = GSS_S_FAILURE;
        minor = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }

    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state || ber_srp_bytes_HAMK->bv_len == 0)
    {
        if (ber_srp_bytes_HAMK->bv_len == 0)
        {
            /*
             * Server sent an empty HAMK token, which indicates
             * SRP password authentication failed.
             */
            minor = KRB5KRB_AP_ERR_MUT_FAIL;
        }
        major = GSS_S_FAILURE;
        goto error;
    }

    srp_print_hex(
        ber_srp_bytes_HAMK->bv_val,
        (int) ber_srp_bytes_HAMK->bv_len,
        "_srp_auth_server_validate(accept_sec_ctx) received ber_srp_bytes_HAMK");

    srp_user_verify_session(
        srp_context_handle->srp_usr,
        ber_srp_bytes_HAMK->bv_val);
    if (!srp_user_is_authenticated(srp_context_handle->srp_usr))
    {
        major = GSS_S_FAILURE;
        goto error;
    }


error:

    /* Free a bunch of stuff ... */
    if (ber_srp_bytes_HAMK)
    {
        ber_bvfree(ber_srp_bytes_HAMK);
    }


    ber_free(ber, 1);
    if (major)
    {
        if (minor)
        {
            *minor_status = minor;
        }
    }

    return major;
}


/*
 * Message format for generated output token (state dependent)
 * |- ASN.1 SRP OID -|- state -|- data -|- ... -|
 *
 *
 * SRP_AUTH_INIT: | ASN.1 SRP OID | SRP_AUTH_INIT (byte) | UPN (type GSS_KRB5_NT_PRINCIPAL_NAME) |
 *
 */
OM_uint32
unix_gss_init_sec_context(
    OM_uint32 *minor_status,
    gss_cred_id_t claimant_cred_handle,
    gss_ctx_id_t *context_handle,
    gss_name_t target_name,
    gss_OID mech_type,
    OM_uint32 req_flags,
    OM_uint32 time_req,
    gss_channel_bindings_t input_chan_bindings,
    gss_buffer_t input_token,
    gss_OID *actual_mech,
    gss_buffer_t output_token,
    OM_uint32 *ret_flags,
    OM_uint32 *time_rec)
{
    /*
     * send_token is used to indicate in later steps
     * what type of token, if any should be sent or processed.
     * NO_TOKEN_SEND = no token should be sent
     * INIT_TOKEN_SEND = initial token will be sent
     * CONT_TOKEN_SEND = continuing tokens to be sent
     * CHECK_MIC = no token to be sent, but have a MIC to check.
     */
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    unsigned char *ptr = NULL;
    OM_uint32 state = 0;
    srp_gss_cred_id_t srp_cred = NULL;
    srp_gss_ctx_id_t srp_context_handle = NULL;
    gss_buffer_desc output_token_mem = {0};
    krb5_error_code krb5_err = 0;
    gss_OID srp_mech_oid = {0};

    dsyslog("Entering init_sec_context\n");

    if (!claimant_cred_handle || !context_handle)
    {
        major = GSS_S_FAILURE;
        goto error;
    }

    srp_cred = (srp_gss_cred_id_t) claimant_cred_handle;
    if (!srp_cred || !srp_cred->password || !srp_cred->srp_mech_oid)
    {
        major = GSS_S_UNAVAILABLE;
        goto error;
    }
    srp_mech_oid = srp_cred->srp_mech_oid;

    /* First call to init_sec_context; allocate new context */
    if (*context_handle == GSS_C_NO_CONTEXT)
    {
        state = SRP_AUTH_INIT;
        srp_debug_printf("unix_gss_init_sec_context: state=SRP_AUTH_INIT\n");
        srp_context_handle =
            (srp_gss_ctx_id_t) calloc(1, sizeof(srp_gss_ctx_id_rec));
        if (!srp_context_handle)
        {
            minor = ENOMEM;
            major = GSS_S_FAILURE;
            goto error;
        }
        memset(srp_context_handle, 0, sizeof(srp_gss_ctx_id_rec));

        srp_context_handle->magic_num = SRP_MAGIC_ID;
        srp_context_handle->state     = state;
        srp_context_handle->cred      = srp_cred;

        /* Needed for Kerberos AES256-SHA1 keyblock generation */
        krb5_err = krb5_init_context(&srp_context_handle->krb5_ctx);
        if (krb5_err)
        {
            major = GSS_S_FAILURE;
            minor = krb5_err;
            goto error;
        }

        major = _unix_gss_make_auth_init_output_token(
                    &minor,
                    srp_mech_oid,
                    srp_cred->name,
                    srp_context_handle,
                    &output_token_mem);

        if (major)
        {
            goto error;
        }
        srp_context_handle->state = SRP_UNIX_SALT_RESPONSE;
        *context_handle = (gss_ctx_id_t) srp_context_handle;
        srp_context_handle = NULL;
        major = GSS_S_CONTINUE_NEEDED;
    }
    else
    {
        srp_context_handle = (srp_gss_ctx_id_t) *context_handle;
        if (!input_token)
        {
            major = GSS_S_FAILURE;
            goto error;
        }
        ptr = input_token->value;

        /* Verify state machine is consistent with expected state */
        state = SRP_AUTH_STATE_VALUE(ptr[0]);
        if (state != srp_context_handle->state)
        {
            major = GSS_S_FAILURE;
            goto error;
        }

        srp_context_handle->state = state;
        switch (srp_context_handle->state)
        {
          case SRP_UNIX_SALT_RESPONSE:
            major = _srp_gss_make_auth_init_output_token(
                        &minor,
                        srp_mech_oid,
                        srp_cred->password,
                        srp_context_handle,
                        input_token,
                        &output_token_mem);
            if (major != GSS_S_CONTINUE_NEEDED)
            {
                goto error;
            }

            srp_context_handle->state = SRP_AUTH_SALT_RESP;
            break;

          case SRP_AUTH_SALT_RESP:
            srp_debug_printf("unix_gss_init_sec_context: "
                             "state=SRP_AUTH_SALT_RESP\n");
            major = _srp_auth_salt_resp(
                         &minor,
                         srp_mech_oid,
                         srp_context_handle,
                         srp_context_handle->state,
                         input_token,
                         &output_token_mem);
            if (major)
            {
                goto error;
            }

            srp_context_handle->state = SRP_AUTH_SERVER_VALIDATE;
            major = GSS_S_CONTINUE_NEEDED;
            break;

          case SRP_AUTH_SERVER_VALIDATE:
            srp_debug_printf("unix_gss_init_sec_context: "
                             "state=SRP_AUTH_SERVER_VALIDATE\n");
            major = _srp_auth_server_validate(
                         &minor,
                         srp_mech_oid,
                         srp_context_handle,
                         srp_context_handle->state,
                         input_token,
                         &output_token_mem);
            if (major)
            {
                srp_debug_printf("unix_gss_init_sec_context: "
                                 "state=SRP_AUTH_FAILED!!!\n");
                srp_context_handle->state = SRP_AUTH_FAILED;
                major = GSS_S_FAILURE;
            }
            else
            {
                srp_debug_printf("unix_gss_init_sec_context: "
                                 "state=SRP_AUTH_COMPLETE!!!\n");
                srp_context_handle->state = SRP_AUTH_COMPLETE;
                memset(&output_token_mem, 0, sizeof(output_token_mem));
                major = GSS_S_COMPLETE;
            }
            break;

          case SRP_AUTH_COMPLETE:
            major = GSS_S_COMPLETE;
          break;

          case SRP_AUTH_FAILED:
            srp_debug_printf("unix_gss_init_sec_context: "
                             "state=SRP_AUTH_FAILED!!!\n");
            major = GSS_S_FAILURE;
          break;

          default:
            srp_debug_printf("unix_gss_init_sec_context: "
                             "state=UNKNOWN!!!\n");
            major = GSS_S_FAILURE;
            break;
        }
    }

    *output_token = output_token_mem;

    if (major == GSS_S_COMPLETE)
    {
        krb5_err = srp_make_enc_keyblock(srp_context_handle);
        if (krb5_err)
        {
            major = GSS_S_FAILURE;
            minor = krb5_err;
            goto error;
        }
        if (actual_mech)
        {
            *actual_mech = srp_mech_oid;
        }
    }
    else if (major == GSS_S_CONTINUE_NEEDED && actual_mech)
    {
        *actual_mech = srp_mech_oid;
    }

error:

    /* Free a bunch of stuff ... */
    if (major)
    {
        if (minor)
        {
            *minor_status = minor;
        }
    }

    return major;
} /* init_sec_context */
