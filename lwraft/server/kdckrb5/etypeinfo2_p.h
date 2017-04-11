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
VmKdcFreeEtypeInfo2Entry(
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry);

VOID
VmKdcFreeEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2);

DWORD
VmKdcMakeEtypeInfo2Entry(
    VMKDC_ENCTYPE etype,
    PVMKDC_SALT salt,
    PVMKDC_DATA s2kparams,
    PVMKDC_ETYPE_INFO2_ENTRY *ppRetEtypeInfo2Entry);

DWORD
VmKdcEncodeEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2,
    PVMKDC_DATA *ppRetData);

VOID
VmKdcPrintEtypeInfo2Entry(
    PVMKDC_ETYPE_INFO2_ENTRY pEtypeInfo2Entry);

VOID
VmKdcPrintEtypeInfo2(
    PVMKDC_ETYPE_INFO2 pEtypeInfo2);
