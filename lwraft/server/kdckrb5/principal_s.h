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



#ifndef _VMKRB5_PRINCIPAL_S_H
#define _VMKRB5_PRINCIPAL_S_H

typedef struct _VMKDC_PRINCIPAL {
    VMKDC_NAME_TYPE type;
    PSTR name;
    VMKDC_DATA *realm;
    DWORD numComponents;
    PVMKDC_DATA *components;
} VMKDC_PRINCIPAL, *PVMKDC_PRINCIPAL;

#define VMKDC_SAFE_FREE_PRINCIPAL(x) \
do { \
    if (x) \
    { \
        VmKdcFreePrincipal(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_PRINCIPAL_H */
