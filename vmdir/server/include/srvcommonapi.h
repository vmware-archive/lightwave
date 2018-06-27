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

/* mainly structures used in high level api implementations */

typedef struct _VMDIR_CA_CERT
{
    /* In the future this may become a chain of certs. */
    PSTR pCert;
    PSTR pCrl;
    PSTR pCN;
    PSTR pSubjectDN;
} VMDIR_CA_CERT, *PVMDIR_CA_CERT;

typedef struct _VMDIR_CA_CERT_ARRAY
{
    UINT32 dwCount;
    PVMDIR_CA_CERT pCACerts;
} VMDIR_CA_CERT_ARRAY, *PVMDIR_CA_CERT_ARRAY;

// certs.c
DWORD
VmDirGetCACerts(
    PVDIR_CONNECTION pConn,
    PCSTR pszDomainName,
    PCSTR pszCACN,
    BOOL  bDetail,
    PVMDIR_CA_CERT_ARRAY* ppCACertificates
    );

VOID
VmDirFreeCACertArray(
    PVMDIR_CA_CERT_ARRAY pArray
    );

//password.c
DWORD
VmDirRefreshPassword(
    PVDIR_CONNECTION pConn,
    BOOL  bRefreshPassword,
    PSTR  *ppszNewPassword
    );
