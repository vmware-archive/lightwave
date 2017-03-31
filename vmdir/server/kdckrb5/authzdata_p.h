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
VmKdcFreeAuthzDataElem(
    PVMKDC_AUTHZDATA_ELEM pAuthzDataElem
);

DWORD
VmKdcMakeAuthzDataElem(
    DWORD ad_type,
    PVOID ptr,
    DWORD len,
    PVMKDC_AUTHZDATA_ELEM *ppRetAuthzDataElem
);

VOID
VmKdcFreeAuthzData(
    PVMKDC_AUTHZDATA pAuthzData
);

DWORD
VmKdcAllocateAuthzData(
    PVMKDC_AUTHZDATA *ppRetAuthzData
);

DWORD
VmKdcAddAuthzData(
    PVMKDC_AUTHZDATA pAuthzData,
    PVMKDC_DATA pData,
    VMKDC_AUTHZDATA_TYPE ad_type
);

DWORD
VmKdcCopyAuthzData(
    PVMKDC_AUTHZDATA pAuthzDataSrc,
    PVMKDC_AUTHZDATA *ppAuthzDataDst
);

DWORD
VmKdcEncodeAuthzData(
    PVMKDC_AUTHZDATA pAuthzData,
    PVMKDC_DATA *ppData
);

DWORD
VmKdcDecodeAuthzData(
    PVMKDC_DATA pData,
    PVMKDC_AUTHZDATA *ppAuthzData
);

VOID
VmKdcPrintAuthzData(
    PVMKDC_AUTHZDATA pAuthzData
);

DWORD
VmKdcAllocatePAC(
    PVMKDC_CONTEXT pContex,
    PVMKDC_AUTHZ_PAC *ppPAC
);
 
DWORD
VmKdcAddPACInfoPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    PVMKDC_AUTHZ_PAC_INFO pPACInfo
);

DWORD
VmKdcGetPACInfoPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    VMKDC_AUTHZ_PAC_INFO_TYPE pacInfoType,
    PVMKDC_DATA *ppData
);
 
DWORD
VmKdcSignPAC(
    PVMKDC_AUTHZ_PAC pPAC,
    DWORD authTime,
    PVMKDC_PRINCIPAL pPrinc,
    PVMKDC_KEY pServerKey,
    PVMKDC_KEY pPrivateKey,
    PVMKDC_DATA *ppData
);

DWORD
VmKdcParsePAC(
    PVMKDC_AUTHZ_PAC pPAC,
    PVMKDC_DATA pData
);

DWORD
VmKdcVerifyPAC(
    PVMKDC_AUTHZ_PAC pPAC
);
 
VOID
VmKdcDestroyPAC(
    PVMKDC_AUTHZ_PAC pPAC
);
 
DWORD
VmKdcAllocatePACInfo(
    VMKDC_AUTHZ_PAC_INFO_TYPE pacInfoType,
    PVMKDC_DATA pacInfoData,
    PVMKDC_AUTHZ_PAC_INFO *ppPACInfo
);

VOID
VmKdcDestroyPACInfo(
    PVMKDC_AUTHZ_PAC_INFO pPACInfo
);
