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



#ifndef _NTLM_ENCRYPT_H_
#define _NTLM_ENCRYPT_H_

#include <krb5.h>
#include "gssapiP_ntlm.h"

/*
 * Straw-man trivial encryption functionality: _NTLM_USE_TRIVIAL_ENCRYPTION
 *
 * To enable this debugging "feature", add -D_NTLM_USE_TRIVIAL_ENCRYPTION
 * to your makefile/vcproj.
 *  !!!!!!!!!!!!! DO NOT USE THIS IN PRODUCTION !!!!!!!!!!!!!!
 */

#define AES256PAD(len) \
    ((len) + (((len%AES_BLOCK_SIZE) > 0) ? \
     (AES_BLOCK_SIZE - (len) % AES_BLOCK_SIZE) : 0))

// #define NTLM_ENC_KEYTYPE ENCTYPE_AES256_CTS_HMAC_SHA1_96
#define NTLM_ENC_KEYTYPE "aes256-cts-hmac-sha1-96"

krb5_error_code
ntlm_gen_keyblock(
    krb5_context krb_ctx,
    char *enc_keytype,
    char *pass,
    char *salt,
    krb5_keyblock *key);

krb5_error_code
ntlm_make_enc_keyblock(
    ntlm_gss_ctx_id_t ntlm_context_handle);

#ifdef _NTLM_USE_TRIVIAL_ENCRYPTION
void xor_encrypt(
    unsigned char *plaintext,
    int plaintext_len,
    const unsigned char *key,
    int keylen);

unsigned char *xor_get_encrypt_key(int *len);

#endif /* _NTLM_USE_TRIVIAL_ENCRYPTION */
#endif
