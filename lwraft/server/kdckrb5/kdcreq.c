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

VOID
VmKdcFreeKdcReq(
    PVMKDC_KDCREQ pKdcReq)
{
    if (pKdcReq) {
        VMKDC_SAFE_FREE_METHOD_DATA(pKdcReq->padata);
        VMKDC_SAFE_FREE_DATA(pKdcReq->req_body.realm);
        VMKDC_SAFE_FREE_PRINCIPAL(pKdcReq->req_body.cname);
        VMKDC_SAFE_FREE_PRINCIPAL(pKdcReq->req_body.sname);
        VMKDC_SAFE_FREE_MEMORY(pKdcReq->req_body.etype.type);
        VMKDC_SAFE_FREE_MEMORY(pKdcReq->req_body.from);
        VMKDC_SAFE_FREE_MEMORY(pKdcReq->req_body.till);
        VMKDC_SAFE_FREE_MEMORY(pKdcReq->req_body.rtime);
        VMKDC_SAFE_FREE_MEMORY(pKdcReq);
    }
}

static
VOID
_VmKdcOptionsGet(
    KDCOptions *heimOptions,
    DWORD *kdcOptions)
{
    if (heimOptions->forwardable)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_FORWARDABLE);
    }
    if (heimOptions->forwarded)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_FORWARDED);
    }
    if (heimOptions->proxiable)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_PROXIABLE);
    }
    if (heimOptions->proxy)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_PROXY);
    }
    if (heimOptions->allow_postdate)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_ALLOW_POSTDATE);
    }
    if (heimOptions->postdated)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_POSTDATED);
    }
    if (heimOptions->renewable)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_RENEWABLE);
    }
    if (heimOptions->disable_transited_check)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_DISABLE_TRANSITED_CHECK);
    }
    if (heimOptions->renewable_ok)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_RENEWABLE_OK);
    }
    if (heimOptions->enc_tkt_in_skey)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_ENC_TKT_IN_SKEY);
    }
    if (heimOptions->renew)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_RENEW);
    }
    if (heimOptions->validate)
    {
        VMKDC_FLAG_SET(*kdcOptions, VMKDC_KO_VALIDATE);
    }
}

