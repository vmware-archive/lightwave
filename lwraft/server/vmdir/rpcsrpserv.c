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
#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#include <csrp/srp.h>
#include <srp_verifier_h.h>

#ifdef IDL_PROTOTYPES
#define long idl_long_int
#define char idl_char
#endif

#ifdef _WIN32
/* proper use of memcpy is safe; banned.h "hit list" */
#pragma warning(disable : 4995)
#endif

static void _srp_bytes_container_free(
    rpc_p_srp_bytes_container cont);

typedef struct _srp_secret_blob_data
{
    char *blob;
    int blob_len;

    /* All pointers below here are aliases into "blob" */
    char *mda;
    int mda_len;
    char *v;
    int v_len;
    char *salt;
    int salt_len;
} srp_secret_blob_data, *srp_p_secret_blob_data;


static long _get_srp_secret_decoded(
    char *username,
    srp_p_secret_blob_data srp_data)
{
    long sts = 0;
    char *srp_secret = NULL;
    char *srp_secret_str = NULL;
    unsigned int srp_secret_str_len = 0;
    unsigned int srp_secret_len_max = 0;
    unsigned int srp_secret_len = 0;
    uint32_t srp_decode_buf_len = 0;
    uint16_t srp_decode_mda_len = 0;
    uint16_t srp_decode_v_len = 0;
    uint8_t srp_decode_salt_len = 0;
    char *srp_decode_ptr = NULL;
    char *srp_mda = NULL;
    char *srp_v = NULL;
    char *srp_salt = NULL;

    /*
     * This is the implementation of the RPC VmDirGetSRPSecret.
     * The public interface VmDirGetSRPSecret() is no longer needed
     * by this implementation
     */
    sts = VmDirSRPGetIdentityData(
              username,
              &srp_secret_str,
              &srp_secret_str_len);
    if (sts)
    {
        goto error;
    }

    srp_secret = calloc(srp_secret_str_len, sizeof(char));
    if (!srp_secret)
    {
        sts = rpc_s_no_memory;
        goto error;
    }
    srp_secret_len_max = srp_secret_str_len;
    sts = sasl_decode64(srp_secret_str,
                        srp_secret_str_len,
                        srp_secret,
                        srp_secret_len_max,
                        &srp_secret_len);
    if (sts != SASL_OK)
    {
        sts = rpc_s_coding_error;
        goto error;
    }

    /*
     * Encoding of data blob (from common/srp.c):
     * calculate buffer size
     * mda: Message Digest Algorithm
     * v: SRP private "hash" value * salt: random salt generated at "hash" creation time
     *
     * 0. 4 byte length
     * 1. utf8(mda) : 2 bytes + string
     * 2. mpi(v)    : 2 bytes + verifier
     * 3. os(salt)  : 1 bytes + salt
     */
    srp_decode_ptr = srp_secret;
    memcpy(&srp_decode_buf_len, srp_decode_ptr, sizeof(uint32_t));
    srp_decode_ptr += sizeof(uint32_t);
    srp_decode_buf_len = ntohl(srp_decode_buf_len);

    memcpy(&srp_decode_mda_len, srp_decode_ptr, sizeof(uint16_t));
    srp_decode_ptr += sizeof(uint16_t);
    srp_decode_mda_len = ntohs(srp_decode_mda_len);
    srp_mda = srp_decode_ptr;
    srp_decode_ptr += srp_decode_mda_len;

    memcpy(&srp_decode_v_len, srp_decode_ptr, sizeof(uint16_t));
    srp_decode_ptr += sizeof(uint16_t);
    srp_decode_v_len = ntohs(srp_decode_v_len);
    srp_v = srp_decode_ptr;
    srp_decode_ptr += srp_decode_v_len;

    memcpy(&srp_decode_salt_len, srp_decode_ptr, sizeof(uint8_t));
    srp_decode_ptr += sizeof(uint8_t);
    srp_salt = srp_decode_ptr;


    /* blob is the buffer, the rest are aliased pointers */
    srp_data->blob = srp_secret;
    srp_data->blob_len = srp_decode_buf_len;
    srp_data->mda = srp_mda;
    srp_data->mda_len = srp_decode_mda_len;
    srp_data->v = srp_v;
    srp_data->v_len = srp_decode_v_len;
    srp_data->salt = srp_salt;
    srp_data->salt_len = srp_decode_salt_len;

error:
    if (sts)
    {
        if (srp_secret)
        {
            free(srp_secret);
        }
    }
    if (srp_secret_str)
    {
        free(srp_secret_str);
    }

    return sts;
}


