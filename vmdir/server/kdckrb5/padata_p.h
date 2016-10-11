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
VmKdcFreePaData(
    PVMKDC_PADATA pPaData);

VOID
VmKdcFreeMethodData(
    PVMKDC_METHOD_DATA pMethodData);

DWORD
VmKdcAllocateMethodData(
    DWORD length,
    PVMKDC_METHOD_DATA *ppRetMethodData);

DWORD
VmKdcEncodeMethodData(
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_DATA *ppRetData);

DWORD
VmKdcMakePaData(
    VMKDC_PADATA_TYPE type,
    DWORD length,
    PUCHAR contents,
    PVMKDC_PADATA *ppRetPaData);

DWORD
VmKdcFindPaData(
    VMKDC_PADATA_TYPE type,
    PVMKDC_METHOD_DATA pMethodData,
    PVMKDC_PADATA *ppRetPaData);

VOID
VmKdcPrintPaData(
    PVMKDC_PADATA pPaData);

VOID
VmKdcPrintMethodData(
    PVMKDC_METHOD_DATA pMethodData);
