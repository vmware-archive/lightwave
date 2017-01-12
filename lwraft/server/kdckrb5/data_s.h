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



#ifndef _VMKRB5_DATA_S_H
#define _VMKRB5_DATA_S_H

#define VMKDC_DATA_INTERNAL_BUFSIZE 64

typedef struct _VMKDC_DATA {
    DWORD   len;
    PVOID   ptr;
    BOOLEAN bAllocated;
    UCHAR   internalBuf[VMKDC_DATA_INTERNAL_BUFSIZE];
} VMKDC_DATA, *PVMKDC_DATA;

#define VMKDC_GET_LEN_DATA(x) ((x)->len)
#define VMKDC_GET_PTR_DATA(x) ((x)->ptr)

#define VMKDC_SAFE_FREE_DATA(x) \
do { \
    if (x) \
    { \
        VmKdcFreeData(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_DATA_S_H */
