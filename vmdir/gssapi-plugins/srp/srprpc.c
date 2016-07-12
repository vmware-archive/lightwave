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
#include <srp_verifier_h.h>
#include "srprpc.h"

#ifdef _WIN32
#define sleep(x) Sleep((x) * 1000)
#endif


/* Duplicate container memory, as caller can't free this */
static long _cli_rpc_container_data_duplicate(
    rpc_p_srp_bytes_container cont,
    unsigned char **data,
    int *data_len)
{
    long sts = 0;
    unsigned char *ret_data = NULL;
    int ret_data_len = 0;

    ret_data_len = cont->len_B;
    ret_data = calloc(ret_data_len, sizeof(unsigned char));
    if (!ret_data)
    {
        sts = rpc_s_no_memory;
        goto error;
    }
    memcpy(ret_data, cont->bytes_B, ret_data_len);
    *data = ret_data;
    *data_len = ret_data_len;

error:
    if (sts)
    {
        if (ret_data)
        {
            free(ret_data);
        }
    }
    return sts;
}

static void _cli_rpc_free_container(
    rpc_p_srp_bytes_container cont)
{
    idl_ulong_int sts = 0;

    if (cont)
    {
        if (cont->bytes_B)
        {
            rpc_sm_client_free(cont->bytes_B, &sts);
        }
        rpc_sm_client_free(cont, &sts);
    }
}

long cli_rpc_srp_verifier_new(
    handle_t hServer,
    long alg,
    long ng_type,
    char *username,
    const unsigned char *bytes_A, int len_A,
    const unsigned char **bytes_B, int *len_B,
    const unsigned char **bytes_s, int *len_s,
    const unsigned char **MDA_value, int *MDA_value_len,
    char *n_hex,
    char *g_hex,
    srp_verifier_handle_t *hSrp)
{
    long sts = 0;
    rpc_srp_bytes_container bytes_cont_A = {0};
    rpc_p_srp_bytes_container bytes_cont_B = NULL;
    rpc_p_srp_bytes_container bytes_cont_s = NULL;
    rpc_p_srp_bytes_container MDA_cont = NULL;
    srp_verifier_handle_t hRetSrp = NULL;
    unsigned char *ret_bytes_B = NULL;
    unsigned char *ret_bytes_s = NULL;
    unsigned char *ret_MDA_value = NULL;
    int ret_len_B = 0;
    int ret_len_s = 0;
    int ret_MDA_value_len = 0;
    int rpc_retry = 0;

    bytes_cont_A.len_B = len_A;
    bytes_cont_A.bytes_B = (unsigned char *) bytes_A;

    /*
     * Reference: BUG 1315106
     * Work-around for failure seen in some W2k12 systems. The failure mode is
     * vmdir Srv_rpc_srp_verifier_new() RPC is called, succeeds, but
     * the returned RPC fails with an error status rpc_s_connection_closed.
     * This happens only once, and only in some W2K12 deployed environments.
     */
    do {
        DO_RPC(rpc_srp_verifier_new(
                  hServer,
                  alg,
                  ng_type,
                  username,
                  &bytes_cont_A, /* in */
                  &bytes_cont_B, /* out */
                  &bytes_cont_s, /* out */
                  &MDA_cont,
                  n_hex,
                  g_hex,
                  &hRetSrp), sts);
        if (sts == rpc_s_connection_closed)
        {
            sleep(1);
            rpc_retry++;
        }
    } while (sts == rpc_s_connection_closed && rpc_retry < 5);
    if (sts)
    {
        goto error;
    }

    sts = _cli_rpc_container_data_duplicate(bytes_cont_B,
                                            &ret_bytes_B,
                                            &ret_len_B);
    if (sts)
    {
        goto error;
    }
    sts = _cli_rpc_container_data_duplicate(bytes_cont_s,
                                            &ret_bytes_s,
                                            &ret_len_s);
    if (sts)
    {
        goto error;
    }

    sts = _cli_rpc_container_data_duplicate(MDA_cont,
                                            &ret_MDA_value,
                                            &ret_MDA_value_len);
    if (sts)
    {
        goto error;
    }

    *bytes_B = ret_bytes_B;
    *len_B = ret_len_B;
    *bytes_s = ret_bytes_s;
    *len_s = ret_len_s;
    *MDA_value = ret_MDA_value;
    *MDA_value_len = ret_MDA_value_len;
    *hSrp = hRetSrp;

error:
    if (sts)
    {
        if (ret_bytes_B)
        {
            free(ret_bytes_B);
        }
        if (ret_bytes_s)
        {
            free(ret_bytes_s);
        }
    }
    _cli_rpc_free_container(bytes_cont_B);
    _cli_rpc_free_container(bytes_cont_s);
    _cli_rpc_free_container(MDA_cont);
    return sts;
}

