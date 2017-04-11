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



#include "includes.h"

static
VOID
VmKdcFreeKdcRep(
    PVMKDC_KDCREP pKdcRep)
{
    if (pKdcRep)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pKdcRep->cname);
        VMKDC_SAFE_FREE_TICKET(pKdcRep->ticket);
        VMKDC_SAFE_FREE_ENCDATA(pKdcRep->enc_part);
        VMKDC_SAFE_FREE_MEMORY(pKdcRep);
    }
}

VOID
VmKdcFreeAsRep(
    PVMKDC_ASREP pAsRep)
{
    VmKdcFreeKdcRep((PVMKDC_KDCREP)pAsRep);
}

VOID
VmKdcFreeTgsRep(
    PVMKDC_TGSREP pTgsRep)
{
    VmKdcFreeKdcRep((PVMKDC_KDCREP)pTgsRep);
}

static
VOID
VmKdcFreeEncKdcRepPart(
    PVMKDC_ENCKDCREPPART pEncKdcRepPart)
{
    if (pEncKdcRepPart)
    {
        VMKDC_SAFE_FREE_KEY(pEncKdcRepPart->key);
        VMKDC_SAFE_FREE_DATA(pEncKdcRepPart->srealm);
        VMKDC_SAFE_FREE_PRINCIPAL(pEncKdcRepPart->sname);
        VMKDC_SAFE_FREE_MEMORY(pEncKdcRepPart);
    }
}

VOID
VmKdcFreeEncAsRepPart(
    PVMKDC_ENCASREPPART pEncAsRepPart)
{
    VmKdcFreeEncKdcRepPart((PVMKDC_ENCKDCREPPART)pEncAsRepPart);
}

VOID
VmKdcFreeEncTgsRepPart(
    PVMKDC_ENCTGSREPPART pEncTgsRepPart)
{
    VmKdcFreeEncKdcRepPart((PVMKDC_ENCKDCREPPART)pEncTgsRepPart);
}

static
DWORD
VmKdcMakeKdcRep(
    VMKDC_MESSAGE_TYPE msg_type,
    PVMKDC_PRINCIPAL client,
    PVMKDC_TICKET ticket,
    PVMKDC_ENCDATA enc_part,
    PVMKDC_KDCREP *ppRetKdcRep)
{
    DWORD dwError = 0;
    PVMKDC_ASREP pKdcRep = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ASREP), (PVOID*)&pKdcRep);
    BAIL_ON_VMKDC_ERROR(dwError);

#if 0
    /* crealm */
    dwError = VmKdcCopyData(client->realm, &pKdcRep->crealm);
    BAIL_ON_VMKDC_ERROR(dwError);
#endif

    /* cname */
    dwError = VmKdcCopyPrincipal(client, &pKdcRep->cname);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* ticket */
    dwError = VmKdcCopyTicket(ticket, &pKdcRep->ticket);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* enc-part */
    dwError = VmKdcCopyEncData(enc_part, &pKdcRep->enc_part);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* pvno */
    pKdcRep->pvno = 5;

    /* msg-type */
    pKdcRep->msg_type = msg_type;

    /* padata */
    pKdcRep->padata = NULL;

    *ppRetKdcRep = pKdcRep;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ASREP(pKdcRep);
    }
    return dwError;
}

DWORD
VmKdcMakeAsRep(
    PVMKDC_PRINCIPAL client,
    PVMKDC_TICKET ticket,
    PVMKDC_ENCDATA enc_part,
    PVMKDC_ASREP *ppRetAsRep)
{
    DWORD dwError = 0;
    PVMKDC_ASREP pAsRep = NULL;

    dwError = VmKdcMakeKdcRep(VMKDC_MESSAGE_TYPE_KRB_AS_REP,
                              client,
                              ticket,
                              enc_part,
                              (PVMKDC_KDCREP *)&pAsRep);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsRep = pAsRep;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ASREP(pAsRep);
    }
    return dwError;
}

DWORD
VmKdcMakeTgsRep(
    PVMKDC_PRINCIPAL client,
    PVMKDC_TICKET ticket,
    PVMKDC_ENCDATA enc_part,
    PVMKDC_ASREP *ppRetTgsRep)
{
    DWORD dwError = 0;
    PVMKDC_ASREP pTgsRep = NULL;

    dwError = VmKdcMakeKdcRep(VMKDC_MESSAGE_TYPE_KRB_TGS_REP,
                              client,
                              ticket,
                              enc_part,
                              (PVMKDC_KDCREP *)&pTgsRep);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetTgsRep = pTgsRep;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ASREP(pTgsRep);
    }
    return dwError;
}

