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
VmKdcFreeAsReq(
    PVMKDC_ASREQ pAsReq);

DWORD
VmKdcDecodeAsReq(
    PVMKDC_DATA pAsnData,
    PVMKDC_ASREQ *ppRetAsReq);

DWORD
VmKdcVerifyAsReqPaData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_ASREQ pAsReq,
    PVMKDC_KEY pKey);

VOID
VmKdcPrintAsReq(
    PVMKDC_ASREQ pAsReq);
