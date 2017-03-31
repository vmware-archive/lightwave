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
VmKdcFreeAsRep(
    PVMKDC_ASREP pAsRep);

VOID
VmKdcFreeTgsRep(
    PVMKDC_TGSREP pTgsRep);

VOID
VmKdcFreeEncAsRepPart(
    PVMKDC_ENCASREPPART pEncAsRepPart);

VOID
VmKdcFreeEncTgsRepPart(
    PVMKDC_ENCTGSREPPART pEncTgsRepPart);

DWORD
VmKdcMakeAsRep(
    PVMKDC_PRINCIPAL client,
    PVMKDC_TICKET ticket,
    PVMKDC_ENCDATA enc_part,
    PVMKDC_ASREP *ppRetAsRep);

DWORD
VmKdcMakeTgsRep(
    PVMKDC_PRINCIPAL client,
    PVMKDC_TICKET ticket,
    PVMKDC_ENCDATA enc_part,
    PVMKDC_ASREP *ppRetTgsRep);

DWORD
VmKdcEncodeAsRep(
    PVMKDC_ASREP pAsRep,
    PVMKDC_DATA *ppAsnData);

DWORD
VmKdcEncodeTgsRep(
    PVMKDC_TGSREP pTgsRep,
    PVMKDC_DATA *ppAsnData);

DWORD
VmKdcMakeEncAsRepPart(
    PVMKDC_KEY key,
    PVMKDC_LAST_REQ last_req,
    DWORD nonce,
    time_t *key_expiration,
    DWORD flags,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PVMKDC_PRINCIPAL server,
    PUCHAR caddr,
    PVMKDC_ENCASREPPART *ppRetEncAsRepPart);

DWORD
VmKdcMakeEncTgsRepPart(
    PVMKDC_KEY key,
    PVMKDC_LAST_REQ last_req,
    DWORD nonce,
    time_t *key_expiration,
    DWORD flags,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PVMKDC_PRINCIPAL server,
    PUCHAR caddr,
    PVMKDC_ENCTGSREPPART *ppRetEncTgsRepPart);

DWORD
VmKdcEncodeEncAsRepPart(
    PVMKDC_ENCASREPPART pEncAsRepPart,
    PVMKDC_DATA *ppRetAsnData);

DWORD
VmKdcEncodeEncTgsRepPart(
    PVMKDC_ENCTGSREPPART pEncTgsRepPart,
    PVMKDC_DATA *ppRetAsnData);

DWORD
VmKdcBuildAsRep(
    PVMKDC_CONTEXT pContext,
    PVMKDC_PRINCIPAL pClient,
    PVMKDC_PRINCIPAL pServer,
    PVMKDC_KEY pKey,
    PVMKDC_KEY pSessionKey,
    PVMKDC_TICKET pTicket,
    PVMKDC_LAST_REQ last_req,
    DWORD nonce,
    time_t *key_expiration,
    DWORD flags,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PUCHAR caddr,
    PVMKDC_ASREP *ppRetAsRep);

DWORD
VmKdcBuildTgsRep(
    PVMKDC_CONTEXT pContext,
    PVMKDC_PRINCIPAL pClient,
    PVMKDC_PRINCIPAL pServer,
    PVMKDC_KEY pKey,
    PVMKDC_KEY pSubKey,
    PVMKDC_KEY pSessionKey,
    PVMKDC_TICKET pTicket,
    PVMKDC_LAST_REQ last_req,
    DWORD nonce,
    time_t *key_expiration,
    DWORD flags,
    time_t authtime,
    time_t *starttime,
    time_t endtime,
    time_t *renew_till,
    PUCHAR caddr,
    PVMKDC_ASREP *ppRetTgsRep);
