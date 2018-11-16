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

#include "includes.h"

#define LWCA_DB_CONFIG "./test-mutentcadb-config/test-mutentcadb-config.json"
#define TEST_CA_ID "testId"
#define TEST_PARENT_CA_ID "testParentId"

static
VOID
_CreateCertificates(
    PLWCA_CERTIFICATE_ARRAY  *ppCertArray
    );

int
Test_LwCADbInitCtx(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pJson =  NULL;

    dwError = LwCAJsonLoadObjectFromFile(LWCA_DB_CONFIG, &pJson);
    assert_int_equal(dwError, 0);

    dwError = LwCADbInitCtx(pJson);
    assert_int_equal(dwError, 0);

    LwCAJsonCleanupObject(pJson);
    return 0;
}

VOID
Test_LwCADbInitCtx_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pJson =  NULL;

    dwError = LwCAJsonLoadObjectFromFile(LWCA_DB_CONFIG, &pJson);
    assert_int_equal(dwError, 0);

    dwError = LwCADbInitCtx(pJson);
    assert_int_equal(dwError, LWCA_DB_ALREADY_INITIALIZED);

    LwCAJsonCleanupObject(pJson);
}

VOID
Test_LwCADbAddCA(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    PLWCA_KEY pEncryptedPrivateKey = NULL;
    BYTE testKey1[20] = "testKey1";

    dwError = LwCACreateKey(testKey1, 20, &pEncryptedPrivateKey);
    assert_int_equal(dwError, 0);

    _CreateCertificates(&pCertArray);
    assert_non_null(pCertArray);
    assert_non_null(pEncryptedPrivateKey);

    dwError = LwCADbCreateCAData("cn=CA",
                                pCertArray,
                                pEncryptedPrivateKey,
                                "10001",
                                "20181025201010.542",
                                "20191025201010.542",
                                "some-tenant-info",
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData, NULL);
    assert_int_equal(dwError, 0);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData, TEST_PARENT_CA_ID);
    assert_int_equal(dwError, 0);

    LwCAFreeKey(pEncryptedPrivateKey);
    LwCAFreeCertificates(pCertArray);
    LwCADbFreeCAData(pCAData);
}

VOID
Test_LwCADbAddCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData1 = NULL;
    PLWCA_DB_CA_DATA pCAData2 = NULL;
    PLWCA_DB_CA_DATA pCAData3 = NULL;
    PLWCA_DB_CA_DATA pCAData4 = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    PLWCA_KEY pEncryptedPrivateKey = NULL;
    BYTE testKey1[20] = "testKey1";

    dwError = LwCACreateKey(testKey1, 20, &pEncryptedPrivateKey);
    assert_int_equal(dwError, 0);

    _CreateCertificates(&pCertArray);
    assert_non_null(pCertArray);
    assert_non_null(pEncryptedPrivateKey);

    dwError = LwCADbCreateCAData("cn=CA",
                                pCertArray,
                                pEncryptedPrivateKey,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData1);
    assert_int_equal(dwError, 0);

    dwError = LwCADbCreateCAData(NULL,
                                pCertArray,
                                pEncryptedPrivateKey,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData2);
    assert_int_equal(dwError, 0);

    dwError = LwCADbCreateCAData("cn=CA",
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData3);
    assert_int_equal(dwError, 0);

    dwError = LwCADbCreateCAData("cn=CA",
                                NULL,
                                pEncryptedPrivateKey,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData4);
    assert_int_equal(dwError, 0);

    dwError = LwCADbAddCA(TEST_CA_ID, NULL, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCA(NULL, pCAData1, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData2, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData3, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData4, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCA(TEST_CA_ID, pCAData1, NULL);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LwCAFreeKey(pEncryptedPrivateKey);
    LwCAFreeCertificates(pCertArray);
    LwCADbFreeCAData(pCAData1);
    LwCADbFreeCAData(pCAData2);
    LwCADbFreeCAData(pCAData3);
    LwCADbFreeCAData(pCAData4);
}

VOID
Test_LwCADbAddCertData(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbAddCertData(TEST_CA_ID, pCertData);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(pCertData);
}

VOID
Test_LwCADbAddCertData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbAddCertData(NULL, pCertData);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbAddCertData(TEST_CA_ID, pCertData);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_MEMORY(pCertData);
}

VOID
Test_LwCADbCheckCA(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bCAExists = FALSE;

    dwError = LwCADbCheckCA(TEST_CA_ID, &bCAExists);
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCADbCheckCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bCAExists = FALSE;

    dwError = LwCADbCheckCA(NULL, &bCAExists);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbCheckCA(TEST_CA_ID, &bCAExists);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);
}

