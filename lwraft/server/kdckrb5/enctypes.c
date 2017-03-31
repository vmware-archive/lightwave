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

typedef struct _VMKDC_ENCTYPE_TO_NAME
{
    int encType;
    char *encTypeName;
} VMKDC_ENCTYPE_TO_NAME, *PVMKDC_ENCTYPE_TO_NAME;

static
PSTR
VmKdcEncTypeToName(int encType)
{
    static VMKDC_ENCTYPE_TO_NAME gEncTypeNames[] =
    {
      {
        VMKDC_ENCTYPE_NULL,
        "KRB5_ENCTYPE_NULL",
      },
      {
        VMKDC_ENCTYPE_DES_CBC_CRC,
        "KRB5_ENCTYPE_DES_CBC_CRC"
      },
      {
        VMKDC_ENCTYPE_DES_CBC_MD4,
        "KRB5_ENCTYPE_DES_CBC_MD4"
      },
      {
        VMKDC_ENCTYPE_DES_CBC_MD5,
        "KRB5_ENCTYPE_DES_CBC_MD5"
      },
      {
        VMKDC_ENCTYPE_DES3_CBC_MD5,
        "KRB5_ENCTYPE_DES3_CBC_MD5"
      },
      {
        VMKDC_ENCTYPE_OLD_DES3_CBC_SHA1,
        "KRB5_ENCTYPE_OLD_DES3_CBC_SHA1"
      },
      {
        VMKDC_ENCTYPE_SIGN_DSA_GENERATE,
        "KRB5_ENCTYPE_SIGN_DSA_GENERATE"
      },
      {
        VMKDC_ENCTYPE_ENCRYPT_RSA_PRIV,
        "KRB5_ENCTYPE_ENCRYPT_RSA_PRIV"
      },
      {
        VMKDC_ENCTYPE_ENCRYPT_RSA_PUB,
        "KRB5_ENCTYPE_ENCRYPT_RSA_PUB"
      },
      {
        VMKDC_ENCTYPE_DES3_CBC_SHA1,
        "KRB5_ENCTYPE_DES3_CBC_SHA1"
      },
      {
        VMKDC_ENCTYPE_AES128_CTS_HMAC_SHA1_96,
        "KRB5_ENCTYPE_AES128_CTS_HMAC_SHA1_96"
      },
      {
        VMKDC_ENCTYPE_AES256_CTS_HMAC_SHA1_96,
        "KRB5_ENCTYPE_AES256_CTS_HMAC_SHA1_96"
      },
      {
        VMKDC_ENCTYPE_ARCFOUR_HMAC_MD5,
        "KRB5_ENCTYPE_ARCFOUR_HMAC_MD5"
      },
      {
        VMKDC_ENCTYPE_ARCFOUR_HMAC_MD5_56,
        "KRB5_ENCTYPE_ARCFOUR_HMAC_MD5_56"
      },
      {
        VMKDC_ENCTYPE_ENCTYPE_PK_CROSS,
        "KRB5_ENCTYPE_ENCTYPE_PK_CROSS"
      },
      {
        VMKDC_ENCTYPE_ARCFOUR_MD4,
        "KRB5_ENCTYPE_ARCFOUR_MD4",
      },
      {
        VMKDC_ENCTYPE_ARCFOUR_HMAC_OLD,
        "KRB5_ENCTYPE_ARCFOUR_HMAC_OLD",
      },
      {
        VMKDC_ENCTYPE_ARCFOUR_HMAC_OLD_EXP,
        "KRB5_ENCTYPE_ARCFOUR_HMAC_OLD_EXP",
      },
    };

    PSTR retName = NULL;
    int i;
    BOOLEAN bFound = FALSE;
    CHAR format[12];
    for (i=0; i<sizeof(gEncTypeNames)/sizeof(VMKDC_ENCTYPE_TO_NAME); i++)
    {
        if (encType == gEncTypeNames[i].encType)
        {
            bFound = TRUE;
            break;
        }
    }
    if (bFound)
    {
        retName = gEncTypeNames[i].encTypeName;
    }
    else
    {
        sprintf(format, "%d", encType);
        retName = format;
    }
    return retName;
}

VOID
VmKdcPrintEncTypes(
    PVMKDC_ENCTYPES enctypes)
{
    DWORD i = 0;
    PSTR encTypeName = NULL;

    for (i=0; i<enctypes->count; i++)
    {
        encTypeName = VmKdcEncTypeToName(enctypes->type[i]);
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintEncTypes: etype[%d]  <%s>",
                 i, encTypeName);
    }
}