static void _free_srp_secret_decoded(
    srp_p_secret_blob_data srp_data)
{
    if (srp_data && srp_data->blob)
    {
        free(srp_data->blob);
    }
}

static long _srp_bytes_container_allocate(
    const char *data,
    long data_len,
    rpc_p_srp_bytes_container *ret_cont)
{
    long sts = 0;
    rpc_p_srp_bytes_container cont = NULL;

    cont = rpc_ss_allocate(sizeof(rpc_srp_bytes_container));
    if (!cont)
    {
        sts = rpc_s_no_memory;
        goto error;
    }

    cont->bytes_B = rpc_ss_allocate(data_len * sizeof(char));
    if (!cont->bytes_B)
    {
        sts = rpc_s_no_memory;
        goto error;
    }
    if (data && data_len > 0)
    {
        memcpy(cont->bytes_B, data, data_len);
        cont->len_B = data_len;
    }
    else
    {
        cont->len_B = 0;
    }

    *ret_cont = cont;

error:
    if (sts)
    {
        _srp_bytes_container_free(cont);
    }
    return sts;
}

static void _srp_bytes_container_free(
    rpc_p_srp_bytes_container cont)
{
    if (cont)
    {
        if (cont->bytes_B)
        {
            rpc_ss_free(cont->bytes_B);
        }
        rpc_ss_free(cont);
    }
}

long Srv_rpc_srp_verifier_new(
     handle_t hServer,
     long alg,
     long ng_type,
     char *username,
     rpc_p_srp_bytes_container bytes_A,
     rpc_p_srp_bytes_container *ret_B_value,
     rpc_p_srp_bytes_container *ret_bytes_s,
     rpc_p_srp_bytes_container *ret_MDA_value,
     char *n_hex,
     char *g_hex,
     srp_verifier_handle_t *hSrp)
{
    long sts = 0;
    srp_secret_blob_data srp_data = {0};
    const char *srp_bytes_B = NULL;
    int srp_bytes_B_len = 0;
    struct SRPVerifier *ver = NULL;
    rpc_p_srp_bytes_container B_value = NULL;
    rpc_p_srp_bytes_container bytes_s = NULL;
    rpc_p_srp_bytes_container MDA_value = NULL;

    /* SRP "V" secret comes from VmDirSRPGetIdentityData()  local call */
    sts = _get_srp_secret_decoded(
              username,
              &srp_data);
    if (sts)
    {
        goto error;
    }

    /*
     * Create response token. This contains (s, B) for I
     */
    ver = srp_verifier_new(alg,
                           ng_type,
                           username,
                           srp_data.salt,
                           (int) srp_data.salt_len,
                           srp_data.v,
                           (int) srp_data.v_len,
                           bytes_A->bytes_B,
                           (int) bytes_A->len_B,
                           &srp_bytes_B,
                           &srp_bytes_B_len,
                           NULL,  /* n_hex, */
                           NULL); /* g_hex */
    if (!ver || !srp_bytes_B)
    {
        sts = rpc_s_no_memory;
        goto error;
    }

    sts = _srp_bytes_container_allocate(srp_bytes_B,
                                        srp_bytes_B_len,
                                        &B_value);
    if (sts)
    {
        goto error;
    }

    sts = _srp_bytes_container_allocate(srp_data.salt,
                                        srp_data.salt_len,
                                        &bytes_s);
    if (sts)
    {
        goto error;
    }

    sts = _srp_bytes_container_allocate(srp_data.mda,
                                        srp_data.mda_len,
                                        &MDA_value);
    if (sts)
    {
        goto error;
    }

    *ret_B_value = B_value;
    *ret_bytes_s = bytes_s;
    *ret_MDA_value = MDA_value;
    *hSrp = (srp_verifier_handle_t) ver;

error:
    if (sts)
    {
        _srp_bytes_container_free(B_value);
        _srp_bytes_container_free(bytes_s);
        _srp_bytes_container_free(MDA_value);
    }
    _free_srp_secret_decoded(&srp_data);
    return sts;
}

