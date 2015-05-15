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



#ifndef _VMKRB5_PAENCTSENC_S_H
#define _VMKRB5_PAENCTSENC_S_H

typedef struct _VMKDC_PAENCTSENC {
    time_t patimestamp;
    DWORD *pausec;
} VMKDC_PAENCTSENC, *PVMKDC_PAENCTSENC;

#define VMKDC_SAFE_FREE_PAENCTSENC(x) \
do { \
    if (x) \
    { \
        VmKdcFreePaEncTsEnc(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_PAENCTSENC_S_H */
