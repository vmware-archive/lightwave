#ifndef _CRYPTO_H
#define _CRYPTO_H

//#include "includes.h"

typedef struct _VMKDC_KRB5_CONTEXT VMKDC_KRB5_CONTEXT, *PVMKDC_KRB5_CONTEXT;
typedef struct _VMKDC_CRYPTO_CONTEXT VMKDC_CRYPTO_CONTEXT, *PVMKDC_CRYPTO_CONTEXT;

typedef struct _VMKDC_CRYPTO
{
    struct _VMKDC_KRB5_CONTEXT *krb5Ctx;
    struct _VMKDC_CRYPTO_CONTEXT *krb5Crypto;
} VMKDC_CRYPTO, *PVMKDC_CRYPTO;

#endif
