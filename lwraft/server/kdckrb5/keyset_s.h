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



#ifndef _VMKRB5_KEYSET_S_H
#define _VMKRB5_KEYSET_S_H

typedef struct _VMKDC_SALT {
    VMKDC_SALTTYPE type;
    PVMKDC_DATA data;
} VMKDC_SALT, *PVMKDC_SALT;

typedef struct _VMKDC_ENCKEY {
    VMKDC_KEYTYPE keytype;
    PVMKDC_ENCDATA encdata;
} VMKDC_ENCKEY, *PVMKDC_ENCKEY;

typedef struct _VMKDC_KEYSET {
    DWORD numKeys;
    DWORD kvno;
    PVMKDC_ENCKEY *encKeys;
    PVMKDC_KEY *keys;
    PVMKDC_SALT *salts;
} VMKDC_KEYSET, *PVMKDC_KEYSET;

#define VMKDC_SAFE_FREE_SALT(x) \
do { \
    if (x) \
    { \
        VmKdcFreeSalt(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_ENCKEY(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEncKey(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_KEYSET(x) \
do { \
    if (x) \
    { \
        VmKdcFreeKeySet(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_KEYSET_S_H */
