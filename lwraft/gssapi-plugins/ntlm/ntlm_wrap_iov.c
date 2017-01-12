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
#include <openssl/rand.h>

#include <string.h>
#include <errno.h>
#include "ntlm_util.h"
#include "ntlm_encrypt.h"

#ifndef _NTLM_USE_TRIVIAL_ENCRYPTION

OM_uint32
ntlm_gss_wrap_iov(OM_uint32 *minor_status,
		    gss_ctx_id_t context_handle,
		    int conf_req_flag,
		    gss_qop_t qop_req,
		    int *conf_state,
		    gss_iov_buffer_desc *iov,
		    int iov_count)
{
    OM_uint32 ret = 0;
    OM_uint32 min = 0;
    ntlm_gss_ctx_id_t ntlm_context_handle = (ntlm_gss_ctx_id_t) context_handle;
    gss_buffer_desc asn1_mech_oid = {0};
    unsigned char *iov0 = NULL;
    int iov0_len = 0;
    unsigned char *in_encbuf = NULL;
    unsigned char *out_encbuf = NULL;
    int in_encbuf_len = 0;
    int out_encbuf_len = 0;

    ret = ntlm_asn1_encode_mech_oid_token(
              &min,
              (gss_OID) gss_mech_ntlm_oid,
              &asn1_mech_oid);
    if (ret)
    {
        goto error;
    }

    /* Fixup iov[0] to have proper GSS/OID header */
    iov0 = asn1_mech_oid.value;
    iov0_len = (int) asn1_mech_oid.length;
    if (iov[0].buffer.value)
    {
        /* Not sure this is safe to do */
        free(iov[0].buffer.value);
    }
    iov[0].buffer.value = iov0;
    iov[0].buffer.length = iov0_len;
    iov[0].type |= GSS_IOV_BUFFER_FLAG_ALLOCATED;

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

    AES_cbc_encrypt(in_encbuf,
                    out_encbuf,
                    in_encbuf_len,
                    &ntlm_context_handle->aes_encrypt_key,
                    ntlm_context_handle->aes_encrypt_iv,
                    AES_ENCRYPT);
    memcpy(iov[1].buffer.value, out_encbuf, out_encbuf_len);

    /* TBD: Adam- Don't know the proper return value for this argument */
    *conf_state = conf_req_flag;

error:
    if (in_encbuf && in_encbuf != iov[1].buffer.value)
    {
        free(in_encbuf);
    }
    if (out_encbuf)
    {
        free(out_encbuf);
    }
    return min ? min : ret;
}

#else

OM_uint32
ntlm_gss_wrap_iov(OM_uint32 *minor_status,
		    gss_ctx_id_t context_handle,
		    int conf_req_flag,
		    gss_qop_t qop_req,
		    int *conf_state,
		    gss_iov_buffer_desc *iov,
		    int iov_count)
{
    OM_uint32 ret = 0;
    OM_uint32 min = 0;
    unsigned char *iov0 = NULL;
    int iov0_len = 0;
    unsigned char *key = NULL;
    int keylen = 0;
    gss_buffer_desc asn1_mech_oid = {0};

    ret = ntlm_asn1_encode_mech_oid_token(
              &min,
              (gss_OID) gss_mech_ntlm_oid,
              &asn1_mech_oid);
    if (ret)
    {
        goto error;
    }

    /* Fixup iov[0] to have proper GSS/OID header */
    iov0 = asn1_mech_oid.value;
    iov0_len = (int) asn1_mech_oid.length;
    if (iov[0].buffer.value)
    {
        free(iov[0].buffer.value);
    }
    iov[0].buffer.value = iov0;
    iov[0].buffer.length = iov0_len;
    iov[0].type |= GSS_IOV_BUFFER_FLAG_ALLOCATED;

    key = xor_get_encrypt_key(&keylen);
    xor_encrypt(iov[1].buffer.value,
                iov[1].buffer.length,
                key,
                keylen);

    /* TBD: Adam- Don't know the proper return value for this argument */
    *conf_state = conf_req_flag;
error:
    return ret;

}

#endif
