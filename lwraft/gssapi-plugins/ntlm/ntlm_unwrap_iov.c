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



#include <openssl/aes.h>
#include <errno.h>
#include <string.h>
#include "ntlm_util.h"
#include "ntlm_encrypt.h"
#include "ntlm_encrypt.h"

#ifndef _NTLM_USE_TRIVIAL_ENCRYPTION

OM_uint32
ntlm_gss_unwrap_iov(OM_uint32 *minor_status,
		      gss_ctx_id_t context_handle,
		      int *conf_state,
		      gss_qop_t *qop_state,
		      gss_iov_buffer_desc *iov,
		      int iov_count)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    ntlm_gss_ctx_id_t ntlm_context_handle = (ntlm_gss_ctx_id_t) context_handle;
    unsigned char *in_encbuf = NULL;
    unsigned char *out_encbuf = NULL;
    int in_encbuf_len = 0;
    int out_encbuf_len = 0;
    int sealed = 1;

    in_encbuf_len = (int) AES256PAD(iov[1].buffer.length);
    if (in_encbuf_len != iov[1].buffer.length)
    {
        /* This may not work if the input buffer size isn't already aligned */
        in_encbuf = calloc(in_encbuf_len, sizeof(unsigned char));
        if (!in_encbuf)
        {
            min = ENOMEM;
            goto error;
        }
        memcpy(in_encbuf, iov[1].buffer.value, iov[1].buffer.length);
    }
    else
    {
        in_encbuf = iov[1].buffer.value;
    }

    out_encbuf_len = in_encbuf_len;
    out_encbuf = calloc(out_encbuf_len, sizeof(unsigned char));
    if (!out_encbuf)
    {
        min = ENOMEM;
        goto error;
    }

    AES_cbc_encrypt(in_encbuf,
                    out_encbuf,
                    in_encbuf_len,
                    &ntlm_context_handle->aes_decrypt_key,
                    ntlm_context_handle->aes_decrypt_iv,
                    AES_DECRYPT);
    memcpy(iov[1].buffer.value, out_encbuf, out_encbuf_len);
    /*
     * TBD: Decode iov[0] to determine if encrypted/cksummed;
     * assume always encrypted.
     */
    *conf_state = sealed;

error:
    if (in_encbuf && in_encbuf != iov[1].buffer.value)
    {
        free(in_encbuf);
    }
    if (out_encbuf)
    {
        free(out_encbuf);
    }
    return min ? min : maj;
}

#else

OM_uint32
ntlm_gss_unwrap_iov(OM_uint32 *minor_status,
		      gss_ctx_id_t context_handle,
		      int *conf_state,
		      gss_qop_t *qop_state,
		      gss_iov_buffer_desc *iov,
		      int iov_count)
{
    unsigned char *key = NULL;
    int keylen = 0;
    int sealed = 0;

    /* TBD:Adam-How to determine the protection level? */
    /* rpc_c_authn_level_pkt_privacy */
    sealed = 1;

    key = xor_get_encrypt_key(&keylen);
    xor_encrypt(iov[1].buffer.value,
                iov[1].buffer.length,
                key,
                keylen);
    /*
     * Decode iov[0] to determine if encrypted/cksummed;
     * assume always encrypted.
     */
    *conf_state = sealed;

    /* Nothing can fail in this implementation :) */
    return 0;
}

#endif
