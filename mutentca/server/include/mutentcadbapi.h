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

#ifndef __MutentCADBAPI_H__
#define __MutentCADBAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

DWORD
LwCADbInitCtx(
    PLWCA_JSON_OBJECT pConfig
    );

VOID
LwCADbFreeCtx(
   VOID
   );

DWORD
LwCADbAddCA(
    PCSTR                       pcszCAId,
    PLWCA_DB_CA_DATA            pCAData,
    PLWCA_DB_ROOT_CERT_DATA     pCARootCertData
    );

DWORD
LwCADbGetCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        *ppCAData
    );

DWORD
LwCADbUpdateCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData
    );

DWORD
LwCADbLockCA(
    PCSTR   pcszCAId,
    PSTR    *ppszUuid
    );

DWORD
LwCADbUnlockCA(
    PCSTR   pcszCAId,
    PCSTR   pcszUuid
    );

DWORD
LwCADbAddCACert(
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    );

DWORD
LwCADbGetCACerts(
    LWCA_DB_GET_CERTS_FLAGS         certsToGet,
    PCSTR                           pcszCAId,       //OPTIONAL: only if requesting all or active CA cert
    PCSTR                           pcszSKI,        //OPTIONAL: only if requesting ca cert via SKI
    PLWCA_DB_ROOT_CERT_DATA_ARRAY   *ppCACerts
    );

DWORD
LwCADbUpdateCACert(
    PCSTR                       pcszCAId,
    PLWCA_DB_ROOT_CERT_DATA     pCACert
    );

DWORD
LwCADbLockCACert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PSTR    *ppszUuid
    );

DWORD
LwCADbUnlockCACert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PCSTR   pcszUuid
    );

DWORD
LwCADbAddCert(
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

DWORD
LwCADbGetCerts(
    LWCA_DB_GET_CERTS_FLAGS     certsToGet,
    PCSTR                       pcszCAId,           //OPTIONAL: only if requesting CA or revoked certs
    PCSTR                       pcszSerialNumber,   //OPTIONAL: only if requesting cert via serial number
    PCSTR                       pcszAKI,            //OPTIONAL: only if requesting revoked certs
    PLWCA_DB_CERT_DATA_ARRAY    *ppCertDataArray
    );

DWORD
LwCADbUpdateCert(
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    );

DWORD
LwCADbLockCert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PSTR    *ppszUuid
    );

DWORD
LwCADbUnlockCert(
    PCSTR   pcszCAId,
    PCSTR   pcszSerialNumber,
    PCSTR   pcszUuid
    );

#ifdef __cplusplus
}
#endif

#endif // __MutentCADBAPI_H__