static
DWORD
VmKdcEncodeKdcRep(
    VMKDC_MESSAGE_TYPE messageType,
    PVMKDC_KDCREP      pKdcRep,
    PVMKDC_DATA        *ppRetData)
{
    PVMKDC_DATA pData = NULL;
    KDC_REP heimRep;
    DWORD dwError = 0;
    PUCHAR repBufPtr = NULL;
    ssize_t  repBufLen = 0;
    DWORD  dwRepBufLen = 0;
    size_t heimRepLen = 0;
    int i = 0;
    int err = 0;

    memset(&heimRep, 0, sizeof(heimRep));

    /* pvno */
    heimRep.pvno = pKdcRep->pvno;

    /* message-type */
    heimRep.msg_type = pKdcRep->msg_type;


#if 0
    /* method-data (optional) */
    if (pKdcRep->padata)
    {
        heimRep.padata = pKdcRep->padata; /* TBD */
    }
#endif

    /* crealm */
#if 1
    heimRep.crealm = (Realm)VMKDC_GET_PTR_DATA(pKdcRep->cname->realm);
#else
    heimRep.crealm = (Realm)VMKDC_GET_PTR_DATA(pKdcRep->crealm);
#endif

    /* cname */
    heimRep.cname.name_type = pKdcRep->cname->type;
    heimRep.cname.name_string.len = pKdcRep->cname->numComponents;
    heimRep.cname.name_string.val = malloc(sizeof(heim_general_string *) * heimRep.cname.name_string.len);
    if (!heimRep.cname.name_string.val)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    for (i=0; i<(int)heimRep.cname.name_string.len; i++)
    {
        heimRep.cname.name_string.val[i] = VMKDC_GET_PTR_DATA(pKdcRep->cname->components[i]);
    }

    /* ticket */
    heimRep.ticket.tkt_vno = 5;
    heimRep.ticket.realm = (Realm)VMKDC_GET_PTR_DATA(pKdcRep->ticket->sname->realm);
    heimRep.ticket.sname.name_type = pKdcRep->ticket->sname->type;
    heimRep.ticket.sname.name_string.len = pKdcRep->ticket->sname->numComponents;
    heimRep.ticket.sname.name_string.val = malloc(sizeof(heim_general_string *) * heimRep.ticket.sname.name_string.len);
    if (!heimRep.ticket.sname.name_string.val)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    for (i=0; i<(int)heimRep.ticket.sname.name_string.len; i++)
    {
        heimRep.ticket.sname.name_string.val[i] = VMKDC_GET_PTR_DATA(pKdcRep->ticket->sname->components[i]);
    }
    heimRep.ticket.enc_part.etype = pKdcRep->enc_part->type;
#if 1
    heimRep.ticket.enc_part.kvno = NULL;
#else
    heimRep.ticket.enc_part.kvno = pKdcRep->enc_part->kvno;
#endif
    heimRep.ticket.enc_part.cipher.length = VMKDC_GET_LEN_DATA(pKdcRep->ticket->enc_part->data);
    heimRep.ticket.enc_part.cipher.data = VMKDC_GET_PTR_DATA(pKdcRep->ticket->enc_part->data);

    /* enc-part */
    heimRep.enc_part.etype = pKdcRep->enc_part->type;
#if 1
    heimRep.enc_part.kvno = NULL;
#else
    heimRep.enc_part.kvno = pKdcRep->enc_part->kvno;
#endif
    heimRep.enc_part.cipher.length = VMKDC_GET_LEN_DATA(pKdcRep->enc_part->data);
    heimRep.enc_part.cipher.data = VMKDC_GET_PTR_DATA(pKdcRep->enc_part->data);

    /*
     * Encode the reply into Heimdal AS_REP structure.
     */
    switch (messageType)
    {
    case VMKDC_MESSAGE_TYPE_KRB_AS_REP:
        ASN1_MALLOC_ENCODE(AS_REP,
                           repBufPtr, repBufLen,
                           &heimRep, &heimRepLen,
                           err);
        break;
    case VMKDC_MESSAGE_TYPE_KRB_TGS_REP:
        ASN1_MALLOC_ENCODE(TGS_REP,
                           repBufPtr, repBufLen,
                           &heimRep, &heimRepLen,
                           err);
        break;
    default:
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
        break;
    }
    if (err != 0 || repBufLen != heimRepLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwRepBufLen = (DWORD) repBufLen;
    dwError = VmKdcAllocateData(repBufPtr, dwRepBufLen, &pData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pData;

error:
    if (dwError)
    {
        VmKdcFreeKdcRep(pKdcRep);
        pKdcRep = NULL;
    }
    if (heimRep.cname.name_string.val)
    {
        free(heimRep.cname.name_string.val);
        heimRep.cname.name_string.val = NULL;
    }
    if (heimRep.ticket.sname.name_string.val)
    {
        free(heimRep.ticket.sname.name_string.val);
        heimRep.ticket.sname.name_string.val = NULL;
    }
    if (repBufPtr)
    {
        free(repBufPtr);
        repBufPtr = NULL;
    }

    return dwError;
}

DWORD
VmKdcEncodeAsRep(
    PVMKDC_ASREP pAsRep,
    PVMKDC_DATA  *ppRetAsnData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;

    dwError = VmKdcEncodeKdcRep(VMKDC_MESSAGE_TYPE_KRB_AS_REP,
                                (PVMKDC_KDCREP)pAsRep,
                                &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsnData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }

    return dwError;
}

DWORD
VmKdcEncodeTgsRep(
    PVMKDC_TGSREP pTgsRep,
    PVMKDC_DATA   *ppRetAsnData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;

    dwError = VmKdcEncodeKdcRep(VMKDC_MESSAGE_TYPE_KRB_TGS_REP,
                                (PVMKDC_KDCREP)pTgsRep,
                                &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsnData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }

    return dwError;
}

DWORD
VmKdcMakeEncKdcRepPart(
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
    PVMKDC_ENCKDCREPPART *ppRetEncKdcRepPart)
{
    DWORD dwError = 0;
    PVMKDC_ENCKDCREPPART pEncKdcRepPart = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCKDCREPPART),
                                  (PVOID*)&pEncKdcRepPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* key */
    dwError = VmKdcCopyKey(key, &pEncKdcRepPart->key);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* last-req */
    pEncKdcRepPart->last_req = NULL; /* TBD */

    /* nonce */
    pEncKdcRepPart->nonce = nonce;

    /* key-expiration (optional) */
    if (key_expiration)
    {
        pEncKdcRepPart->key_expiration = key_expiration;
    }

    /* flags */
    pEncKdcRepPart->flags = flags;

    /* authtime (optional) */
    pEncKdcRepPart->authtime = authtime;

    /* starttime (optional) */
    if (key_expiration)
    {
        pEncKdcRepPart->starttime = starttime;
    }

    /* endtime */
    pEncKdcRepPart->endtime = endtime;

    /* renew-till (optional) */
    if (renew_till)
    {
        pEncKdcRepPart->renew_till = renew_till;
    }

#if 0
    /* srealm */
    pEncKdcRepPart->srealm = srealm;
#endif

    /* sname */
    dwError = VmKdcCopyPrincipal(server, &pEncKdcRepPart->sname);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* caddr (optional) */
    pEncKdcRepPart->caddr = caddr;

    *ppRetEncKdcRepPart = pEncKdcRepPart;

error:
    if (dwError)
    {
        if (pEncKdcRepPart) {
            VmKdcFreeEncKdcRepPart(pEncKdcRepPart);
            pEncKdcRepPart = NULL;
        }
    }
    return dwError;
}

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
    PVMKDC_ENCASREPPART *ppRetEncAsRepPart)
{
    DWORD dwError = 0;
    PVMKDC_ENCASREPPART pEncAsRepPart = NULL;

    dwError = VmKdcMakeEncKdcRepPart(key,
                                     last_req,
                                     nonce,
                                     key_expiration,
                                     flags,
                                     authtime,
                                     starttime,
                                     endtime,
                                     renew_till,
                                     server,
                                     caddr,
                                     (PVMKDC_ENCKDCREPPART*)&pEncAsRepPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetEncAsRepPart = pEncAsRepPart;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCASREPPART(pEncAsRepPart);
    }
    return dwError;
}

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
    PVMKDC_ENCTGSREPPART *ppRetEncTgsRepPart)
{
    DWORD dwError = 0;
    PVMKDC_ENCTGSREPPART pEncTgsRepPart = NULL;

    dwError = VmKdcMakeEncKdcRepPart(key,
                                     last_req,
                                     nonce,
                                     key_expiration,
                                     flags,
                                     authtime,
                                     starttime,
                                     endtime,
                                     renew_till,
                                     server,
                                     caddr,
                                     (PVMKDC_ENCKDCREPPART*)&pEncTgsRepPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetEncTgsRepPart = pEncTgsRepPart;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCTGSREPPART(pEncTgsRepPart);
    }
    return dwError;
}

