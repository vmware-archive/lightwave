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



#ifndef _VMKRB5_ETYPEINFO2_S_H
#define _VMKRB5_ETYPEINFO2_S_H

typedef struct _VMKDC_ETYPE_INFO2_ENTRY {
    VMKDC_ENCTYPE etype;
    PVMKDC_SALT salt;
    PVMKDC_DATA s2kparams;
} VMKDC_ETYPE_INFO2_ENTRY, *PVMKDC_ETYPE_INFO2_ENTRY;

typedef struct _VMKDC_ETYPE_INFO2 {
    DWORD length;
    PVMKDC_ETYPE_INFO2_ENTRY *entry;
} VMKDC_ETYPE_INFO2, *PVMKDC_ETYPE_INFO2;

#define VMKDC_SAFE_FREE_ETYPE_INFO2_ENTRY(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEtypeInfo2Entry(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_ETYPE_INFO2(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEtypeInfo2(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_ETYPEINFO2_S_H */
