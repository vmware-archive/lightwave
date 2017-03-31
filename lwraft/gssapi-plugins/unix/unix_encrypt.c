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
 * Module: srp_encrypt.c
 * Abstract:
 *     VMware GSSAPI SRP Authentication Plugin
 *     Functions related to SRP data encryption
 *
 * Author: Adam Bernstein (abernstein@vmware.com)
 */

#include <krb5.h>
#include <sasl/saslutil.h>
#include <errno.h>
#include <string.h>
#include "unix_encrypt.h"
#include "unix_util.h"
#include "gssapi_alloc.h"

krb5_error_code
srp_gen_keyblock(
    krb5_context krb5_ctx,
    char *enc_keytype,
    char *pass,
    char *salt,
    krb5_keyblock *key)
{
    krb5_error_code krb_err = 0;
    krb5_enctype enctype;
    krb5_data pass_data = {0};
    krb5_data salt_data = {0};

    memset(&enctype, 0, sizeof(enctype));

    pass_data.data = pass;
    pass_data.length = (int) strlen(pass);
    salt_data.data = salt;
    salt_data.length = (int) strlen(salt);

#if 0
    /* Prefer to use this, as it takes ENCTYPE_AES256_CTS_HMAC_SHA1_96 */
    enctype = find_enctype(enc_keytype);
    if (!enctype)
    {
        krb_err = EINVAL;
        goto error;
    }
#else
    krb_err = krb5_string_to_enctype(
                  enc_keytype,
                  &enctype);
    if (krb_err)
    {
        goto error;
    }
#endif

    krb_err = krb5_c_string_to_key(
                  krb5_ctx,
                  enctype,
                  &pass_data,
                  &salt_data,
                  key);
    if (krb_err)
    {
        goto error;
    }

error:

    return krb_err;
}

static krb5_error_code
srp_expand_session_key(
    const char *pass,
    int passlen,
    const unsigned char *salt,
    int saltlen,
    int iter,
    int keylen,
    unsigned char *out)
{
    krb5_error_code sts = 0;
    sts = PKCS5_PBKDF2_HMAC(
              pass,
              passlen,
              salt,
              saltlen,
              iter,
              SRP_EXPAND_KEY_HASH,
              keylen,
              out);
    return sts == 0 ? EINVAL : 0;
}

static int
srp_init_hmac(
    HMAC_CTX *phctx,
    unsigned char *key,
    int key_len)
{
    int sts = 0;
    HMAC_CTX hctx;
    unsigned char md[40] = {0};
    unsigned int mdlen = 0;

    memset(&hctx, 0, sizeof(hctx));
    HMAC_CTX_init(&hctx);
    sts = HMAC_Init_ex(&hctx, key, key_len, EVP_sha1(), NULL);
    if (sts == 0)
    {
        return sts;
    }
    HMAC_Update(&hctx, "", 0);
    HMAC_Final(&hctx, md, &mdlen);

    *phctx = hctx;
    return 0;
}

static int
srp_compute_hmac(
    HMAC_CTX hctx,
    unsigned char *data,
    int data_len,
    unsigned char *md,
    int *md_len)
{
    int sts = 0;

    /* These functions return 0 on error, 1 for success */
    sts = HMAC_Init_ex(&hctx, NULL, 0, EVP_sha1(), NULL);
    if (sts == 0)
    {
        return sts;
    }
    sts = HMAC_Update(&hctx, data, data_len);
    if (sts == 0)
    {
        return sts;
    }
    sts = HMAC_Final(&hctx, md, md_len);
    if (sts == 0)
    {
        return sts;
    }
    return sts;
}