DWORD
VmKdcEncodeEncKdcRepPart(
    VMKDC_MESSAGE_TYPE messageType,
    PVMKDC_ENCASREPPART pEncKdcRepPart,
    PVMKDC_DATA *ppRetAsnData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;
    EncKDCRepPart heimPart;
    unsigned char *partBufPtr = NULL;
    size_t heimPartLen = 0;
    size_t partBufLen = 0;
    int err = 0;
    int i = 0;

    memset(&heimPart, 0, sizeof(heimPart));

    /* key */
    heimPart.key.keytype = pEncKdcRepPart->key->type;
    heimPart.key.keyvalue.length = VMKDC_GET_LEN_DATA(pEncKdcRepPart->key->data);
    heimPart.key.keyvalue.data = VMKDC_GET_PTR_DATA(pEncKdcRepPart->key->data);

    /* last_req */
    heimPart.last_req.len = 0;
    heimPart.last_req.val = NULL;

    /* nonce */
    heimPart.nonce = pEncKdcRepPart->nonce;

    /* key_expiration */
    if (pEncKdcRepPart->key_expiration)
    {
        heimPart.key_expiration = pEncKdcRepPart->key_expiration;
    }

    /* flags */
    VmKdcTicketFlagsSet(pEncKdcRepPart->flags, &heimPart.flags);

    /* authtime */
    heimPart.authtime = pEncKdcRepPart->authtime;

    /* starttime */
    if (pEncKdcRepPart->starttime)
    {
        heimPart.starttime = pEncKdcRepPart->starttime;
    }

    /* endtime */
    heimPart.endtime = pEncKdcRepPart->endtime;

    /* renew_till */
    if (pEncKdcRepPart->renew_till)
    {
        heimPart.renew_till = pEncKdcRepPart->renew_till;
    }

    /* srealm */
    heimPart.srealm = VMKDC_GET_PTR_DATA(pEncKdcRepPart->sname->realm);

    /* sname */
    heimPart.sname.name_type = pEncKdcRepPart->sname->type;
    heimPart.sname.name_string.len = pEncKdcRepPart->sname->numComponents;
    heimPart.sname.name_string.val = malloc(sizeof(heim_general_string *)*heimPart.sname.name_string.len);
    if (!heimPart.sname.name_string.val)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    for (i=0; i<(int)heimPart.sname.name_string.len; i++)
    {
        heimPart.sname.name_string.val[i] = VMKDC_GET_PTR_DATA(pEncKdcRepPart->sname->components[i]);
    }

    /* caddr (optional) */
    if (pEncKdcRepPart->caddr)
    {
        heimPart.caddr = NULL; /* TBD */
    }

    /* encrypted_pa_data (optional) */
    if (pEncKdcRepPart->encrypted_pa_data)
    {
        heimPart.encrypted_pa_data = NULL; /* TBD */
    }

    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Encode the EncKdcRepPart into Heimdal structure
     */
    switch (messageType)
    {
    case VMKDC_MESSAGE_TYPE_KRB_AS_REP:
        ASN1_MALLOC_ENCODE(EncASRepPart,
                           partBufPtr, partBufLen,
                           &heimPart, &heimPartLen,
                           err);
        break;
    case VMKDC_MESSAGE_TYPE_KRB_TGS_REP:
        ASN1_MALLOC_ENCODE(EncTGSRepPart,
                           partBufPtr, partBufLen,
                           &heimPart, &heimPartLen,
                           err);
        break;
    default:
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
        break;
    }
    if (err != 0 || partBufLen != heimPartLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(partBufPtr, (DWORD) partBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsnData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }
    if (heimPart.sname.name_string.val)
    {
        free(heimPart.sname.name_string.val);
        heimPart.sname.name_string.val = NULL;
    }
    if (partBufPtr) {
        free(partBufPtr);
        partBufPtr = NULL;
    }

    return dwError;
}

DWORD
VmKdcEncodeEncAsRepPart(
    PVMKDC_ENCASREPPART pEncAsRepPart,
    PVMKDC_DATA *ppRetAsnData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;

    dwError = VmKdcEncodeEncKdcRepPart(VMKDC_MESSAGE_TYPE_KRB_AS_REP,
                                       (PVMKDC_ENCKDCREPPART)pEncAsRepPart,
                                       &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsnData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }

    return dwError;
}

DWORD
VmKdcEncodeEncTgsRepPart(
    PVMKDC_ENCTGSREPPART pEncTgsRepPart,
    PVMKDC_DATA *ppRetAsnData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;

    dwError = VmKdcEncodeEncKdcRepPart(VMKDC_MESSAGE_TYPE_KRB_TGS_REP,
                                       (PVMKDC_ENCKDCREPPART)pEncTgsRepPart,
                                       &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsnData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }

    return dwError;
}

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
    PVMKDC_ASREP *ppRetAsRep)
{
    DWORD dwError = 0;
    PVMKDC_ASREP pAsRep = NULL;
    PVMKDC_DATA pAsnData = NULL;
    PVMKDC_ENCDATA pEncData = NULL;
    PVMKDC_ENCASREPPART pEncAsRepPart = NULL;

    /*
     * Initializate an AS-REP-PART structure
     */
    dwError = VmKdcMakeEncAsRepPart(pSessionKey,
                                    last_req,
                                    nonce,
                                    key_expiration,
                                    flags,
                                    authtime,
                                    starttime,
                                    endtime,
                                    renew_till,
                                    pServer,
                                    caddr,
                                    &pEncAsRepPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * ASN.1 encode the AS-REP-PART
     */
    dwError = VmKdcEncodeEncAsRepPart(pEncAsRepPart, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Encrypt the ASN.1 encoded AS-REP-PART
     */
    dwError = VmKdcEncryptEncData(pContext,
                                  pKey,
                                  VMKDC_KU_AS_REP_ENC_PART,
                                  pAsnData,
                                  &pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Initialize a KDC-REP structure
     */
    dwError = VmKdcMakeAsRep(pClient,
                             pTicket,
                             pEncData,
                             &pAsRep);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetAsRep = pAsRep;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ASREP(pAsRep);
    }
    VMKDC_SAFE_FREE_ENCASREPPART(pEncAsRepPart);
    VMKDC_SAFE_FREE_ENCDATA(pEncData);
    VMKDC_SAFE_FREE_DATA(pAsnData);

    return dwError;
}

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
    PVMKDC_ASREP *ppRetTgsRep)
{
    DWORD dwError = 0;
    PVMKDC_ASREP pTgsRep = NULL;
    PVMKDC_DATA pAsnData = NULL;
    PVMKDC_ENCDATA pEncData = NULL;
    PVMKDC_ENCTGSREPPART pEncTgsRepPart = NULL;

    /*
     * Initializate an TGS-REP-PART structure
     */
    dwError = VmKdcMakeEncTgsRepPart(pSessionKey,
                                     last_req,
                                     nonce,
                                     key_expiration,
                                     flags,
                                     authtime,
                                     starttime,
                                     endtime,
                                     renew_till,
                                     pServer,
                                     caddr,
                                     &pEncTgsRepPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * ASN.1 encode the TGS-REP-PART
     */
    dwError = VmKdcEncodeEncTgsRepPart(pEncTgsRepPart, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Encrypt the ASN.1 encoded TGS-REP-PART
     */
    if (pSubKey)
    {
        dwError = VmKdcEncryptEncData(pContext,
                                      pSubKey,
                                      VMKDC_KU_TGS_REP_ENC_PART_SUB_KEY,
                                      pAsnData,
                                      &pEncData);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    else
    {
        dwError = VmKdcEncryptEncData(pContext,
                                      pKey,
                                      VMKDC_KU_TGS_REP_ENC_PART_SESSION,
                                      pAsnData,
                                      &pEncData);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Initialize a TGS-REP structure
     */
    dwError = VmKdcMakeTgsRep(pClient,
                              pTicket,
                              pEncData,
                              &pTgsRep);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetTgsRep = pTgsRep;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_TGSREP(pTgsRep);
    }
    VMKDC_SAFE_FREE_ENCTGSREPPART(pEncTgsRepPart);
    VMKDC_SAFE_FREE_ENCDATA(pEncData);
    VMKDC_SAFE_FREE_DATA(pAsnData);

    return dwError;
}
