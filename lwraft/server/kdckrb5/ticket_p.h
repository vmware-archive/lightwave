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



VOID
VmKdcAllocateTicket(
    PVMKDC_TICKET *ppRetTicket);

VOID
VmKdcFreeTicket(
    PVMKDC_TICKET pTicket);

DWORD
VmKdcMakeTicket(
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_ENCDATA pEncData,
    PVMKDC_TICKET *ppRetTicket);

VOID
VmKdcFreeEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart);

DWORD
VmKdcMakeEncTicketPart(
    VMKDC_TICKET_FLAGS flags,
    PVMKDC_KEY key,
    PVMKDC_PRINCIPAL client,
    PVMKDC_TRANSITED_ENCODING transited,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PVMKDC_ADDRESSES caddr,
    PVMKDC_AUTHZDATA authorization_data,
    PVMKDC_ENCTICKETPART *ppRetEncTicketPart);

VOID
VmKdcPrintEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart);

DWORD
VmKdcEncodeEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart,
    PVMKDC_DATA *ppRetData);

DWORD
VmKdcDecodeEncTicketPart(
    PVMKDC_DATA pData,
    PVMKDC_ENCTICKETPART *ppRetEncTicketPart);

VOID
VmKdcPrintTicket(
    PVMKDC_TICKET pTicket);

DWORD
VmKdcCopyTicket(
    PVMKDC_TICKET pTicket,
    PVMKDC_TICKET *ppRetTicket);

DWORD
VmKdcBuildTicket(
    PVMKDC_CONTEXT pContext,
    PVMKDC_PRINCIPAL pClient,
    PVMKDC_PRINCIPAL pServer,
    PVMKDC_KEY pKey,
    PVMKDC_KEY pSessionKey,
    VMKDC_TICKET_FLAGS flags,
    PVMKDC_TRANSITED_ENCODING transited,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PVMKDC_ADDRESSES caddr,
    PVMKDC_AUTHZDATA authorization_data,
    PVMKDC_TICKET *ppRetTicket);

VOID
VmKdcTicketFlagsSet(
    VMKDC_TICKET_FLAGS flags,
    void *asnFlags);
