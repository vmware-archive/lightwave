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

/*
 * Test helper method which creates db certificate array object with two certificates.
 */
VOID
TestCreateCertificates(
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

VOID
Test_LwCACreateCertificate_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate = NULL;

    dwError = LwCACreateCertificate("dummyCert", &pCertificate);
    assert_int_equal(dwError, 0);

    assert_string_equal(pCertificate, "dummyCert");

    LwCAFreeCertificate(pCertificate);
}

VOID
Test_LwCACreateCertificate_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate = NULL;

    dwError = LwCACreateCertificate(NULL, &pCertificate);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificate);
}

VOID
Test_LwCACreateCertArray_Valid(
    VOID **state
    )
{
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;

    TestCreateCertificates(&pCertificates);
    assert_non_null(pCertificates);

    assert_int_equal(pCertificates->dwCount, 2);
    assert_non_null(pCertificates->ppCertificates);
    assert_string_equal(pCertificates->ppCertificates[0], "dummyCert1");
    assert_string_equal(pCertificates->ppCertificates[1], "dummyCert2");

    LwCAFreeCertificates(pCertificates);
}

VOID
Test_LwCACreateCertArray_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PSTR *ppCertificates = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;

    dwError = LwCAAllocateMemory(1 * sizeof(PSTR), (PVOID*)&ppCertificates);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("dummyCert1", &ppCertificates[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateCertArray(NULL, 0, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);

    dwError = LwCACreateCertArray(ppCertificates, 0, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);

    dwError = LwCACreateCertArray(ppCertificates, 2, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);

    LWCA_SAFE_FREE_MEMORY(ppCertificates[0]);
    LWCA_SAFE_FREE_MEMORY(ppCertificates);
}

VOID
Test_LwCACreateKey_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pKey = NULL;
    BYTE testKey[20] = "testKey";

    dwError = LwCACreateKey(testKey, 20, &pKey);
    assert_int_equal(dwError, 0);

    assert_non_null(pKey);
    assert_memory_equal(pKey->pData, testKey, 20);

    LwCAFreeKey(pKey);
}

VOID
Test_LwCACreateKey_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pKey = NULL;
    BYTE testKey[20] = "testKey";

    dwError = LwCACreateKey(testKey, 0, &pKey);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pKey);

    dwError = LwCACreateKey(NULL, 10, &pKey);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pKey);
}

VOID
Test_LwCACopyCertArray_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pOrigCertificates = NULL;
    PLWCA_CERTIFICATE_ARRAY pCopyCertificates = NULL;

    TestCreateCertificates(&pOrigCertificates);
    assert_non_null(pOrigCertificates);

    dwError = LwCACopyCertArray(pOrigCertificates, &pCopyCertificates);
    assert_int_equal(dwError, 0);

    assert_int_equal(pCopyCertificates->dwCount, 2);
    assert_non_null(pCopyCertificates->ppCertificates);
    assert_string_equal(pCopyCertificates->ppCertificates[0], "dummyCert1");
    assert_string_equal(pCopyCertificates->ppCertificates[1], "dummyCert2");

    LwCAFreeCertificates(pCopyCertificates);
    LwCAFreeCertificates(pOrigCertificates);
}

VOID
Test_LwCACopyCertArray_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;

    dwError = LwCACopyCertArray(NULL, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);
}

VOID
Test_LwCACopyKey_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pOrigKey = NULL;
    PLWCA_KEY pKey = NULL;
    BYTE testKey[20] = "testKey";

    dwError = LwCACreateKey(testKey, 20, &pOrigKey);
    assert_int_equal(dwError, 0);

    dwError = LwCACopyKey(pOrigKey, &pKey);
    assert_int_equal(dwError, 0);

    assert_non_null(pKey);
    assert_memory_equal(pKey->pData, testKey, 20);

    LwCAFreeKey(pOrigKey);
    LwCAFreeKey(pKey);
}

VOID
Test_LwCACopyKey_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_KEY pKey = NULL;

    dwError = LwCACopyKey(NULL, &pKey);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pKey);
}