krb5_error_code
srp_make_enc_keyblock(
    srp_gss_ctx_id_t srp_context_handle)
{
    char *srp_session_key_str = NULL;
    unsigned char *hmac_key = NULL;
    int b64_alloc_len = 0;

    unsigned char *ptr_expanded_key = NULL;
    unsigned char expanded_session_key[SRP_EXPAND_KEY_LEN] = {0};

    unsigned char srp_session_key[SRP_EXPAND_SESSION_KEY_LEN] = {0};
    int srp_session_key_len = sizeof(srp_session_key);

    unsigned char iv_data[AES_BLOCK_SIZE] = {0};
    int iv_data_len = sizeof(iv_data);

    int b64_session_key_len = 0;
    krb5_error_code krb5_err = KRB5_BAD_ENCTYPE;

    if (!srp_context_handle->srp_session_key ||
        srp_context_handle->srp_session_key_len == 0)
    {
        krb5_err = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }

    srp_print_hex(srp_context_handle->srp_session_key,
                  srp_context_handle->srp_session_key_len,
                  "srp_make_enc_keyblock: SRP-negotiated session key ");

    /* Expand SRP session key to obtain more bytes for IV/session key */
    krb5_err = srp_expand_session_key(
                   srp_context_handle->srp_session_key,
                   srp_context_handle->srp_session_key_len,
                   srp_context_handle->upn_name,         /* salt */
                   (int) strlen(srp_context_handle->upn_name), /* salt length */
                   SRP_EXPAND_KEY_ITER,
                   sizeof(expanded_session_key),
                   expanded_session_key);
    if (krb5_err)
    {
        goto error;
    }

    /* Carve up parts of the expanded key for various purposes */
    ptr_expanded_key = expanded_session_key;

    /* Initialization vector */
    memcpy(iv_data, ptr_expanded_key, iv_data_len);
    ptr_expanded_key += iv_data_len;

    srp_print_hex(iv_data,
                  iv_data_len,
                  "srp_make_enc_keyblock: got initialization vector ");

    /* SRP "derived session" key */
    memcpy(srp_session_key, ptr_expanded_key, srp_session_key_len);
    ptr_expanded_key += sizeof(srp_session_key);

    /* HMAC key, remaining 16 bytes */
    hmac_key = ptr_expanded_key;

    srp_print_hex(srp_session_key,
                  srp_session_key_len,
                  "srp_make_enc_keyblock: got derived session key");

    /* Build b64 encoded string of SRP session key */
    b64_alloc_len = (srp_session_key_len + 2) / 3  * 4 + 1;
    srp_session_key_str = calloc(b64_alloc_len, sizeof(char));
    if (!srp_session_key_str)
    {
        krb5_err = ENOMEM;
        goto error;
    }

    krb5_err = sasl_encode64(
                   srp_session_key,
                   srp_session_key_len,
                   srp_session_key_str,
                   b64_alloc_len,
                   &b64_session_key_len);
    if (krb5_err)
    {
        krb5_err = ENOMEM;
        goto error;
    }
    srp_session_key_str[b64_session_key_len] = '\0';

    srp_context_handle->keyblock = calloc(1, sizeof(krb5_keyblock));
    if (!srp_context_handle->keyblock)
    {
        krb5_err = ENOMEM;
        goto error;
    }

    /* Generate encryption key from SRP shared key */
    krb5_err = srp_gen_keyblock(
                   srp_context_handle->krb5_ctx,
                   SRP_ENC_KEYTYPE,
                   srp_session_key_str,
                   srp_context_handle->upn_name,
                   srp_context_handle->keyblock);
    if (krb5_err)
    {
        goto error;
    }

    srp_print_hex(srp_context_handle->keyblock->contents,
                  srp_context_handle->keyblock->length,
                  "srp_make_enc_keyblock: keyblock value");

     memset(srp_context_handle->aes_encrypt_iv, 0, iv_data_len);
     memcpy(srp_context_handle->aes_encrypt_iv, iv_data, iv_data_len);

     memset(srp_context_handle->aes_decrypt_iv, 0, iv_data_len);
     memcpy(srp_context_handle->aes_decrypt_iv, iv_data, iv_data_len);

    AES_set_encrypt_key(
        srp_context_handle->keyblock->contents,
        srp_context_handle->keyblock->length * 8,
        &srp_context_handle->aes_encrypt_key);
    AES_set_decrypt_key(
        srp_context_handle->keyblock->contents,
        srp_context_handle->keyblock->length * 8,
        &srp_context_handle->aes_decrypt_key);

    if (srp_init_hmac(&srp_context_handle->hmac_ctx,
                      hmac_key,
                      SRP_EXPAND_HMAC_KEY))
    {
        krb5_err = ENOMEM;
        goto error;
    }

error:
    if (krb5_err)
    {
        if (srp_context_handle->keyblock)
        {
            free(srp_context_handle->keyblock);
        }
    }

    if (srp_session_key_str)
    {
        free(srp_session_key_str);
    }
    return krb5_err;
}

