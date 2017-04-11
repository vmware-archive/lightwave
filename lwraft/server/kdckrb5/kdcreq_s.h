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



#ifndef _VMKRB5_KDCREQ_S_H
#define _VMKRB5_KDCREQ_S_H

typedef struct _VMKDC_KDCREQ_BODY {
    DWORD kdc_options;
    PVMKDC_PRINCIPAL cname; /* optional */
    PVMKDC_DATA realm;
    PVMKDC_PRINCIPAL sname; /* optional */
    time_t *from; /* optional */
    time_t *till; /* optional */
    time_t *rtime; /* optional */
    DWORD nonce;
    VMKDC_ENCTYPES etype;
    PUCHAR addresses; /* TBD */
    PUCHAR enc_authorization_data; /* TBD */
    PUCHAR *additional_tickets; /* TBD */
} VMKDC_KDCREQ_BODY, *PVMKDC_KDCREQ_BODY;

typedef struct _VMKDC_KDCREQ {
    DWORD pvno;
    VMKDC_MESSAGE_TYPE msg_type;
    PVMKDC_METHOD_DATA padata; /* optional */
    VMKDC_KDCREQ_BODY req_body;
} VMKDC_KDCREQ, *PVMKDC_KDCREQ;

#endif /* _VMKRB5_KDCREQ_S_H */