VOID
Test_LwCADbCheckCertData(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bExists = FALSE;

    dwError = LwCADbCheckCertData(TEST_CA_ID, "1234", &bExists);
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCADbCheckCertData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bExists = FALSE;

    dwError = LwCADbCheckCertData(NULL, NULL, &bExists);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbCheckCertData(TEST_CA_ID, "1234", &bExists);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);
}

VOID
Test_LwCADbGetCA(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pDbCAData = NULL;

    dwError = LwCADbGetCA(TEST_CA_ID, &pDbCAData);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(pDbCAData);
}

VOID
Test_LwCADbGetCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pDbCAData = NULL;

    dwError = LwCADbGetCA(NULL, &pDbCAData);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbGetCA(TEST_CA_ID, &pDbCAData);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_MEMORY(pDbCAData);
}

VOID
Test_LwCADbGetCACertificates(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;

    dwError = LwCADbGetCACertificates(TEST_CA_ID, &pCertArray);
    assert_int_equal(dwError, 0);

    LwCAFreeCertificates(pCertArray);
}

VOID
Test_LwCADbGetCACertificates_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;

    dwError = LwCADbGetCACertificates(NULL, &pCertArray);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbGetCACertificates(TEST_CA_ID, &pCertArray);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LwCAFreeCertificates(pCertArray);
}

VOID
Test_LwCADbGetCertData(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;

    dwError = LwCADbGetCertData(TEST_CA_ID, &pCertDataArray);
    assert_int_equal(dwError, 0);

    LwCADbFreeCertDataArray(pCertDataArray);
}

VOID
Test_LwCADbGetCertData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA_ARRAY pCertDataArray = NULL;

    dwError = LwCADbGetCertData(NULL, &pCertDataArray);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbGetCertData(TEST_CA_ID, &pCertDataArray);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LwCADbFreeCertDataArray(pCertDataArray);
}

VOID
Test_LwCADbGetCACRLNumber(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszCRLNumber = NULL;

    dwError = LwCADbGetCACRLNumber(TEST_CA_ID, &pszCRLNumber);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
}

VOID
Test_LwCADbGetCACRLNumber_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszCRLNumber = NULL;

    dwError = LwCADbGetCACRLNumber(NULL, &pszCRLNumber);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbGetCACRLNumber(TEST_CA_ID, &pszCRLNumber);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_STRINGA(pszCRLNumber);
}

VOID
Test_LwCADbGetParentCAId(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszParentCAId = NULL;

    dwError = LwCADbGetParentCAId(TEST_CA_ID, &pszParentCAId);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
}

VOID
Test_LwCADbGetParentCAId_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR pszParentCAId = NULL;

    dwError = LwCADbGetParentCAId(NULL, &pszParentCAId);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbGetParentCAId(TEST_CA_ID, &pszParentCAId);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_STRINGA(pszParentCAId);
}

VOID
Test_LwCADbUpdateCA(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pDbCAData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CA_DATA), (PVOID*)&pDbCAData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbUpdateCA(TEST_CA_ID, pDbCAData);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(pDbCAData);
}

VOID
Test_LwCADbUpdateCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pDbCAData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CA_DATA), (PVOID*)&pDbCAData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbUpdateCA(NULL, pDbCAData);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbUpdateCA(TEST_CA_ID, pDbCAData);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_MEMORY(pDbCAData);
}

VOID
Test_LwCADbUpdateCAStatus(
    VOID **state
    )
{
    DWORD dwError = 0;

    dwError = LwCADbUpdateCAStatus(TEST_CA_ID, LWCA_CA_STATUS_INACTIVE);
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCADbUpdateCAStatus_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;

    dwError = LwCADbUpdateCAStatus(NULL, LWCA_CA_STATUS_INACTIVE);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbUpdateCAStatus(TEST_CA_ID, LWCA_CA_STATUS_INACTIVE);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);
}

VOID
Test_LwCADbUpdateCertData(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbUpdateCertData(TEST_CA_ID, pCertData);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(pCertData);
}

VOID
Test_LwCADbUpdateCertData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCAAllocateMemory(sizeof(LWCA_DB_CERT_DATA), (PVOID*)&pCertData);
    assert_int_equal(dwError, 0);

    dwError = LwCADbUpdateCertData(NULL, pCertData);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbUpdateCertData(TEST_CA_ID, pCertData);
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);

    LWCA_SAFE_FREE_MEMORY(pCertData);
}