int
srp_encrypt_aes256_hmac_sha1(
    srp_gss_ctx_id_t srp_context_handle,
    unsigned char *plaintext,
    int plaintext_len,
    unsigned char *out_ciphertext,
    unsigned char **out_hmacbuf,
    int *out_hmacbuf_len)
{
    int sts = 0;
    int hmacbuf_len = 0;
    int hmac_bufpad_len = 0;
    int ciphertext_len = 0;
    int ciphertext_pad_len = 0;
    int verifier_len = 0;
    unsigned char *hmacbuf = NULL;
    unsigned char *hmacbuf_end = NULL;
    unsigned char *ciphertext = NULL;
    unsigned char *ret_hmacbuf = NULL;

    /*
     * Message format:
     *   ciphertext = AES256(key, plaintext)
     *   |-- HMAC-SHA1(ciphertext) (20) --|-- ciphertext --|)
     *
     * Result:
     *   Contiguous ciphertext buffer is split into two pieces across
     *   iov, as iov[1] cannot be resized, but iov[0] can.
     *
     *     iov[0] data: |-- AES256 (verifier-len) --|
     *     iov[1] data: |-- AES256 (plaintext-len) --|
     */

    ciphertext_pad_len = AES256PAD(plaintext_len);
    /*
     * Note: The below padding may cause buffer expansion which cannot fit into
     * the original iov[1] payload buffer. The "residual data" from this
     * expansion is returned in iov[0], semantically the hmac verifier.
     */
    hmac_bufpad_len = ciphertext_pad_len + SRP_SHA1_HMAC_BUFSIZ;
    hmacbuf = (unsigned char *) calloc(hmac_bufpad_len, sizeof(unsigned char));
    if (!hmacbuf)
    {
        sts = ENOMEM;
        goto error;
    }
    hmacbuf_end = hmacbuf;

    /* Same size as the input buffer; holds the output cipher text */
    ciphertext_len = ciphertext_pad_len;
    ciphertext = (unsigned char *) calloc(ciphertext_len,
                                          sizeof(unsigned char));
    if (!ciphertext)
    {
        sts = ENOMEM;
        goto error;
    }

    /* AES256 encrypt the plaintext payload data */
    AES_cbc_encrypt(
        plaintext,
        ciphertext,
        ciphertext_len,
        &srp_context_handle->aes_encrypt_key,
        srp_context_handle->aes_encrypt_iv,
        AES_ENCRYPT);

    /* Perform hmac-sha validation over ciphertext payload */
    if (!srp_compute_hmac(
         srp_context_handle->hmac_ctx,
         ciphertext,
         ciphertext_len,
         hmacbuf,
         &hmacbuf_len))
    {
        sts = EINVAL;
        goto error;
    }

    if (hmacbuf_len > SRP_SHA1_HMAC_BUFSIZ)
    {
        hmacbuf_len = SRP_SHA1_HMAC_BUFSIZ;
    }
    hmacbuf_end += hmacbuf_len;

    srp_print_hex(hmacbuf,
                  hmacbuf_len,
                  "srp_encrypt_aes256_hmac_sha1: hmac =");

    /* Copy the ciphertext message after the HMAC data */
    memcpy(hmacbuf_end, ciphertext, ciphertext_len);

    /* Verifier data: what cannot fit into iov[1] */
    verifier_len = hmac_bufpad_len - ciphertext_len;

    ret_hmacbuf = (unsigned char *) calloc(verifier_len,
                                           sizeof(unsigned char));
    if (!ret_hmacbuf)
    {
        sts = ENOMEM;
        goto error;
    }

    /* Split cipher text into two iov values: iov[0] = HMAC code */
    memcpy(ret_hmacbuf, hmacbuf, verifier_len);

    /* iov[1] = cipher text */
    memcpy(out_ciphertext,
           hmacbuf + verifier_len,
           plaintext_len);

    /* Additional iov[0] length due to padding expansion */
    *out_hmacbuf = ret_hmacbuf;
    *out_hmacbuf_len = verifier_len;

error:
    if (sts)
    {
        if (ret_hmacbuf)
        {
            free(ret_hmacbuf);
        }
    }
    if (hmacbuf)
    {
        free(hmacbuf);
    }
    if (ciphertext)
    {
        free(ciphertext);
    }
    return sts;
}


