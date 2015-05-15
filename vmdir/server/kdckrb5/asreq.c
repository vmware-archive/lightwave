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
VmKdcFreeAsReq(
    PVMKDC_ASREQ pAsReq)
{
    VmKdcFreeKdcReq((PVMKDC_KDCREQ)pAsReq);
}

DWORD
VmKdcDecodeAsReq(
    PVMKDC_DATA    pAsnData,
    PVMKDC_ASREQ *ppRetAsReq)
{
    DWORD dwError = 0;
    PVMKDC_ASREQ pAsReq = NULL;

    dwError = VmKdcDecodeKdcReq(VMKDC_MESSAGE_TYPE_KRB_AS_REQ,
                               pAsnData, (PVMKDC_ASREQ *)&pAsReq);
    BAIL_ON_VMKDC_ERROR(dwError);

error:
    if (dwError) {
        VmKdcFreeAsReq(pAsReq);
        pAsReq = NULL;
    }
    *ppRetAsReq = pAsReq;
    return dwError;
}

DWORD
VmKdcVerifyAsReqPaData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_ASREQ pAsReq,
    PVMKDC_KEY pKey)
{
    DWORD dwError = 0;
    PVMKDC_METHOD_DATA pMethodData = NULL;
    PVMKDC_PADATA pPaData = NULL;
    PVMKDC_PAENCTSENC paEncTsEnc = NULL;
    PVMKDC_ENCDATA pEncData = NULL;
    PVMKDC_DATA pData = NULL;
    int iDelta = 0;
    time_t kdcTime = 0;

    pMethodData = pAsReq->padata;
    if (!pMethodData)
    {
        dwError = ERROR_NO_PREAUTH;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Find the ENC-TIMESTAMP preauthentication data
     */
    dwError = VmKdcFindPaData(VMKDC_PADATA_ENC_TIMESTAMP, pMethodData, &pPaData);
    if (dwError)
    {
        dwError = ERROR_NO_PREAUTH;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Decode the ENC-PA-TS-ENC
     */
    dwError = VmKdcDecodeEncData(pPaData->data, &pEncData);
    BAIL_ON_VMKDC_ERROR(dwError);

    /*
     * Decrypt the ENC-PA-TS-ENC
     */
    dwError = VmKdcDecryptEncData(pContext,
                                  pKey,
                                  VMKDC_KU_PA_ENC_TIMESTAMP,
                                  pEncData,
                                  &pData);
    if (dwError)
    {
        dwError = ERROR_FAILED_PREAUTH;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Decode the ENC-TIMESTAMP
     */
    dwError = VmKdcDecodePaEncTsEnc(pData, &paEncTsEnc);
    if (dwError)
    {
        dwError = ERROR_FAILED_PREAUTH;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Verify that the timestamp is within clock skew of the KDC time
     */
    kdcTime = time(NULL);
    iDelta = abs((unsigned)kdcTime - (unsigned)paEncTsEnc->patimestamp);
    if (iDelta > pContext->pGlobals->iClockSkew)
    {
        dwError = ERROR_FAILED_PREAUTH;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

error:
    VMKDC_SAFE_FREE_PAENCTSENC(paEncTsEnc);
    VMKDC_SAFE_FREE_ENCDATA(pEncData);
    VMKDC_SAFE_FREE_DATA(pData);

    return dwError;
}

VOID
VmKdcPrintAsReq(
    PVMKDC_ASREQ asReq)
{
    CHAR timeFmtBuf[32];

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintAsReq: pvno   <%d>",
             asReq->pvno);
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintAsReq: msg_type <%d>",
             asReq->msg_type);
    if (asReq->req_body.realm) {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintAsReq: realm  <%s>",
                 VMKDC_GET_PTR_DATA(asReq->req_body.realm));
    }
    VmKdcPrintPrincipal(asReq->req_body.cname);
    VmKdcPrintPrincipal(asReq->req_body.sname);
    if (asReq->req_body.from)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                 "VmKdcPrintAsReq: from     <%s>",
                 VmKdcCtimeTS(asReq->req_body.from, timeFmtBuf));
    }
    if (asReq->req_body.till)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL,
                 "VmKdcPrintAsReq: till     <%s>",
                 VmKdcCtimeTS(asReq->req_body.till, timeFmtBuf));
    }
    if (asReq->req_body.rtime)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintAsReq: rtime    <%s>",
                 VmKdcCtimeTS(asReq->req_body.rtime, timeFmtBuf));
    }
    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintAsReq: nonce  <%d>",
             asReq->req_body.nonce);
    VmKdcPrintEncTypes(&asReq->req_body.etype);
}
