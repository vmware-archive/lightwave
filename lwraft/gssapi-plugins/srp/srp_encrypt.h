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



#ifndef _SRP_ENCRYPT_H_
#define _SRP_ENCRYPT_H_

#include <krb5.h>
#include "gssapiP_srp.h"

/*
 * Straw-man trivial encryption functionality: _SRP_USE_TRIVIAL_ENCRYPTION
 *
 * To enable this debugging "feature", add -D_SRP_USE_TRIVIAL_ENCRYPTION
 * to your makefile/vcproj.
 *  !!!!!!!!!!!!! DO NOT USE THIS IN PRODUCTION !!!!!!!!!!!!!!
 */

#define AES256PAD(len) \
    ((len) + (((len%AES_BLOCK_SIZE) > 0) ? \
     (AES_BLOCK_SIZE - (len) % AES_BLOCK_SIZE) : 0))

// #define SRP_ENC_KEYTYPE ENCTYPE_AES256_CTS_HMAC_SHA1_96
#define SRP_ENC_KEYTYPE "aes256-cts-hmac-sha1-96"

#define SRP_EXPAND_KEY_LEN  64
#define SRP_EXPAND_SESSION_KEY_LEN  32
#define SRP_EXPAND_HMAC_KEY 16
#define SRP_EXPAND_KEY_ITER 128
#define SRP_EXPAND_KEY_HASH EVP_sha1()
#define SRP_SHA1_HMAC_BUFSIZ 20
#define SRP_MECH_OID_OFFSET 16

krb5_error_code
srp_gen_keyblock(
    krb5_context krb_ctx,
    char *enc_keytype,
    char *pass,
    char *salt,
    krb5_keyblock *key);

krb5_error_code
srp_make_enc_keyblock(
    srp_gss_ctx_id_t srp_context_handle);

int
srp_encrypt_aes256_hmac_sha1(
    srp_gss_ctx_id_t srp_context_handle,
    unsigned char *plaintext,
    int plaintext_len,
    unsigned char *out_ciphertext,
    unsigned char **out_hmacbuf,
    int *out_hmacbuf_len);

int
srp_decrypt_aes256_hmac_sha1(
    srp_gss_ctx_id_t srp_context_handle,
    unsigned char *in_hmacbuf,
    int in_hmacbuf_len,
    unsigned char *in_ciphertext,
    int in_ciphertext_len,
    unsigned char *ret_plaintext);

#ifdef _SRP_USE_TRIVIAL_ENCRYPTION
void xor_encrypt(
    unsigned char *plaintext,
    int plaintext_len,
    const unsigned char *key,
    int keylen);

unsigned char *xor_get_encrypt_key(int *len);

#endif /* _SRP_USE_TRIVIAL_ENCRYPTION */
#endif
