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
 * Module: srp_accept_sec_ctx.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Implements SRP accept security context
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <krb5.h>
#include "gssapiP_unix.h"
#include "gssapi_unix.h"
#include "gssapi_alloc.h"
#include "unix_util.h"
#include <lber.h>

#include <vmdirdefines.h>
#include "includes.h"
#include "unix_crypt.h"

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
#include "unix_encrypt.h"

static SRP_HashAlgorithm G_alg     = SRP_SHA1;
static SRP_NGType        G_ng_type = SRP_NG_2048;
static const char        *G_n_hex  = 0;
static const char        *G_g_hex  = 0;

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
        len < (int) GSSAPI_UNIX_MECH_OID_LEN_ST)
    {
        maj = GSS_S_CALL_BAD_STRUCTURE;
        goto error;
    }

    if (oid_len != GSSAPI_UNIX_MECH_OID_LEN_ST &&
        memcmp(ptr, GSSAPI_UNIX_MECH_OID_ST, oid_len) != 0)
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


/* Create the temporary SRP secret using username shadow pwd entry */
static int
_srpVerifierInit(
    char *username,
    char *password,
    unsigned char **ret_bytes_s,
    int *ret_len_s,
    unsigned char **ret_bytes_v,
    int *ret_len_v)
{
    int sts = 0;
    const unsigned char *bytes_s = NULL;
    int len_s = 0;
    const unsigned char *bytes_v = NULL;
    int len_v = 0;

    if (!username || !password || !ret_bytes_s || !ret_bytes_v)
    {
        sts = -1;
        goto error;
    }

    srp_create_salted_verification_key(
        G_alg,
        G_ng_type,
        username,
        (const unsigned char *) password,
        (int) strlen(password),
        &bytes_s,
        &len_s,
        &bytes_v,
        &len_v,
        G_n_hex,
        G_g_hex);
    
    srp_print_hex(bytes_s, len_s, 
                  "_srpVerifierInit(accept_sec_context): bytes_s");
    srp_print_hex(bytes_v, len_v, 
                  "_srpVerifierInit(accept_sec_context): bytes_v");

    *ret_bytes_s = (unsigned char *) bytes_s;
    *ret_len_s   = len_s;

    *ret_bytes_v = (unsigned char *) bytes_v;
    *ret_len_v = len_v;

error:
    return 0;
}

static
struct SRPVerifier *
_srpServerNew(
    char *username,
    unsigned char *bytes_s,
    int len_s,
    unsigned char *bytes_v,
    int len_v,
    unsigned char *bytes_A,
    int len_A,
    unsigned char **ret_bytes_B,
    int *ret_len_B)
{
    int sts = 0;
    const unsigned char *bytes_B = NULL;
    int len_B = 0;
    struct SRPVerifier *ver = NULL;

    ver = srp_verifier_new(
              G_alg,
              G_ng_type,
              username,
              bytes_s,
              len_s,
              bytes_v,
              len_v,
              bytes_A,
              len_A,
              &bytes_B,
              &len_B,
              G_n_hex,
              G_g_hex);
    if (!bytes_B)
    {
        /* Verifier SRP-6a safety check violated! */
        sts = -1;
        goto error;
    }

    *ret_bytes_B = (unsigned char *) bytes_B;
    *ret_len_B = len_B;

error:
    if (sts == -1)
    {
        ver = NULL;
    }
    return ver;
}

static
int
_srpServerVerify(
    struct SRPVerifier *ver,
    unsigned char *bytes_M,
    unsigned char **ret_bytes_HAMK)
{
    const unsigned char *bytes_HAMK = NULL;
    int sts = 0;
    srp_verifier_verify_session(ver, bytes_M, &bytes_HAMK);

    if ( !bytes_HAMK )
    {
        sts = -1;
        goto error;
    }

    *ret_bytes_HAMK = (unsigned char *) bytes_HAMK;

error:

    return sts;
}

/*
 * Read SRP_AUTH_INIT token, verify version is compatible. Retrieve
 * user salt value from the /etc/shadow password file, then format
 * a reply token containing the salt value.
 */