long cli_rpc_srp_verifier_get_session_key(
    handle_t hServer,
    srp_verifier_handle_t hSrp,
    const unsigned char **key,
    int *key_len)
{
    long sts = 0;
    rpc_p_srp_bytes_container key_cont = NULL;
    unsigned char *ret_key = NULL;
    int ret_key_len = 0;

    DO_RPC(rpc_srp_verifier_get_session_key(
              hServer,
              hSrp,
              &key_cont), sts);
    if (sts)
    {
        goto error;
    }

    sts = _cli_rpc_container_data_duplicate(key_cont, &ret_key, &ret_key_len);
    if (sts)
    {
        goto error;
    }

    *key = (const unsigned char *) ret_key;
    *key_len = ret_key_len;

error:
    if (sts)
    {
        if (ret_key)
        {
            free(ret_key);
        }
    }
    _cli_rpc_free_container(key_cont);
    return sts;
}

long cli_rpc_srp_verifier_get_session_key_length(
    handle_t hServer,
    srp_verifier_handle_t hSrp,
    long *ret_key_length)
{
    long sts = 0;
    idl_long_int key_length = 0;

    DO_RPC(rpc_srp_verifier_get_session_key_length(
              hServer,
              hSrp,
              &key_length), sts);
    if (sts)
    {
        goto error;
    }
    *ret_key_length = key_length;

error:
    return sts;
}

long cli_rpc_srp_verifier_verify_session(
        handle_t hServer,
        srp_verifier_handle_t hSrp,
        const unsigned char *user_M, int user_M_len,
        const unsigned char **bytes_HAMK, int *bytes_HAMK_len)
{
    long sts = 0;
    rpc_srp_bytes_container user_M_cont = {0};
    rpc_p_srp_bytes_container bytes_HAMK_cont = NULL;
    unsigned char *ret_bytes_HAMK = NULL;
    int ret_bytes_HAMK_len = 0;

    user_M_cont.len_B = user_M_len;
    user_M_cont.bytes_B = (unsigned char *) user_M;

    DO_RPC(rpc_srp_verifier_verify_session(
              hServer,
              hSrp,
              &user_M_cont,
              &bytes_HAMK_cont), sts);
    if (sts)
    {
        goto error;
    }
    if (!bytes_HAMK_cont || !bytes_HAMK_cont->bytes_B)
    {
        sts = rpc_s_no_memory;
        goto error;
    }

    sts = _cli_rpc_container_data_duplicate(bytes_HAMK_cont,
                                            &ret_bytes_HAMK,
                                            &ret_bytes_HAMK_len);
    if (sts)
    {
        goto error;
    }
    *bytes_HAMK = ret_bytes_HAMK;
    *bytes_HAMK_len = ret_bytes_HAMK_len;

error:
    if (sts)
    {
        if (ret_bytes_HAMK)
        {
            free(ret_bytes_HAMK);
        }
    }
    _cli_rpc_free_container(bytes_HAMK_cont);
    return sts;
}

long cli_rpc_srp_verifier_delete(
        handle_t hServer,
        srp_verifier_handle_t *phSrp)
{
    long sts = 0;

    if (hServer && phSrp)
    {
        DO_RPC(rpc_srp_verifier_delete(
                  hServer,
                  phSrp), sts);
    }
    return sts;
}
