/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

typedef struct _LWCA_REST_INT_CA_SPEC
{
    PSTR                        pszCAId;
    PSTR                        pszParentCAId;
    PLWCA_INT_CA_REQ_DATA       pIntCAReqData;
    PLWCA_CERT_VALIDITY         pCertValidity;
} LWCA_REST_INT_CA_SPEC, *PLWCA_REST_INT_CA_SPEC;

typedef struct _LWCA_REST_SIGN_CERT_SPEC
{
    PSTR                        pszCSR;
    PLWCA_CERT_VALIDITY         pCertValidity;
    LWCA_SIGNING_ALGORITHM      signAlgorithm;
} LWCA_REST_SIGN_CERT_SPEC, *PLWCA_REST_SIGN_CERT_SPEC;

DWORD
LwCARestGetCertValidityInput(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_CERT_VALIDITY*        ppCertValidity
    );

DWORD
LwCARestGetIntCAInputSpec(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_REST_INT_CA_SPEC*     ppIntCASpec
    );

DWORD
LwCARestGetSignCertInputSpec(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_REST_SIGN_CERT_SPEC*  ppSignCertSpec
    );

DWORD
LwCARestGetCertificateInput(
    PLWCA_JSON_OBJECT           pJsonBody,
    PLWCA_CERTIFICATE*          ppCert;
    );

VOID
LwCARestFreeIntCAInputSpec(
    PLWCA_REST_INT_CA_SPEC      pIntCASpec
    );

VOID
LwCARestFreeSignCertInputSpec(
    PLWCA_REST_SIGN_CERT_SPEC   pSignCertSpec
    );
