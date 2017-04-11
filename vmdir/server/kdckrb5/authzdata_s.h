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



#ifndef _VMKRB5_AUTHZDATA_S_H
#define _VMKRB5_AUTHZDATA_S_H

typedef struct _VMKDC_AUTHZ_PAC VMKDC_AUTHZ_PAC, *PVMKDC_AUTHZ_PAC;

typedef struct _VMKDC_AUTHZDATA_ELEM {
    DWORD ad_type;
    PVMKDC_DATA ad_data;
} VMKDC_AUTHZDATA_ELEM, *PVMKDC_AUTHZDATA_ELEM;

typedef struct _VMKDC_AUTHZDATA {
    DWORD count;
    PVMKDC_AUTHZDATA_ELEM *elem;
} VMKDC_AUTHZDATA, *PVMKDC_AUTHZDATA;

typedef DWORD VMKDC_AUTHZ_PAC_INFO_TYPE;

typedef struct _VMKDC_AUTHZ_PAC_INFO {
    VMKDC_AUTHZ_PAC_INFO_TYPE pac_type;
    PVMKDC_DATA pac_data;
} VMKDC_AUTHZ_PAC_INFO, *PVMKDC_AUTHZ_PAC_INFO;

#define VMKDC_SAFE_FREE_AUTHZDATA_ELEM(x) \
do { \
    if (x) \
    { \
        VmKdcFreeAuthzDataElem(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_AUTHZDATA(x) \
do { \
    if (x) \
    { \
        VmKdcFreeAuthzData(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_AUTHZDATA_S_H */
