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



#ifndef _VMKRB5_AUTHENTICATOR_S_H
#define _VMKRB5_AUTHENTICATOR_S_H

typedef struct _VMKDC_AUTHENTICATOR {
    DWORD authenticator_vno;
    PVMKDC_PRINCIPAL cname;
    PVMKDC_CHECKSUM cksum;
    DWORD cusec;
    time_t ctime;
    PVMKDC_KEY subkey;
    DWORD seq_number;
    PVMKDC_AUTHZDATA authorization_data;
} VMKDC_AUTHENTICATOR, *PVMKDC_AUTHENTICATOR;

#define VMKDC_SAFE_FREE_AUTHENTICATOR(x) \
do { \
    if (x) \
    { \
        VmKdcFreeAuthenticator(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_AUTHENTICATOR_S_H */
