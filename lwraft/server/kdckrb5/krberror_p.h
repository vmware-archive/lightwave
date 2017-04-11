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
VmKdcFreeKrbError(
    IN PVMKDC_KRB_ERROR pKrbError);

DWORD
VmKdcMakeKrbError(
    IN int                       pvno,
    IN OPTIONAL time_t           *ctime,
    IN time_t                    stime,
    IN VMKDC_KRB_ERR             error_code,
    IN OPTIONAL PCSTR            crealm,
    IN OPTIONAL PVMKDC_PRINCIPAL cname,
    IN PCSTR                     realm,
    IN PVMKDC_PRINCIPAL          sname,
    IN OPTIONAL PVMKDC_DATA      e_text,
    IN OPTIONAL PVMKDC_DATA      e_data,
    OUT PVMKDC_KRB_ERROR         *ppRetKrbError);

DWORD
VmKdcBuildKrbErrorEData(
    IN  PVMKDC_KEY pKey,
    OUT PVMKDC_DATA *ppRetData);

DWORD
VmKdcEncodeKrbError(
    IN PVMKDC_KRB_ERROR pKrbError,
    OUT PVMKDC_DATA     *ppRetData);

DWORD
VmKdcBuildKrbError(
    IN int                       pvno,
    IN OPTIONAL time_t           *ctime,
    IN time_t                    stime,
    IN VMKDC_KRB_ERR             error_code,
    IN OPTIONAL PCSTR            crealm,
    IN OPTIONAL PVMKDC_PRINCIPAL cname,
    IN PCSTR                     realm,
    IN PVMKDC_PRINCIPAL          sname,
    IN OPTIONAL PVMKDC_DATA      e_text,
    IN OPTIONAL PVMKDC_DATA      e_data,
    OUT PVMKDC_DATA              *ppRetData);

VOID
VmKdcPrintKrbError(
    IN PVMKDC_KRB_ERROR pKrbError);
