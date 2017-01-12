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



#ifndef _VMKRB5_KRBERROR_S_H
#define _VMKRB5_KRBERROR_S_H

typedef struct _VMKDC_KRB_ERROR {
    int pvno;
    VMKDC_MESSAGE_TYPE msg_type;
    time_t *ctime;
    int cusec;
    time_t stime;
    int susec;
    int error_code;
    PVMKDC_DATA crealm;
    PVMKDC_PRINCIPAL cname;
    PVMKDC_DATA realm;
    PVMKDC_PRINCIPAL sname;
    PVMKDC_DATA e_text;
    PVMKDC_DATA e_data;
} VMKDC_KRB_ERROR, *PVMKDC_KRB_ERROR;

#define VMKDC_SAFE_FREE_KRB_ERROR(x) \
do { \
    if (x) \
    { \
        VmKdcFreeKrbError(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_KRBERROR_S_H */
