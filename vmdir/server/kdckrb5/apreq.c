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
VmKdcFreeApReq(
    PVMKDC_APREQ pApReq)
{
    if (pApReq) {
        VMKDC_SAFE_FREE_TICKET(pApReq->ticket);
        VMKDC_SAFE_FREE_ENCDATA(pApReq->authenticator);
        VMKDC_SAFE_FREE_MEMORY(pApReq);
    }
}

DWORD
VmKdcDecodeApReq(
    PVMKDC_DATA pData,
    PVMKDC_APREQ *ppRetApReq)
{
    PVMKDC_APREQ pApReq = NULL;
    AP_REQ heimReq = {0};
    size_t heimReqLen = 0;
    DWORD dwError = 0;
    PUCHAR reqBufPtr = NULL;
    DWORD  reqBufLen = 0;
    PVMKDC_PRINCIPAL sname = NULL;
    PVMKDC_ENCDATA enc_part = NULL;

    reqBufPtr = VMKDC_GET_PTR_DATA(pData);
    reqBufLen = VMKDC_GET_LEN_DATA(pData);

    decode_AP_REQ(reqBufPtr, reqBufLen, &heimReq, &heimReqLen);
    if (heimReqLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Translate the decoded request to a VMKDC_APREQ structure.
     */
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_APREQ), (PVOID*)&pApReq);
    BAIL_ON_VMKDC_ERROR(dwError);
    memset(pApReq, 0, sizeof(VMKDC_APREQ));

    /* pvno */
    pApReq->pvno = heimReq.pvno;

    /* message-type */
    pApReq->msg_type = heimReq.msg_type;

    /* ap-options */
#if 0
    pApReq->ap_options = heimReq.ap_options;
#endif

    /* ticket */

    dwError = VmKdcMakePrincipal(heimReq.ticket.realm,
                                 heimReq.ticket.sname.name_string.len,
                                 (PCSTR *)heimReq.ticket.sname.name_string.val,
                                 &sname);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcMakeEncData(heimReq.ticket.enc_part.etype,
                               (heimReq.ticket.enc_part.kvno ? *heimReq.ticket.enc_part.kvno : 0),
                               heimReq.ticket.enc_part.cipher.data,
                               (DWORD) heimReq.ticket.enc_part.cipher.length,
                               &enc_part);
    BAIL_ON_VMKDC_ERROR(dwError);

    dwError = VmKdcMakeTicket(sname, enc_part, &pApReq->ticket);
    BAIL_ON_VMKDC_ERROR(dwError);
    
    /* authenticator */

    dwError = VmKdcMakeEncData(heimReq.authenticator.etype,
                               (heimReq.authenticator.kvno ? *heimReq.authenticator.kvno : 0),
                               heimReq.authenticator.cipher.data,
                               (DWORD) heimReq.authenticator.cipher.length,
                               &pApReq->authenticator);
    BAIL_ON_VMKDC_ERROR(dwError);

    *ppRetApReq = pApReq;

error:
    if (dwError)
    {
        VmKdcFreeApReq(pApReq);
        pApReq = NULL;
    }
    VMKDC_SAFE_FREE_PRINCIPAL(sname);
    VMKDC_SAFE_FREE_ENCDATA(enc_part);
    free_AP_REQ(&heimReq);

    return dwError;
}

VOID
VmKdcPrintApReq(
    PVMKDC_APREQ apReq)
{
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintApReq: pvno   <%d>",
             apReq->pvno);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintApReq: msg_type <%d>",
             apReq->msg_type);
    VmKdcPrintTicket(apReq->ticket);
    VmKdcPrintEncData(apReq->authenticator);
}
