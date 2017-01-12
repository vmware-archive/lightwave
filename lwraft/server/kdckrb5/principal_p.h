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
VmKdcFreePrincipal(
    PVMKDC_PRINCIPAL pPrincipal);

DWORD
VmKdcAllocatePrincipal(
    PVMKDC_PRINCIPAL *ppRetPrincipal);

DWORD
VmKdcCopyPrincipal(
    PVMKDC_PRINCIPAL pPrincipal,
    PVMKDC_PRINCIPAL *ppRetPrincipal);

DWORD
VmKdcMakePrincipal(
    PCSTR realmName,
    DWORD numComponents,
    PCSTR *components,
    PVMKDC_PRINCIPAL *ppRetPrincipal);

DWORD
VmKdcParsePrincipalName(
    PVMKDC_CONTEXT pContext,
    PCSTR pPrincipalName,
    PVMKDC_PRINCIPAL *ppRetPrincipal);

DWORD
VmKdcUnparsePrincipalName(
    PVMKDC_PRINCIPAL pPrincipal,
    PSTR *ppRetPrincipalName);

DWORD
VmKdcGetPrincipalName(
    PVMKDC_PRINCIPAL pPrincipal,
    PSTR *ppRetPrincipalName);

VOID
VmKdcPrintPrincipal(
    PVMKDC_PRINCIPAL pPrincipal);
