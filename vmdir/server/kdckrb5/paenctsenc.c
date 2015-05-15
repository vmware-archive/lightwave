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
VmKdcFreePaEncTsEnc(
    PVMKDC_PAENCTSENC pPaEncTsEnc)
{
    if (pPaEncTsEnc) {
        if (pPaEncTsEnc->pausec)
        {
            VMKDC_SAFE_FREE_MEMORY(pPaEncTsEnc->pausec);
        }
        VMKDC_SAFE_FREE_MEMORY(pPaEncTsEnc);
    }
}

DWORD
VmKdcDecodePaEncTsEnc(
    PVMKDC_DATA pData,
    PVMKDC_PAENCTSENC *ppRetPaEncTsEnc)
{
    PVMKDC_PAENCTSENC pPaEncTsEnc = NULL;
    PA_ENC_TS_ENC heimPa = {0};
    size_t heimPaLen = 0;
    DWORD dwError = 0;
    PUCHAR paBufPtr = NULL;
    DWORD  paBufLen = 0;

    paBufPtr = VMKDC_GET_PTR_DATA(pData);
    paBufLen = VMKDC_GET_LEN_DATA(pData);

    decode_PA_ENC_TS_ENC(paBufPtr, paBufLen, &heimPa, &heimPaLen);
    if (heimPaLen <= 0)
    {
        dwError = ERROR_PROTOCOL;
        BAIL_ON_VMKDC_ERROR(dwError);
    }

    /*
     * Translate the decoded request to a VMKDC_PAENCTSENC structure.
     */
    dwError = VmKdcAllocateMemory(sizeof(VMKDC_PAENCTSENC), (PVOID*)&pPaEncTsEnc);
    BAIL_ON_VMKDC_ERROR(dwError);
    memset(pPaEncTsEnc, 0, sizeof(VMKDC_PAENCTSENC));

    /* patimestamp */

    pPaEncTsEnc->patimestamp = heimPa.patimestamp;

    /* pausec (optional) */
    if (heimPa.pausec)
    {
        dwError = VmKdcAllocateMemory(sizeof(*pPaEncTsEnc->pausec),
                                      (PVOID*)&pPaEncTsEnc->pausec);
        BAIL_ON_VMKDC_ERROR(dwError);
        *pPaEncTsEnc->pausec =  *heimPa.pausec;
    }

    *ppRetPaEncTsEnc = pPaEncTsEnc;

error:
    if (dwError)
    {
        VmKdcFreePaEncTsEnc(pPaEncTsEnc);
        pPaEncTsEnc = NULL;
    }
    free_PA_ENC_TS_ENC(&heimPa);

    return dwError;
}

VOID
VmKdcPrintPaEncTsEnc(
    PVMKDC_PAENCTSENC pPaEncTsEnc)
{
    CHAR timeFmtBuf[32];

    VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintPaEncTsEnc: patimestamp <%s>",
             VmKdcCtimeTS(&pPaEncTsEnc->patimestamp, timeFmtBuf));
    if (pPaEncTsEnc->pausec)
    {
        VMDIR_LOG_VERBOSE(VMDIR_LOG_MASK_ALL, "VmKdcPrintPaEncTsEnc: pausec <%d>",
                 pPaEncTsEnc->pausec);
    }
}