long Srv_rpc_srp_verifier_get_session_key(
     handle_t hServer,
     srp_verifier_handle_t hSrp,
     rpc_p_srp_bytes_container *ret_key_cont)
{
    long sts = 0;
    const char *key = NULL;
    int key_len = 0;
    struct SRPVerifier *ver = (struct SRPVerifier *) hSrp;
    rpc_p_srp_bytes_container key_cont = NULL;

    if (!ver)
    {
        sts = rpc_s_invalid_handle;
        goto error;
    }
    key = srp_verifier_get_session_key(ver, &key_len);
    if (!key || key_len == 0)
    {
        sts = rpc_s_auth_nokey;
        goto error;
    }

    sts = _srp_bytes_container_allocate(key, key_len, &key_cont);
    if (sts)
    {
        goto error;
    }

    *ret_key_cont = key_cont;
error:
    if (sts)
    {
        _srp_bytes_container_free(key_cont);
    }
    return sts;
}

long Srv_rpc_srp_verifier_get_session_key_length(
     handle_t hServer,
     srp_verifier_handle_t hSrp,
     long *ret_key_length)
{
    long sts = 0;
    int key_length = 0;
    struct SRPVerifier *ver = (struct SRPVerifier *) hSrp;

    if (!ver)
    {
        sts = rpc_s_invalid_handle;
        goto error;
    }
    key_length = srp_verifier_get_session_key_length(ver);
    *ret_key_length = key_length;

error:
    return sts;
}

long Srv_rpc_srp_verifier_verify_session(
     handle_t hServer,
     srp_verifier_handle_t hSrp,
     rpc_p_srp_bytes_container user_M_cont,
     rpc_p_srp_bytes_container *ret_bytes_HAMK_cont)
{
    long sts = 0;
    struct SRPVerifier *ver = (struct SRPVerifier *) hSrp;
    char *bytes_M = NULL;
    const char *bytes_HAMK = NULL;
    rpc_p_srp_bytes_container bytes_HAMK_cont = NULL;
    long bytes_HAMK_len = 0;

    if (!ver)
    {
        sts = rpc_s_invalid_handle;
        goto error;
    }

    /* Oddly enough, the length for user_M isn't used here. */
    bytes_M = user_M_cont->bytes_B;
    srp_verifier_verify_session(ver,
                                bytes_M,
                                &bytes_HAMK);
    if (!bytes_HAMK)
    {
        *ret_bytes_HAMK_cont = NULL;
        sts = rpc_s_auth_mut_fail;
        goto error;
    }
    bytes_HAMK_len = srp_verifier_get_session_key_length(ver);

    sts = _srp_bytes_container_allocate(bytes_HAMK,
                                        bytes_HAMK_len,
                                        &bytes_HAMK_cont);
    if (sts)
    {
        goto error;
    }
    *ret_bytes_HAMK_cont = bytes_HAMK_cont;

error:
    if (sts)
    {
        _srp_bytes_container_free(bytes_HAMK_cont);
    }
    return sts;
}

long Srv_rpc_srp_verifier_delete(
     handle_t hServer,
     srp_verifier_handle_t *hSrp)
{
    long sts = 0;
    struct SRPVerifier **ver = (struct SRPVerifier **) hSrp;

    if (!ver || !*ver)
    {
        sts = rpc_s_invalid_handle;
        goto error;
    }
    srp_verifier_delete(*ver);
    *hSrp = NULL;

error:
    return sts;
}


/*
 * Same as Srv_rpc_srp_verifier_delete(), but called when connection
 * exception occurs, like client dies, or connection times out.
 */
void srp_verifier_handle_t_rundown(void *ctx)
{
    struct SRPVerifier *ver = (struct SRPVerifier *) ctx;
    if (ver)
    {
        srp_verifier_delete(ver);
    }
}
