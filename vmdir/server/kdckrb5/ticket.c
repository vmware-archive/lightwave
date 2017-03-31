/*
 * Copyright 2012-2016 VMware, Inc.  All Rights Reserved.
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

VOID
VmKdcFreeTicket(
    PVMKDC_TICKET pTicket)
{
    if (pTicket)
    {
        VMKDC_SAFE_FREE_PRINCIPAL(pTicket->sname);
        VMKDC_SAFE_FREE_ENCDATA(pTicket->enc_part);
        VMKDC_SAFE_FREE_MEMORY(pTicket);
    }
}

DWORD
VmKdcMakeTicket(
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_ENCDATA pEncData,
    PVMKDC_TICKET *ppRetTicket)
{
    DWORD dwError = 0;
    PVMKDC_TICKET pTicket = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_TICKET), (PVOID*)&pTicket);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCopyPrincipal(pPrincipal, &pTicket->sname);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcCopyEncData(pEncData, &pTicket->enc_part);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetTicket = pTicket;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_TICKET(pTicket);
    }
    return dwError;
}

VOID
VmKdcFreeEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart)
{
    if (pEncTicketPart)
    {
        VMKDC_SAFE_FREE_KEY(pEncTicketPart->key);
        VMKDC_SAFE_FREE_DATA(pEncTicketPart->crealm);
        VMKDC_SAFE_FREE_MEMORY(pEncTicketPart->starttime);
        VMKDC_SAFE_FREE_MEMORY(pEncTicketPart->renew_till);
        VMKDC_SAFE_FREE_PRINCIPAL(pEncTicketPart->cname);
#ifdef VMDIR_ENABLE_PAC
        VMKDC_SAFE_FREE_AUTHZDATA(pEncTicketPart->authorization_data);
#endif
        VMKDC_SAFE_FREE_MEMORY(pEncTicketPart);
    }
}

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
    PVMKDC_ENCTICKETPART *ppRetEncTicketPart)
{
    DWORD dwError = 0;
    PVMKDC_ENCTICKETPART pEncTicketPart = NULL;

    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCTICKETPART),
                                  (PVOID*)&pEncTicketPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* flags */
    pEncTicketPart->flags = flags;
    
    /* key */
    dwError = VmKdcCopyKey(key, &pEncTicketPart->key);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* crealm, cname */
    dwError = VmKdcCopyPrincipal(client, &pEncTicketPart->cname);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* transited */
    pEncTicketPart->transited = NULL; /* TBD */

    /* authtime */
    pEncTicketPart->authtime = authtime;

    /* starttime (optional) */
    if (starttime)
    {
        dwError = VmKdcAllocateMemory(sizeof(*pEncTicketPart->starttime),
                                      (PVOID*)&pEncTicketPart->starttime);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pEncTicketPart->starttime = *starttime;
    }

    /* endtime */
    pEncTicketPart->endtime = endtime;

    /* renew_till (optional) */
    if (renew_till)
    {
        dwError = VmKdcAllocateMemory(sizeof(*pEncTicketPart->renew_till),
                                      (PVOID*)&pEncTicketPart->renew_till);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pEncTicketPart->renew_till = *renew_till;
    }

    /* caddr (optional) */
    if (caddr)
    {
#if 1
        pEncTicketPart->caddr = caddr;
#else
        dwError = VmKdcCopyAddresses(caddr, &pEncTicketPart->caddr);
        BAIL_ON_VMKDC_ERROR(dwError);
#endif
    }

#ifndef VMDIR_ENABLE_PAC
    pEncTicketPart->authorization_data = authorization_data; // Don't know if there is authorization data if there isn't a PAC
#else
    /* authorization_data (optional) */
    if (authorization_data)
    {
        dwError = VmKdcCopyAuthzData(authorization_data,
                                     &pEncTicketPart->authorization_data);
        BAIL_ON_VMKDC_ERROR(dwError);
    }
#endif

    *ppRetEncTicketPart = pEncTicketPart;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCTICKETPART(pEncTicketPart);
    }

    return dwError;
}

