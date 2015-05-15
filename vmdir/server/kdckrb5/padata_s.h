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



#ifndef _VMKRB5_PADATA_S_H
#define _VMKRB5_PADATA_S_H

typedef struct _PVMKDC_PADATA {
    VMKDC_PADATA_TYPE type;
    PVMKDC_DATA data;
} VMKDC_PADATA, *PVMKDC_PADATA;

typedef struct _PVMKDC_METHOD_DATA {
    DWORD length;
    PVMKDC_PADATA *padata;
} VMKDC_METHOD_DATA, *PVMKDC_METHOD_DATA;

#define VMKDC_SAFE_FREE_PADATA(x) \
do { \
    if (x) \
    { \
        VmKdcFreePaData(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_METHOD_DATA(x) \
do { \
    if (x) \
    { \
        VmKdcFreeMethodData(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_PADATA_S_H */
