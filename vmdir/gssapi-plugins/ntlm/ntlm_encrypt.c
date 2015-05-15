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



#include <krb5.h>
#include <sasl/saslutil.h>
#include <errno.h>
#include <string.h>
#include "ntlm_encrypt.h"
#include "ntlm_util.h"

krb5_error_code
ntlm_gen_keyblock(
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

krb5_error_code 
ntlm_make_enc_keyblock(
    ntlm_gss_ctx_id_t ntlm_context_handle)
{
    char *ntlm_session_key_str = NULL;
    krb5_error_code krb5_err = KRB5_BAD_ENCTYPE;
    int b64_alloc_len = ntlm_context_handle->ntlm_session_key_len * 4 / 3 + 3;
    int b64_session_key_len = 0;
    int sts = 0;

    if (ntlm_context_handle->ntlm_session_key &&
        ntlm_context_handle->ntlm_session_key_len > 0)
    {

        /* Build b64 encoded string of NTLM session key */
        ntlm_session_key_str = calloc(b64_alloc_len, sizeof(char));
        if (!ntlm_session_key_str)
        {
            krb5_err = ENOMEM;
            goto error;
        }

        sts = sasl_encode64(
                  ntlm_context_handle->ntlm_session_key,
                  ntlm_context_handle->ntlm_session_key_len,
                  ntlm_session_key_str,
                  b64_alloc_len,
                  &b64_session_key_len);
        if (sts)
        {
            krb5_err = ENOMEM;
            goto error;
        }
        ntlm_session_key_str[b64_session_key_len] = '\0';
#if 1
/* TBD: Adam debuging only */
        ntlm_print_hex(ntlm_context_handle->ntlm_session_key,
                      ntlm_context_handle->ntlm_session_key_len,
                      "ntlm_make_enc_keyblock: got session key");
#endif
        ntlm_context_handle->keyblock = calloc(1, sizeof(krb5_keyblock));
        if (!ntlm_context_handle->keyblock)
        {
            krb5_err = ENOMEM;
            goto error;
        }

        /* Generate encryption key from NTLM shared key */
        krb5_err = ntlm_gen_keyblock(
                       ntlm_context_handle->krb5_ctx,
                       NTLM_ENC_KEYTYPE,
                       ntlm_session_key_str,
                       ntlm_context_handle->upn_name,
                       ntlm_context_handle->keyblock);
#if 1
/* TBD: Adam debuging only */
        ntlm_print_hex(ntlm_context_handle->keyblock->contents,
                      ntlm_context_handle->keyblock->length,
                      "ntlm_make_enc_keyblock: keyblock value");
#endif
    }

error:
    if (krb5_err)
    {
        if (ntlm_context_handle->keyblock)
        {
            free(ntlm_context_handle->keyblock);
        }
    }

    if (ntlm_session_key_str)
    {
        free(ntlm_session_key_str);
    }
    return krb5_err;
}


#ifdef _NTLM_USE_TRIVIAL_ENCRYPTION

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
