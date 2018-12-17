/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

DWORD
VmwCaAcquireOidcToken(
    PVMW_CA_PARAMS pCaParams,
    PCSTR          pcszScope,
    PSTR*          ppszAccessToken
    );

DWORD
VmwCaGenerateCertSigningRequest(
    PVMW_CA_PARAMS pCaParams,
    PVMCA_CSR*     ppCSR
    );

DWORD
VmwCaMakeRestRequest(
    PCSTR   pcszUrl,
    PCSTR   pcszHeader,
    PCSTR   pcszPostData,
    PCSTR   pcszRequestType,
    BOOL    bInsecure,
    PSTR*   ppszOut
    );

VOID
VmwCaFreeParams(
    PVMW_CA_PARAMS pParams
    );

DWORD
VmwCaWriteToFile(
    PCSTR pcszFilePath,
    PCSTR pcszContents
    );

DWORD
VmwCaReadFromFile(
    PCSTR pcszFilePath,
    PSTR* ppszContents
    );

BOOL
VmwCaIsIPV6AddrFormat(
    PCSTR   pszAddr
    );

DWORD
VmwGetKeySize(
    PCSTR   pcszKeySize,
    size_t* pKeySize
    );