static
OM_uint32
_unix_gss_auth_init(
    OM_uint32 *minor_status,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    int sts = 0;
    ber_tag_t ber_state = 0;
    struct berval ber_ctx = {0};
    BerElement *ber = NULL;
    struct berval *ber_username = NULL;
    ber_int_t gss_srp_version_maj = 0;
    ber_int_t gss_srp_version_min = 0;
    BerElement *ber_resp = NULL;
    struct berval *flatten = NULL;
    int berror = 0;
    char *unix_username = NULL;
    char *username_salt = NULL;
    char *username_hash = NULL;

    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_unix_gss_auth_init(): state=SRP_AUTH_INIT\n");

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

    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    if (gss_srp_version_maj != UNIX_MECH_PROTOCOL_MAJ_VERSION ||
        gss_srp_version_min != UNIX_MECH_PROTOCOL_MIN_VERSION)
    {
        /*
         * Deal with protocol/version specific issues here. Currently
         * there is only one version, so error out if this does not match.
         */
        maj = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }
    
    berror = ber_scanf(ber, "O}", &ber_username);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    
    unix_username = calloc(ber_username->bv_len + 1, sizeof(char));
    if (!unix_username)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }
    memcpy(unix_username, ber_username->bv_val, ber_username->bv_len);
    srp_debug_printf("_unix_gss_auth_init(): username=%s\n", unix_username);

    /* Retrieve the salt value from the shadow password file */
    sts = get_sp_salt(unix_username, &username_salt, &username_hash);
    if (sts)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    srp_debug_printf("_unix_gss_auth_init(): salt=%s hash=%s\n",
                     username_salt, username_hash);
    srp_context_handle->username_hash = username_hash;
    srp_context_handle->unix_username = unix_username;
    username_hash = NULL;
    unix_username = NULL;

    ber_resp = ber_alloc_t(LBER_USE_DER);
    if (!ber_resp)
    {
        maj = GSS_S_FAILURE;
        min = ENOMEM;
        goto error;
    }

    /*
     * Response format:
     * tag | UNIX_salt
     */
    berror = ber_printf(ber_resp, "t{o}",
                 SRP_UNIX_SALT_RESPONSE,
                 username_salt,
                 (ber_len_t) strlen(username_salt));
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

    maj = GSS_S_CONTINUE_NEEDED;

error:
    if (maj)
    {
        if (min)
        {
            *minor_status = min;
        }
    }
    if (ber_username)
    {
        ber_bvfree(ber_username);
    }
    if (username_salt)
    {
        free(username_salt);
    }

    ber_bvfree(flatten);
    ber_free(ber, 1);
    ber_free(ber_resp, 1);

    return maj;
}

/*
 * Given the shadow hash value for username, generate a temporary srp
 * "V" verifier value, which is used for authentication with the client
 * for this session. 
 */
