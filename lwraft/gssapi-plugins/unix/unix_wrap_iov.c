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
 * Module: srp_wrap_iov.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Implements SRP wrap IOV; sign/seal support
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include <openssl/aes.h>
#include <openssl/rand.h>

#include <string.h>
#include <errno.h>
#include "unix_util.h"
#include "unix_encrypt.h"
#include "gssapi_alloc.h"

#ifndef _SRP_USE_TRIVIAL_ENCRYPTION

OM_uint32
srp_gss_wrap_iov(OM_uint32 *minor_status,
		    gss_ctx_id_t context_handle,
		    int conf_req_flag,
		    gss_qop_t qop_req,
		    int *conf_state,
		    gss_iov_buffer_desc *iov,
		    int iov_count)
{
    OM_uint32 ret = 0;
    OM_uint32 min = 0;
    srp_gss_ctx_id_t srp_context_handle = (srp_gss_ctx_id_t) context_handle;
    gss_buffer_desc asn1_mech_oid = {0};
    unsigned char *plaintext = NULL;
    unsigned char *ciphertext = NULL;
    int plaintext_len = 0;
    int ciphertext_len = 0;
    int iov0buf_len = 128;
    unsigned char *iov0buf = NULL;
    int hmacbuf_len = 0;
    unsigned char *hmacbuf = NULL;

    iov0buf = (unsigned char *) gssalloc_calloc(iov0buf_len, sizeof(unsigned char));
    if (!iov0buf)
    {
        min = ENOMEM;
        goto error;
    }

    ret = srp_asn1_encode_mech_oid_token(
              &min,
              (gss_OID) gss_mech_srp_oid,
              &asn1_mech_oid);
    if (ret)
    {
        goto error;
    }

    if (iov[0].buffer.value)
    {
        gssalloc_free(iov[0].buffer.value);
        iov[0].buffer.value = NULL;
    }

    memcpy(iov0buf, asn1_mech_oid.value, asn1_mech_oid.length);
    iov[0].buffer.value = iov0buf;
    iov[0].buffer.length = iov0buf_len;
    iov[0].type |= GSS_IOV_BUFFER_FLAG_ALLOCATED;
    gssalloc_free(asn1_mech_oid.value);
    asn1_mech_oid.value = NULL;

    plaintext_len = (int) iov[1].buffer.length;
    if (plaintext_len != iov[1].buffer.length)
    {
        /* This may not work if the input buffer size isn't already aligned */
        plaintext = calloc(plaintext_len, sizeof(unsigned char));
        if (!plaintext)
        {
            min = ENOMEM;
            goto error;
        }
        memcpy(plaintext, iov[1].buffer.value, iov[1].buffer.length);
    }
    else
    {
        plaintext = iov[1].buffer.value;
    }

    ciphertext_len = plaintext_len;
    ciphertext = calloc(ciphertext_len, sizeof(unsigned char));

    min = srp_encrypt_aes256_hmac_sha1(
            srp_context_handle,
            plaintext,
            plaintext_len,
            ciphertext,
            &hmacbuf,
            &hmacbuf_len);
    if (min)
    {
        goto error;
    }
    memcpy(iov[1].buffer.value, ciphertext, ciphertext_len);

    if (hmacbuf_len > 0)
    {
        memcpy(((unsigned char *) iov[0].buffer.value) + SRP_MECH_OID_OFFSET,
               hmacbuf,
               hmacbuf_len);
        iov[0].buffer.length = SRP_MECH_OID_OFFSET + hmacbuf_len;
    }
    else
    {
        min = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }

    /* TBD: Adam- Don't know the proper return value for this argument */
    *conf_state = conf_req_flag;

error:
    if (plaintext && plaintext != iov[1].buffer.value)
    {
        free(plaintext);
    }
    if (ciphertext)
    {
        free(ciphertext);
    }
    if (hmacbuf)
    {
        free(hmacbuf);
    }
    if (ret)
    {
        if (iov0buf)
        {
            gssalloc_free(iov0buf);
        }
    }
    return min ? min : ret;
}

#else

OM_uint32
srp_gss_wrap_iov(OM_uint32 *minor_status,
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

    ret = srp_asn1_encode_mech_oid_token(
              &min,
              (gss_OID) gss_mech_srp_oid,
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
        gssalloc_free(iov[0].buffer.value);
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