DWORD
VmKdcDecodeKdcReq(
    VMKDC_MESSAGE_TYPE messageType,
    PVMKDC_DATA    pData,
    PVMKDC_KDCREQ *ppRetKdcReq)
{
    PVMKDC_KDCREQ pKdcReq = NULL;
    AS_REQ heimReq = {0};
    size_t heimReqLen = 0;
    DWORD dwError = 0;
    PUCHAR reqBufPtr = NULL;
    DWORD  reqBufLen = 0;
    unsigned int i = 0;

    reqBufPtr = VMKDC_GET_PTR_DATA(pData);
    reqBufLen = VMKDC_GET_LEN_DATA(pData);

    /*
     * Decode the request into Heimdal AS_REQ structure.
     */
    switch (messageType)
    {
    case VMKDC_MESSAGE_TYPE_KRB_AS_REQ:
        decode_AS_REQ(reqBufPtr, reqBufLen, &heimReq, &heimReqLen);
        break;
    case VMKDC_MESSAGE_TYPE_KRB_TGS_REQ:
        decode_TGS_REQ(reqBufPtr, reqBufLen, &heimReq, &heimReqLen);
        break;
    default:
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
        break;
    }
    if (heimReqLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Translate the decoded request to a VMKDC_KDCREQ structure.
     */
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_KDCREQ), (PVOID*)&pKdcReq);
    BAIL_ON_VMKDC_ERROR(dwError);
    memset(pKdcReq, 0, sizeof(VMKDC_KDCREQ));

    /* pvno */
    pKdcReq->pvno = heimReq.pvno;

    /* message-type */
    pKdcReq->msg_type = heimReq.msg_type;

    /* method-data (optional) */
    if (heimReq.padata)
    {
        dwError = VmKdcAllocateMethodData(heimReq.padata->len,
                                          &pKdcReq->padata);
        BAIL_ON_VMKDC_ERROR(dwError);

        for (i=0; i<heimReq.padata->len; i++)
        {
            dwError = VmKdcMakePaData(heimReq.padata->val[i].padata_type,
                                      (DWORD) heimReq.padata->val[i].padata_value.length,
                                      heimReq.padata->val[i].padata_value.data,
                                      &pKdcReq->padata->padata[i]);
            BAIL_ON_VMKDC_ERROR(dwError);
        }
    }

    /* kdc-options */
    _VmKdcOptionsGet(&heimReq.req_body.kdc_options, &pKdcReq->req_body.kdc_options);

    /* cname (optional) */
    if (heimReq.req_body.cname)
    {
        dwError = VmKdcMakePrincipal(heimReq.req_body.realm,
                                     heimReq.req_body.cname->name_string.len,
                                     (PCSTR *)heimReq.req_body.cname->name_string.val,
                                     &pKdcReq->req_body.cname);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* realm */
    dwError = VmKdcAllocateDataString(heimReq.req_body.realm,
                                      &pKdcReq->req_body.realm);
    BAIL_ON_VMKDC_ERROR(dwError);

    /* sname (optional) */
    if (heimReq.req_body.sname)
    {
        dwError = VmKdcMakePrincipal(heimReq.req_body.realm,
                                     heimReq.req_body.sname->name_string.len,
                                     (PCSTR *)heimReq.req_body.sname->name_string.val,
                                     &pKdcReq->req_body.sname);
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /* from (optional) */
    if (heimReq.req_body.from)
    {
        dwError = VmKdcAllocateMemory(sizeof(time_t),
                                  (PVOID*)&pKdcReq->req_body.from);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pKdcReq->req_body.from = *((time_t *)heimReq.req_body.from);
    }

    /* till (optional) */
    if (heimReq.req_body.till)
    {
        dwError = VmKdcAllocateMemory(sizeof(time_t),
                                  (PVOID*)&pKdcReq->req_body.till);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pKdcReq->req_body.till = *((time_t *)heimReq.req_body.till);
    }

    /* rtime (optional) */
    if (heimReq.req_body.rtime)
    {
        dwError = VmKdcAllocateMemory(sizeof(time_t),
                                  (PVOID*)&pKdcReq->req_body.rtime);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pKdcReq->req_body.rtime = *((time_t *)heimReq.req_body.rtime);
    }

    /* nonce */
    pKdcReq->req_body.nonce = heimReq.req_body.nonce;

    /* etype */
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_ENCTYPE) * heimReq.req_body.etype.len,
                                  (PVOID*)&pKdcReq->req_body.etype.type);
    BAIL_ON_VMKDC_ERROR(dwError);
    pKdcReq->req_body.etype.count = heimReq.req_body.etype.len;
    for (i=0; i<heimReq.req_body.etype.len; i++)
    {
        pKdcReq->req_body.etype.type[i] = heimReq.req_body.etype.val[i];
    }

    /* addresses */
#if 0
    if (pHeimReq->req_body.addresses)
    {
        pKdcReq->req_body.addresses = pHeimReq->req_body.addresses;
    }
#endif

    /* enc-authorization-data */
#if 0
    if (pHeimReq->req_body.enc_authorization_data)
    {
        pKdcReq->req_body.enc_authorization_data = pHeimReq->req_body.authorization_data;
    }
#endif

    /* additional-tickets */
#if 0
    if (pHeimReq->req_body.additional_tickets)
    {
        pKdcReq->req_body.additional_tickets = pHeimReq->req_body.additional_tickets;
    }
#endif

error:
    if (dwError)
    {
        VmKdcFreeKdcReq(pKdcReq);
        pKdcReq = NULL;
    }
    free_AS_REQ(&heimReq);

    *ppRetKdcReq = pKdcReq;
    return dwError;
}