int
srp_decrypt_aes256_hmac_sha1(
    srp_gss_ctx_id_t srp_context_handle,
    unsigned char *in_hmacbuf,
    int in_hmacbuf_len,
    unsigned char *in_ciphertext,
    int in_ciphertext_len,
    unsigned char *ret_plaintext)
{
    int sts = 0;
    unsigned char *cipherhmac_buf = NULL;
    unsigned char *plaintext = NULL;
    unsigned char *ciphertext_start = NULL;
    int cipherhmac_buf_len = 0;
    int ciphertext_len = 0;
    int hmac_computed_len = 0;
    unsigned char hmac[SRP_SHA1_HMAC_BUFSIZ] = {0};
    unsigned char hmac_computed[SRP_SHA1_HMAC_BUFSIZ] = {0};

    /* Splice in_hmacbuf + in_ciphertext together, this is the ciphertext */
    cipherhmac_buf_len = in_hmacbuf_len + in_ciphertext_len;

    /* Buffer must adhere to AES-256 padding requirements */
    ciphertext_len = AES256PAD((cipherhmac_buf_len - SRP_SHA1_HMAC_BUFSIZ));
    cipherhmac_buf = (unsigned char *) calloc(cipherhmac_buf_len,
                                              sizeof(unsigned char));
    if (!cipherhmac_buf)
    {
        sts = ENOMEM;
        goto error;
    }

    plaintext = (unsigned char *) calloc(ciphertext_len,
                                         sizeof(unsigned char));
    if (!plaintext )
    {
        sts = ENOMEM;
        goto error;
    }

    memcpy(cipherhmac_buf, in_hmacbuf, in_hmacbuf_len);
    memcpy(cipherhmac_buf + in_hmacbuf_len, in_ciphertext, in_ciphertext_len);

    /* Save the HMAC-SHA1 verifier from client */
    memcpy(hmac, cipherhmac_buf, SRP_SHA1_HMAC_BUFSIZ);

    srp_print_hex(hmac,
                   SRP_SHA1_HMAC_BUFSIZ,
                   "srp_decrypt_aes256_hmac_sha1: client hmac");

    /* Perform hmac-sha validation over the ciphertext */
    ciphertext_start = cipherhmac_buf + SRP_SHA1_HMAC_BUFSIZ;
    if (!srp_compute_hmac(
         srp_context_handle->hmac_ctx,
         ciphertext_start,
         ciphertext_len,
         hmac_computed,
         &hmac_computed_len))
    {
        sts = EINVAL;
        goto error;
    }

    /* Verify computed verifier matches client verifier */
    if (hmac_computed_len != SRP_SHA1_HMAC_BUFSIZ ||
        memcmp(hmac, hmac_computed, hmac_computed_len) != 0)
    {
        srp_print_hex(hmac_computed,
                       SRP_SHA1_HMAC_BUFSIZ,
                       "srp_decrypt_aes256_hmac_sha1: ERROR computed hmac");
        /* verifier failed, return error */
        sts = GSS_S_DEFECTIVE_TOKEN;
        goto error;
    }

    /* This is the full ciphertext, which can then be decrypted. */
    AES_cbc_encrypt(ciphertext_start,
                    plaintext,
                    ciphertext_len,
                    &srp_context_handle->aes_decrypt_key,
                    srp_context_handle->aes_decrypt_iv,
                    AES_DECRYPT);

    memcpy(ret_plaintext, plaintext, ciphertext_len);

error:
    if (plaintext)
    {
        free(plaintext);
    }
    if (cipherhmac_buf)
    {
        free(cipherhmac_buf);
    }
    return sts;
}

#ifdef _SRP_USE_TRIVIAL_ENCRYPTION

/* Straw-man trivial encryption function */
void xor_encrypt(
    unsigned char *plaintext,
    int plaintext_len,
    const unsigned char *key,
    int keylen)
{
    int i = 0;
    int k = 0;

    for (i=0; i<plaintext_len; i++)
    {
        plaintext[i] ^= key[k++];
        k %= keylen;
    }
    return;
}

unsigned char *xor_get_encrypt_key(int *len)
{
    static char *g_key = "!)@(#*%&^dEadBeEf~!@#$%^&*()_+";
    *len = strlen(g_key);

    return (unsigned char *) g_key;
}
#endif
