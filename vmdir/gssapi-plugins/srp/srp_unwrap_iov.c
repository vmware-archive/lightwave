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
#include "srp_util.h"
#include "srp_encrypt.h"
#include "srp_encrypt.h"
#include "gssapi_alloc.h"




#ifndef _SRP_USE_TRIVIAL_ENCRYPTION

OM_uint32
srp_gss_unwrap_iov(OM_uint32 *minor_status,
		      gss_ctx_id_t context_handle,
		      int *conf_state,
		      gss_qop_t *qop_state,
		      gss_iov_buffer_desc *iov,
		      int iov_count)
{
    OM_uint32 maj = 0;
    OM_uint32 min = 0;
    srp_gss_ctx_id_t srp_context_handle = (srp_gss_ctx_id_t) context_handle;
    int ciphertext_len = 0;
    unsigned char *plaintext = NULL;
    int plaintext_len = 0;
    int sealed = 1;

    ciphertext_len = (int) AES256PAD(iov[1].buffer.length);

    plaintext_len = ciphertext_len;
    plaintext = calloc(plaintext_len, sizeof(unsigned char));
    if (!plaintext)
    {
        min = ENOMEM;
        goto error;
    }

    maj = srp_decrypt_aes256_hmac_sha1(
              srp_context_handle,
              ((unsigned char *) iov[0].buffer.value)  + SRP_MECH_OID_OFFSET,
              (int) (iov[0].buffer.length - SRP_MECH_OID_OFFSET),
              iov[1].buffer.value,
              (int) iov[1].buffer.length,
              plaintext);
    if (maj)
    {
        min = maj;
        goto error;
    }
    memcpy(iov[1].buffer.value, plaintext, plaintext_len);

    /*
     * TBD: Decode iov[0] to determine if encrypted/cksummed;
     * assume always encrypted.
     */
    *conf_state = sealed;

error:
    if (plaintext)
    {
        free(plaintext);
    }
    return min ? min : maj;
}

#else

OM_uint32
srp_gss_unwrap_iov(OM_uint32 *minor_status,
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
