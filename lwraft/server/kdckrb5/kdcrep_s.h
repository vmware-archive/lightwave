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



#ifndef _VMKRB5_KDCREP_S_H
#define _VMKRB5_KDCREP_S_H

typedef struct _VMKDC_LAST_REQ {
    char *dummy;
} VMKDC_LAST_REQ, *PVMKDC_LAST_REQ;

typedef struct _VMKDC_ENCKDCREPPART {
    PVMKDC_KEY key;
    PVMKDC_LAST_REQ last_req;
    DWORD nonce;
    time_t *key_expiration; /* optional */
    DWORD flags;
    time_t authtime;
    time_t *starttime; /* optional */
    time_t endtime;
    time_t *renew_till; /* optional */
    PVMKDC_DATA srealm;
    PVMKDC_PRINCIPAL sname;
    PUCHAR caddr; /* optional */
    PUCHAR encrypted_pa_data; /* optional */
} VMKDC_ENCKDCREPPART, *PVMKDC_ENCKDCREPPART;

typedef VMKDC_ENCKDCREPPART VMKDC_ENCASREPPART, *PVMKDC_ENCASREPPART;
typedef VMKDC_ENCKDCREPPART VMKDC_ENCTGSREPPART, *PVMKDC_ENCTGSREPPART;

typedef struct _VMKDC_KDCREP {
    DWORD pvno;
    VMKDC_MESSAGE_TYPE msg_type;
    PUCHAR padata; /* optional */
    PVMKDC_DATA crealm;
    PVMKDC_PRINCIPAL cname;
    PVMKDC_TICKET ticket;
    PVMKDC_ENCDATA enc_part;
} VMKDC_KDCREP, *PVMKDC_KDCREP;

typedef VMKDC_KDCREP VMKDC_ASREP, *PVMKDC_ASREP;
typedef VMKDC_KDCREP VMKDC_TGSREP, *PVMKDC_TGSREP;

#define VMKDC_SAFE_FREE_ASREP(x) \
do { \
    if (x) \
    { \
        VmKdcFreeAsRep(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_TGSREP(x) \
do { \
    if (x) \
    { \
        VmKdcFreeTgsRep(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_ENCASREPPART(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEncAsRepPart(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_ENCTGSREPPART(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEncTgsRepPart(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_KDCREP_S_H */