VOID
Test_LwCADbUpdateCACRLNumber(
    VOID **state
    )
{
    DWORD dwError = 0;

    dwError = LwCADbUpdateCACRLNumber(TEST_CA_ID, "1234");
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCADbUpdateCACRLNumber_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;

    dwError = LwCADbUpdateCACRLNumber(NULL, "1234");
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    dwError = LwCADbUpdateCACRLNumber(TEST_CA_ID, "1234");
    assert_int_equal(dwError, LWCA_DB_NOT_INITIALIZED);
}

int
Test_LwCADbFreeCtx(
    VOID **state
    )
{
    LwCADbFreeCtx();
    return 0;
}

VOID
Test_LwCADbCAData(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;
    PLWCA_KEY pEncryptedPrivateKey = NULL;
    BYTE testKey1[20] = "testKey1";

    dwError = LwCACreateKey(testKey1, 20, &pEncryptedPrivateKey);
    assert_int_equal(dwError, 0);

    _CreateCertificates(&pCertArray);
    assert_non_null(pCertArray);
    assert_non_null(pEncryptedPrivateKey);

    dwError = LwCADbCreateCAData("cn=CA",
                                pCertArray,
                                pEncryptedPrivateKey,
                                "10001",
                                "20181025201010.542",
                                "20191025201010.542",
                                "some-tenant-info",
                                LWCA_CA_STATUS_ACTIVE,
                                &pCAData);
    assert_int_equal(dwError, 0);

    assert_non_null(pCAData);
    assert_string_equal(pCAData->pszSubjectName, "cn=CA");

    assert_non_null(pCAData->pCertificates);
    assert_int_equal(pCAData->pCertificates->dwCount, 2);
    assert_non_null(pCAData->pCertificates->ppCertificates);
    assert_string_equal(pCAData->pCertificates->ppCertificates[0], "dummyCert1");
    assert_string_equal(pCAData->pCertificates->ppCertificates[1], "dummyCert2");

    assert_non_null(pCAData->pEncryptedPrivateKey);
    assert_memory_equal(pCAData->pEncryptedPrivateKey->pData, testKey1, 20);

    assert_string_equal(pCAData->pszCRLNumber, "10001");
    assert_string_equal(pCAData->pszLastCRLUpdate, "20181025201010.542");
    assert_string_equal(pCAData->pszNextCRLUpdate, "20191025201010.542");

    assert_int_equal(pCAData->status, LWCA_CA_STATUS_ACTIVE);

    LwCAFreeKey(pEncryptedPrivateKey);
    LwCAFreeCertificates(pCertArray);
    LwCADbFreeCAData(pCAData);
}

VOID
Test_LwCADbCAData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;

    dwError = LwCADbCreateCAData("cn=CA",
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                LWCA_CA_STATUS_ACTIVE,
                                NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
}


VOID
Test_LwCADbCertData(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCADbCreateCertData("dummySerial",
                                    "-1",
                                    "-1",
                                    1,
                                    "-1",
                                    LWCA_CERT_STATUS_ACTIVE,
                                    &pCertData);
    assert_int_equal(dwError, 0);

    assert_non_null(pCertData);
    assert_string_equal(pCertData->pszSerialNumber, "dummySerial");
    assert_string_equal(pCertData->pszTimeValidFrom, "-1");
    assert_string_equal(pCertData->pszTimeValidTo, "-1");
    assert_int_equal(pCertData->revokedReason, 1);
    assert_string_equal(pCertData->pszRevokedDate, "-1");
    assert_int_equal(pCertData->status, LWCA_CERT_STATUS_ACTIVE);

    LwCADbFreeCertData(pCertData);
}

VOID
Test_LwCADbCertData_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CERT_DATA pCertData = NULL;

    dwError = LwCADbCreateCertData(NULL,
                                    "-1",
                                    "-1",
                                    0,
                                    "-1",
                                    LWCA_CERT_STATUS_ACTIVE,
                                    &pCertData);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertData);

    LwCADbFreeCertData(pCertData);
}

/*
 * Helper method which creates db certificate array object with two certificates.
 */
static
VOID
_CreateCertificates(
    PLWCA_CERTIFICATE_ARRAY  *ppCertArray
    )
{
    DWORD dwError = 0;
    PSTR *ppCertificates = NULL;

    dwError = LwCAAllocateMemory(2 * sizeof(PSTR), (PVOID*)&ppCertificates);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("dummyCert1", &ppCertificates[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("dummyCert2", &ppCertificates[1]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateCertArray(ppCertificates, 2, ppCertArray);
    assert_int_equal(dwError, 0);

    assert_non_null(ppCertArray);

    LWCA_SAFE_FREE_MEMORY(ppCertificates[1]);
    LWCA_SAFE_FREE_MEMORY(ppCertificates[0]);
    LWCA_SAFE_FREE_MEMORY(ppCertificates);
}
