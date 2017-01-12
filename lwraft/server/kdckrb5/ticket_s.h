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



#ifndef _VMKRB5_TICKET_S_H
#define _VMKRB5_TICKET_S_H

typedef DWORD VMKDC_TICKET_FLAGS;
typedef char *PVMKDC_ADDRESSES;
typedef char *PVMKDC_AUTHZDATA;

typedef struct _VMKDC_TRANSITED_ENCODING {
    DWORD tr_type;
    PVMKDC_DATA contents;
} VMKDC_TRANSITED_ENCODING, *PVMKDC_TRANSITED_ENCODING;

typedef struct _VMKDC_ENCTICKETPART{
    VMKDC_TICKET_FLAGS flags;
    PVMKDC_KEY key;
    PVMKDC_DATA crealm;
    PVMKDC_PRINCIPAL cname;
    PVMKDC_TRANSITED_ENCODING transited;
    time_t authtime;
    time_t *starttime; /* optional */
    time_t endtime;
    time_t *renew_till; /* optional */
    PVMKDC_ADDRESSES caddr; /* optional */
    PVMKDC_AUTHZDATA authorization_data; /* optional */
} VMKDC_ENCTICKETPART, *PVMKDC_ENCTICKETPART;

typedef struct _VMKDC_TICKET {
    DWORD tkt_vno;
    PVMKDC_DATA realm;
    PVMKDC_PRINCIPAL sname;
    PVMKDC_ENCDATA enc_part;
} VMKDC_TICKET, *PVMKDC_TICKET;

#define VMKDC_FLAG_SET(x,y) ((x) |= 1<<(y))
#define VMKDC_FLAG_ISSET(x,y) (((x) & 1<<(y))!=0)

#define VMKDC_SAFE_FREE_TICKET(x) \
do { \
    if (x) \
    { \
        VmKdcFreeTicket(x); \
        x = NULL; \
    } \
} while (0)

#define VMKDC_SAFE_FREE_ENCTICKETPART(x) \
do { \
    if (x) \
    { \
        VmKdcFreeEncTicketPart(x); \
        x = NULL; \
    } \
} while (0)

#endif /* _VMKRB5_TICKET_S_H */