VOID
VmKdcPrintEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart)
{
    CHAR timeFmtBuf[32];

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "ENCTICKETPART:");
    VmKdcPrintKey(pEncTicketPart->key);
    VmKdcPrintPrincipal(pEncTicketPart->cname);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "authtime <%s>",
             VmKdcCtimeTS(&pEncTicketPart->authtime, timeFmtBuf));
    if (pEncTicketPart->starttime)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "starttime <%s>",
                 VmKdcCtimeTS(pEncTicketPart->starttime, timeFmtBuf));
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "endtime  <%s>",
             VmKdcCtimeTS(&pEncTicketPart->endtime, timeFmtBuf));
    if (pEncTicketPart->renew_till)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "renew_till <%s>",
             VmKdcCtimeTS(pEncTicketPart->renew_till, timeFmtBuf));
    }
}

VOID
VmKdcTicketFlagsSet(
    VMKDC_TICKET_FLAGS flags,
    void *asnFlags)
{
    TicketFlags *heimFlags = (TicketFlags *)asnFlags;

    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_FORWARDABLE))
    {
        heimFlags->forwardable = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_FORWARDED))
    {
        heimFlags->forwarded = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_PROXIABLE))
    {
        heimFlags->proxiable = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_PROXY))
    {
        heimFlags->proxy = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_MAY_POSTDATE))
    {
        heimFlags->may_postdate = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_POSTDATED))
    {
        heimFlags->postdated = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_INVALID))
    {
        heimFlags->invalid = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_RENEWABLE))
    {
        heimFlags->renewable = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_INITIAL))
    {
        heimFlags->initial = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_PRE_AUTHENT))
    {
        heimFlags->pre_authent = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_HW_AUTHENT))
    {
        heimFlags->hw_authent = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_TRANSITED_POLICY_CHECKED))
    {
        heimFlags->transited_policy_checked = 1;
    }
    if (VMKDC_FLAG_ISSET(flags, VMKDC_TF_OK_AS_DELEGATE))
    {
        heimFlags->ok_as_delegate = 1;
    }
}

DWORD
VmKdcEncodeEncTicketPart(
    PVMKDC_ENCTICKETPART pEncTicketPart,
    PVMKDC_DATA *ppRetData)
{
    DWORD dwError = 0;
    PVMKDC_DATA pAsnData = NULL;
    EncTicketPart heimPart;
    unsigned char *partBufPtr = NULL;
    size_t heimPartLen = 0;
    size_t partBufLen = 0;
    int err = 0;
    int i = 0;

    memset(&heimPart, 0, sizeof(heimPart));

    /* flags */
    VmKdcTicketFlagsSet(pEncTicketPart->flags, &heimPart.flags);

    /* key */

    heimPart.key.keytype = pEncTicketPart->key->type;
    heimPart.key.keyvalue.length = VMKDC_GET_LEN_DATA(pEncTicketPart->key->data);
    heimPart.key.keyvalue.data = VMKDC_GET_PTR_DATA(pEncTicketPart->key->data);

    /* crealm */
    heimPart.crealm = VMKDC_GET_PTR_DATA(pEncTicketPart->cname->realm);

    /* cname */
    heimPart.cname.name_type = pEncTicketPart->cname->type;
    heimPart.cname.name_string.len = pEncTicketPart->cname->numComponents;
    heimPart.cname.name_string.val = malloc(sizeof(heim_general_string *)*heimPart.cname.name_string.len);
    if (!heimPart.cname.name_string.val)
    {
        dwError = ERROR_NO_MEMORY;
        BAIL_ON_VMKDC_ERROR(dwError);
    }
    for (i=0; i<(int)heimPart.cname.name_string.len; i++)
    {
        heimPart.cname.name_string.val[i] = VMKDC_GET_PTR_DATA(pEncTicketPart->cname->components[i]);
    }

    /* transited */
    heimPart.transited.tr_type = 0; /* TBD */
    heimPart.transited.contents.length = 0;
    heimPart.transited.contents.data = NULL;

    /* authtime */
    heimPart.authtime = pEncTicketPart->authtime;

    /* starttime */
    heimPart.starttime = pEncTicketPart->starttime;

    /* endtime */
    heimPart.endtime = pEncTicketPart->endtime;

    /* renew_till */
    heimPart.renew_till = pEncTicketPart->renew_till;

    /* caddr (optional) */
    heimPart.caddr = NULL; /* TBD */

#ifndef VMDIR_ENABLE_PAC
    heimPart.authorization_data = NULL;
#else
    /* authorization_data (optional) */
    if (pEncTicketPart->authorization_data)
    {
        dwError = VmKdcAllocateMemory(sizeof(*heimPart.authorization_data),
                                      (PVOID*)&heimPart.authorization_data);
        BAIL_ON_VMKDC_ERROR(dwError);

        heimPart.authorization_data->len = pEncTicketPart->authorization_data->count;

        dwError = VmKdcAllocateMemory(sizeof(*heimPart.authorization_data->val) * heimPart.authorization_data->len,
                                      (PVOID*)&heimPart.authorization_data->val);
        BAIL_ON_VMKDC_ERROR(dwError);

        for (i=0; i<pEncTicketPart->authorization_data->count; i++)
        {
            heimPart.authorization_data->val[i].ad_type = pEncTicketPart->authorization_data->elem[i]->ad_type;
            heimPart.authorization_data->val[i].ad_data.length = VMKDC_GET_LEN_DATA(pEncTicketPart->authorization_data->elem[i]->ad_data);
            heimPart.authorization_data->val[i].ad_data.data = VMKDC_GET_PTR_DATA(pEncTicketPart->authorization_data->elem[i]->ad_data);
        }
    }
#endif // VMDIR_ENABLE_PAC

    /*
     * Encode the EncTicketPart into Heimdal structure
     */
    ASN1_MALLOC_ENCODE(EncTicketPart,
                       partBufPtr, partBufLen,
                       &heimPart, &heimPartLen,
                       err);
    if (err != 0 || partBufLen != heimPartLen)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    dwError = VmKdcAllocateData(partBufPtr, (int) partBufLen, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetData = pAsnData;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_DATA(pAsnData);
    }
    if (heimPart.cname.name_string.val)
    {
        free(heimPart.cname.name_string.val);
        heimPart.cname.name_string.val = NULL;
    }
    if (partBufPtr)
    {
        free(partBufPtr);
        partBufPtr = NULL;
    }
    if (heimPart.authorization_data)
    {
        VMKDC_SAFE_FREE_MEMORY(heimPart.authorization_data->val);
        VMKDC_SAFE_FREE_MEMORY(heimPart.authorization_data);
    }

    return dwError;
}

