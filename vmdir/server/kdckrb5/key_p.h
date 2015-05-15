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
VmKdcFreeKey(
    PVMKDC_KEY pKey);

VOID
VmKdcZeroKey(
    PVMKDC_KEY pKey);

DWORD
VmKdcMakeKey(
    VMKDC_KEYTYPE  type,
    DWORD          kvno,
    PUCHAR         keyData,
    DWORD          keyLen,
    PVMKDC_KEY    *ppRetKey);

DWORD
VmKdcRandomKey(
    PVMKDC_CONTEXT pContext,
    VMKDC_KEYTYPE type,
    PVMKDC_KEY *ppRetKey);

DWORD
VmKdcCopyKey(
    PVMKDC_KEY src,
    PVMKDC_KEY *dst);

VOID
VmKdcPrintKey(
    PVMKDC_KEY pKey);

DWORD
VmKdcDecodeMasterKey(
    PBYTE asn1MasterKey,
    DWORD asn1MasterKeyLen,
    PVMKDC_KEY *ppMasterKey);
