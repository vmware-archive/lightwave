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
VmKdcFreeTgsReq(
    PVMKDC_TGSREQ pTgsReq)
{
    VmKdcFreeKdcReq((PVMKDC_KDCREQ)pTgsReq);
}

DWORD
VmKdcDecodeTgsReq(
    PVMKDC_DATA    pAsnData,
    PVMKDC_TGSREQ *ppRetTgsReq)
{
    DWORD dwError = 0;
    PVMKDC_ASREQ pTgsReq = NULL;

    dwError = VmKdcDecodeKdcReq(VMKDC_MESSAGE_TYPE_KRB_TGS_REQ,
                                pAsnData, (PVMKDC_TGSREQ *)&pTgsReq);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    if (dwError) {
        VmKdcFreeTgsReq(pTgsReq);
        pTgsReq = NULL;
    }
    *ppRetTgsReq = pTgsReq;
    return dwError;
}

VOID
VmKdcPrintTgsReq(
    PVMKDC_ASREQ tgsReq)
{
    CHAR timeFmtBuf[32];

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintTgsReq: pvno   <%d>",
             tgsReq->pvno);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintTgsReq: msg_type <%d>",
             tgsReq->msg_type);
    if (tgsReq->req_body.realm) {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintTgsReq: realm  <%s>",
                 VMKDC_GET_PTR_DATA(tgsReq->req_body.realm));
    }
    VmKdcPrintPrincipal(tgsReq->req_body.cname);
    VmKdcPrintPrincipal(tgsReq->req_body.sname);
    if (tgsReq->req_body.from)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                 "VmKdcPrintTgsReq: t_from     <%s>",
                 VmKdcCtimeTS(tgsReq->req_body.from, timeFmtBuf));
    }
    if (tgsReq->req_body.till)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                 "VmKdcPrintTgsReq: t_till     <%s>",
                 VmKdcCtimeTS(tgsReq->req_body.till, timeFmtBuf));
    }
    if (tgsReq->req_body.rtime)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintTgsReq: t_rtime    <%s>",
                 VmKdcCtimeTS(tgsReq->req_body.rtime, timeFmtBuf));
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintTgsReq: nonce  <%d>",
             tgsReq->req_body.nonce);
    VmKdcPrintEncTypes(&tgsReq->req_body.etype);
}