DWORD
VmKdcDecodeEncTicketPart(
    PVMKDC_DATA pData,
    PVMKDC_ENCTICKETPART *ppRetEncTicketPart)
{
    DWORD dwError = 0;
    PVMKDC_ENCTICKETPART pEncTicketPart = NULL;
    EncTicketPart heimPart;
    unsigned char *partBufPtr = NULL;
    size_t heimPartLen = 0;
    size_t partBufLen = 0;
    PVMKDC_KEY pKey = NULL;
    PVMKDC_PRINCIPAL pClient = NULL;
#ifdef VMDIR_ENABLE_PAC
    PVMKDC_AUTHZDATA pAuthzData = NULL;
    size_t i = 0;
    PVMKDC_DATA pTmpData = NULL;
#else
    PVOID pAuthzData = NULL;
#endif

    memset(&heimPart, 0, sizeof(heimPart));
    partBufPtr = VMKDC_GET_PTR_DATA(pData);
    partBufLen = VMKDC_GET_LEN_DATA(pData);

    /*
     * Decode the EncTicketPart into a Heimdal EncTicketPart structure.
     */
    decode_EncTicketPart(partBufPtr, partBufLen, &heimPart, &heimPartLen);
    if (heimPartLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* flags */

    /* key */
    dwError = VmKdcMakeKey(heimPart.key.keytype,
                           0,
                           heimPart.key.keyvalue.data,
                           (int) heimPart.key.keyvalue.length,
                           &pKey);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* crealm, cname */
    dwError = VmKdcMakePrincipal(heimPart.crealm,
                                 heimPart.cname.name_string.len,
                                 (PCSTR *)heimPart.cname.name_string.val,
                                 &pClient);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* transited */
    /* authtime */
    /* starttime */
    /* endtime */
    /* renew_till */
    /* caddr */

#ifdef VMDIR_ENABLE_PAC
    /* authorization_data */
    if (heimPart.authorization_data)
    {
        dwError = VmKdcAllocateAuthzData(&pAuthzData);
        BAIL_ON_VMKDC_ERROR(dwError);

        for (i=0; i<heimPart.authorization_data->len; i++)
        {
            dwError = VmKdcAllocateData(
                              heimPart.authorization_data->val->ad_data.data,
                              heimPart.authorization_data->val->ad_data.length,
                              &pTmpData);
            BAIL_ON_VMKDC_ERROR(dwError);

            dwError = VmKdcAddAuthzData(
                              pAuthzData,
                              pTmpData,
                              heimPart.authorization_data->val->ad_type);
            BAIL_ON_VMKDC_ERROR(dwError);

            VMKDC_SAFE_FREE_DATA(pTmpData);
            pTmpData = NULL;
        }
    }
#endif // VMDIR_ENABLE_PAC

    /*
     * Translate the decoded EncTicketPart to a VMKDC_ENCTICKETPART structure.
     */
    dwError = VmKdcMakeEncTicketPart(
                      0, /* flags */
                      pKey, /* key */
                      pClient, /* crealm, cname */
                      NULL, /* transited */
                      heimPart.authtime, /* authtime */
                      heimPart.starttime, /* starttime */
                      heimPart.endtime, /* endtime */
                      heimPart.renew_till, /* renew_till */
                      NULL, /* caddr */
                      pAuthzData, /* authorization_data */
                      &pEncTicketPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetEncTicketPart = pEncTicketPart;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_ENCTICKETPART(pEncTicketPart);
    }
    free_EncTicketPart(&heimPart);
    VMKDC_SAFE_FREE_PRINCIPAL(pClient);
    VMKDC_SAFE_FREE_KEY(pKey);
#ifdef ENABLE_VMDIR_PAC
    VMKDC_SAFE_FREE_AUTHZDATA(pAuthzData);
    VMKDC_SAFE_FREE_DATA(pTmpData);
#endif
    return dwError;
}

VOID
VmKdcPrintTicket(
    PVMKDC_TICKET pTicket)
{
    VmKdcPrintPrincipal(pTicket->sname);
}

DWORD
VmKdcCopyTicket(
    PVMKDC_TICKET pTicket,
    PVMKDC_TICKET *ppRetTicket)
{
    DWORD dwError = 0;
    PVMKDC_TICKET pNewTicket = NULL;

    BAIL_ON_VMKDC_INVALID_POINTER(pTicket, dwError);

    dwError = VmKdcMakeTicket(pTicket->sname,
                              pTicket->enc_part,
                              &pNewTicket);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetTicket = pNewTicket;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_TICKET(pNewTicket);
    }

    return dwError;
}

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
    PVMKDC_TICKET *ppRetTicket)
{
    DWORD dwError = 0;
    PVMKDC_TICKET pTicket = NULL;
    PVMKDC_ENCDATA pEncData = NULL;
    PVMKDC_ENCTICKETPART pEncTicketPart = NULL;
    PVMKDC_DATA pAsnData = NULL;

    /*
     * Initialize a ENC-TICKET-PART structure
     */
    dwError = VmKdcMakeEncTicketPart(
                      flags, /* flags */
                      pSessionKey, /* key */
                      pClient, /* crealm, cname */
                      transited, /* transited */
                      authtime, /* authtime */
                      starttime, /* starttime */
                      endtime, /* endtime */
                      renew_till, /* renew_till */
                      caddr, /* caddr */
                      authorization_data, /* authorization_data */
                      &pEncTicketPart);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * ASN.1 encode the ENC-TICKET-PART
     */
    dwError = VmKdcEncodeEncTicketPart(pEncTicketPart, &pAsnData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Encrypt the ASN.1 encoded ENC-TICKET-PART
     */
    dwError = VmKdcEncryptEncData(
                      pContext,
                      pKey,
                      VMKDC_KU_TICKET,
                      pAsnData,
                      &pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Initialize a ticket structure
     */
    dwError = VmKdcMakeTicket(
                      pServer,
                      pEncData,
                      &pTicket);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetTicket = pTicket;

error:
    if (dwError)
    {
        VMKDC_SAFE_FREE_TICKET(pTicket);
    }
    VMKDC_SAFE_FREE_ENCTICKETPART(pEncTicketPart);
    VMKDC_SAFE_FREE_DATA(pAsnData);
    VMKDC_SAFE_FREE_ENCDATA(pEncData);

    return dwError;
}
