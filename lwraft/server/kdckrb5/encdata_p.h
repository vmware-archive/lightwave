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
VmKdcFreeEncData(
    PVMKDC_ENCDATA pEncData);

DWORD
VmKdcMakeEncData(
    VMKDC_ENCTYPE type,
    DWORD kvno,
    PUCHAR contents,
    DWORD length,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcEncryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_DATA pInData,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcDecryptEncData(
    PVMKDC_CONTEXT pContext,
    PVMKDC_KEY pKey,
    VMKDC_KEY_USAGE keyUsage,
    PVMKDC_ENCDATA pEncData,
    PVMKDC_DATA *ppRetData);

DWORD
VmKdcDecodeEncData(
    PVMKDC_DATA pData,
    PVMKDC_ENCDATA *ppRetEncData);

DWORD
VmKdcCopyEncData(
    PVMKDC_ENCDATA pEncData,
    PVMKDC_ENCDATA *ppRetEncData);

VOID
VmKdcPrintEncData(
    PVMKDC_ENCDATA pEncData);