static
OM_uint32
_unix_gss_salt_resp(
    OM_uint32 *minor_status,
    srp_gss_ctx_id_t srp_context_handle,
    int state,
    gss_buffer_t input_token,
    gss_buffer_t output_token)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    unsigned char *bytes_s = NULL;
    int len_s = 0;
    unsigned char *bytes_v = NULL;
    int len_v = 0;
    struct berval *flatten = NULL;
    BerElement *ber = NULL;
    BerElement *ber_resp = NULL;
    int berror = 0;
    struct berval ber_ctx = {0};
    struct berval *ber_bytes_A = NULL;
    ber_tag_t ber_state = 0;
    struct SRPVerifier *ver = NULL;
    unsigned char *bytes_B = NULL;
    const unsigned char *srp_session_key = NULL;
    int srp_session_key_len = 0;
    int len_B = 0;
    int sts = 0;

    /*
     * This call creates the temporary server-side SRP secret
     *
     * bytes_s: SRP salt, publically known to client/server
     * bytes_v: SRP secret, privately known only by server
     */
    sts = _srpVerifierInit(
              srp_context_handle->unix_username,
              srp_context_handle->username_hash,
              &bytes_s,
              &len_s,
              &bytes_v,
              &len_v);
    if (sts)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }
    srp_debug_printf("_unix_gss_salt_resp(): salt len=%d", len_s);
    srp_print_hex(bytes_s, len_s, 
                  "_srp_gss_auth_init(accept_sec_context): bytes_s");
    srp_print_hex(bytes_v, len_v, 
                  "_srp_gss_auth_init(accept_sec_context): bytes_v");


    ber_ctx.bv_val = (void *) input_token->value;
    ber_ctx.bv_len = input_token->length;
    ber = ber_init(&ber_ctx);
    if (!ber)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_unix_gss_salt_resp(): state=SRP_UNIX_SALT_RESPONSE\n");

    /*
     * ptr points to ASN.1 encoded data which is dependent on the authentication
     * state. The appropriate decoder format string is applied for each state
     */
    berror = ber_scanf(ber, "t{O}", &ber_state, &ber_bytes_A);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    /*
     * This is mostly impossible, as state IS the "t" field.
     * More a double check for proper decoding.
     */
    if ((int) ber_state != state)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_unix_gss_salt_resp(accept_sec_context): bytes_A");
    srp_print_hex(ber_bytes_A->bv_val,
                  (int) ber_bytes_A->bv_len,
                  "_srp_gss_auth_init(accept_sec_context): bytes_A");

    ver = _srpServerNew(
              srp_context_handle->unix_username,
              bytes_s,
              len_s,
              bytes_v,
              len_v,
              ber_bytes_A->bv_val,
              (int) ber_bytes_A->bv_len,
              &bytes_B,
              &len_B);
    
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
    berror = ber_printf(ber_resp, "t{ooo}",
                 SRP_AUTH_SALT_RESP,
/* TBD: Make this a macro */
                 "SHA-1",
                 (ber_len_t) strlen("SHA-1"),
                 bytes_s,
                 (ber_len_t) len_s,
                 bytes_B,
                 (ber_len_t) len_B);
    if (berror == -1)
    {
        maj = GSS_S_FAILURE;
        goto error;
    }

    srp_debug_printf("_unix_gss_salt_resp(accept_sec_context): bytes_B");
    srp_print_hex(bytes_B,
                  (int) len_B,
                  "_srp_gss_auth_init(accept_sec_context): bytes_B");

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
    srp_context_handle->srp_ver = ver;

    srp_session_key = srp_verifier_get_session_key(
        srp_context_handle->srp_ver,
        &srp_session_key_len);
    if (!srp_session_key || srp_session_key_len == 0)
    {
        min = sts;
        maj = GSS_S_FAILURE;
        goto error;
    }

    if (srp_session_key && srp_session_key_len > 0)
    {
        srp_context_handle->srp_session_key =
            calloc(srp_session_key_len, sizeof(unsigned char));
        if (!srp_context_handle->srp_session_key)
        {
            maj = GSS_S_FAILURE;
            min = ENOMEM;
            goto error;
        }
        memcpy(srp_context_handle->srp_session_key,
               srp_session_key,
               srp_session_key_len);
        srp_context_handle->srp_session_key_len = srp_session_key_len;

        srp_print_hex(srp_session_key, srp_session_key_len,
                      "_srp_gss_auth_init(accept_sec_ctx) got session key");
    }

    maj = GSS_S_CONTINUE_NEEDED;

error:
    if (ber_bytes_A)
    {
        ber_bvfree(ber_bytes_A);
    }
    ber_bvfree(flatten);
    ber_free(ber, 1);
    ber_free(ber_resp, 1);
    if (bytes_v)
    {
        free(bytes_v);
    }
    if (bytes_s)
    {
        free(bytes_s);
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
    unsigned char *bytes_HAMK = NULL;
    struct berval *flatten = NULL;
    int sts = 0;

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

    min = _srpServerVerify(
              srp_context_handle->srp_ver,
              ber_srp_bytes_M->bv_val,
              &bytes_HAMK);
    if (min == -1 || !bytes_HAMK)
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
    /*
     * Generate HAMK response. When min is an error code,
     * an empty HAMK response (zero length) is created.
     */
    if (min == 0)
    {
        sts = srp_verifier_get_session_key_length(
                  srp_context_handle->srp_ver);
        if (sts == 0)
        {
            maj = GSS_S_FAILURE;
            goto error;
        }
        ber_HAMK.bv_len = sts;
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
    gss_buffer_desc name_buf = {0};

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
        maj = _unix_gss_auth_init(minor_status,
                                  srp_context_handle,
                                  state,
                                  &input_token_srp,
                                  output_token);
        if (maj)
        {
            if (maj == GSS_S_CONTINUE_NEEDED)
            {
                srp_context_handle->state = SRP_UNIX_SALT_RESPONSE;
            }
            goto error;
        }
        break;

      case SRP_UNIX_SALT_RESPONSE:
        maj = _unix_gss_salt_resp(minor_status,
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
            name_buf.value = srp_context_handle->unix_username;
            name_buf.length = strlen(name_buf.value);

            tmp_maj = gss_import_name(
                          &min,
                          &name_buf,
                          GSS_C_NT_ANONYMOUS,
                          src_name);
            if (tmp_maj)
            {
                maj = tmp_maj;
                *minor_status = tmp_min;
                goto error;
            }
            srp_context_handle->upn_name =
                strdup(srp_context_handle->unix_username);
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
        krb5_err = srp_make_enc_keyblock(srp_context_handle);
        if (krb5_err)
        {
            maj = GSS_S_FAILURE;
            min = krb5_err;
            goto error;
        }
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
