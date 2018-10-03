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

#define TEST_ROOT_CA_ID "RootCA"
#define TEST_ROOT_CA_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "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\n" \
        "DCU=\n" \
        "-----END CERTIFICATE-----"
#define TEST_ROOT_CA_KEY "-----BEGIN RSA PRIVATE KEY-----\n" \
        "Proc-Type: 4,ENCRYPTED\n" \
        "DEK-Info: AES-256-CBC,7D346D2EF644F5DAA43295EA56BE6250\n" \
        "\n" \
        "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\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_ROOT_CA_PASSPHRASE "test"
#define TEST_CLIENT_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "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\n" \
        "-----END CERTIFICATE-----"
#define TEST_CLIENT_KEY "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEowIBAAKCAQEAwYQE8omkJkVOGQ0bcASC3azM16Rge+rwdpwVf0P0nO8JboVU\n" \
        "yZx/I9ESezZqot4tuesL+Tf/Nsr5LQCKsg5Qh6UUXLCLcg3hBUqpOPG1AAPSC1HQ\n" \
        "N0pf2mocT9wGz1TZxJ8UpVr+xIB8Zv3hV9uoRxd2kJWkUiVmKidjBzPegeh1X4rQ\n" \
        "7pX54Sh5tWaZnV1HRQWRIaBWk88n1h6LGI51uwFMoJmi88ViQz5Uj5n5fhRSsSqV\n" \
        "/gP54Q84yxJDgP3nYksy/10jBc/+GxYqGWRnSv2id9Sr2OG/Nkrdj7e2g7ZQqhed\n" \
        "klRcJ2oBmkZnWutg+1gwxWASDtSA1YYRDBMw4wIDAQABAoIBACi7AteRxO0RNUjs\n" \
        "i1PefzTtRAg3Xt92KbqtS5EH2pqVbWTHx3rP56rAPKWWZId3QCSbpl+EO8Fqo1un\n" \
        "e4nSnXyPcGO5/P+6kBwvBMMpjLE+HJ2pUKvyiY5NOzxL0VsePt+1pADR0EHH0WdU\n" \
        "FHQLlbWw6JGyDW4E1SYWOC+x6SlqR7BKBcYlv5pfhNHG+h33x6ZwH+lL43o9AmxG\n" \
        "n9G/qU3nLbrcZZnI5v/M/vAi0leAc2P1aEY/Fia0mMfNYTyZe0rImEoWTFqTTg4/\n" \
        "I3TsoYT6uFXMCli3Bf9WJDBxets4wV8H+K0aTDnWc12hc1qbfptcAPWPGEOyTIcJ\n" \
        "091m1YECgYEA++NvPp90VGRSZQ27Jz5cloUPCggZjEBJ+XkeMG6G5d/FaX/0Awz2\n" \
        "McYdsCNVi+5RSg2LTzllh1PVPqv4HfF1W9G2Lo29ONI8gPlgHRWD/Ey+ogPGToLn\n" \
        "Mn7CGqGex6XX0upeFANlbww9m6MeiwVrED6EBLC5ZY7RHzpzIt+7ssMCgYEAxKyp\n" \
        "sY8YCaDuS9yj6fJJhbB9OJHUDgbAhRVO0af/aeSnIHHuoFf4tZ3UBJLWQQ00X9Id\n" \
        "ntFAMtMa4c1UQOquS7e0xSVMzOZEOyUakh4d/N9cHVu8tAdNFgIZ02ZAz3PB008V\n" \
        "x6xZd1Rb7HDi4L/50NoHbPhnUUAWFZnqrZpYZ2ECgYEAqBbv56RU5vRDn7f4Coot\n" \
        "Ey0cCh+5nMRSGS7gHIXFc6VOgcanOVnu+OwyZ5ZMwiJ5EKfVLTsqK5KgHUB5RNIa\n" \
        "9PkPUCW5/i8dvRutLftrpnQ4SrmmHkJwsluwgEEWSji7u742pbhWejJ68l96RF+L\n" \
        "QL3XL6IXNlSF7Qb+SmlvQi0CgYA5auMbuFffL8GCGkcsXJTEV/Mm2cQb2DLKZ58c\n" \
        "LDyv8JYuLbTp0OUGOK6WKzwrv6wjsVb+b76bV/BILcbEMP9zkY1P5QTX6P3QepOY\n" \
        "RvygP6FH3OlpcZw6qaBajEatifDzPOpx4co4wUMY8xf5X3KSW6TBVn/rQ5du9QGD\n" \
        "b0UR4QKBgG7B+34YKHkEFsSdYiuIX5GO+pwY7uluCdf2LvO85Gyv6/rSHKv4tY+o\n" \
        "fYfuYcow6msSegAoEOwVStqYsyaLQvFzYij/SwFT/kM5cNQUuNkdeNv1DK9XbRBy\n" \
        "qhei7nNlk+ZsilNkQd6jXhg9OT3llbhsMkuoa7jql/hbMup+PyAq\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_CLIENT_PASSPHRASE ""
#define TEST_DUMMY_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "MIID5TCCAs2gAwIBAgIJAONKhmr803trMA0GCSqGSIb3DQEBCwUAMGkxIDAeBgNV\n" \
        "BAMMF0NBLERDPWx3LXRlc3Rkb20sREM9Y29tMQswCQYDVQQGEwJVUzE4MDYGA1UE\n" \
        "CgwvbHd0LXByYXNvb250LWktMDRhNmE5ZDQxYWU2MGU5NTAubHctdGVzdGRvbS5j\n" \
        "b20wHhcNMTgwOTI4MTczMDI5WhcNMjgwOTIyMTcxMTU4WjBGMUQwQgYDVQQDDDtt\n" \
        "dXRlbnRjYS1wb3N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5j\n" \
        "b20tc2VydmljZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALZKdjuu\n" \
        "EKvpnsr4qU7IPX+lJo6eoSCTX/dZs6gs+pyXshkkfLR1um+vcRNqiJF8SjsRflXv\n" \
        "Si6tzE2Cf2i8k6Tu2EVBhXvrZRVDnjlxVlUam5K9nLonHqT+w77xhcNugHJU6wet\n" \
        "xm/kFOUVqQkCn9ehRNivFSAAQgkUgTQS1b9AeMFFKq6rvjwi/vqWm5KaRbVMNYj7\n" \
        "2T+MT57i5mJpJVgzHRMoMBmEhYNC7kos0Qdej5Ud8o4UuAjRktsN0f+f9Pm9r4ku\n" \
        "4NC2p31B1N5od9fM4Jtj2fWbSoTNMhctJ1GBhCicuPgX3DEu1ASVZnIx9k0CDE/n\n" \
        "uUHSkSwBfwPQYd0CAwEAAaOBsjCBrzALBgNVHQ8EBAMCBeAwHQYDVR0OBBYEFIc1\n" \
        "FeBfbMLmSQNkVffWJUwlzKNhMB8GA1UdIwQYMBaAFK69Ssu1s5g6ixVIN/luNvxm\n" \
        "CyODMGAGCCsGAQUFBwEBBFQwUjBQBggrBgEFBQcwAoZEaHR0cHM6Ly9sd3QtcHJh\n" \
        "c29vbnQtaS0wNGE2YTlkNDFhZTYwZTk1MC5sdy10ZXN0ZG9tLmNvbS9hZmQvdmVj\n" \
        "cy9zc2wwDQYJKoZIhvcNAQELBQADggEBADzpitGGg+nWAmvjU/jxYvRU3GFZjG25\n" \
        "Mot99e+LloRrbpPS3YIVLdNkKG7uU2ndXebJmwG+McgY1ZCgk31FcBFkM/eeeWvI\n" \
        "soR6dcK63nDO7t9tSygYSriTZnMbGhsKCK9bXbPt8RrjmZN8rnRgaXKf1J9vgy6X\n" \
        "SRGV8DAEMzURA7fFPK2R36CyKaBTS+7MfQ9NRKuBU8wETzVWqgFBRYlVzYuVdcCW\n" \
        "v59zouvgwjoQ/qB3LKh4y2w7aPfPXRrI3P3WZZi+xZSsdK2uzjrU+Wh1GNCEKdDz\n" \
        "+D1z/13oztwrHTqAdq8BflLG9gpS1jZ+tVqJTiL2r2TBP27fC/9rNVM=\n" \
        "-----END CERTIFICATE-----"

// defines output value of __wrap_LwCADbCheckCA
BOOLEAN bCAExists = FALSE;

PLWCA_REQ_CONTEXT pReqCtx = NULL;

int
TestLwCACreateRequestContext(
    VOID **state
    )
{
    DWORD dwError = 0;

    if (!pReqCtx)
    {
        dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID*)&pReqCtx);
        assert_int_equal(dwError, 0);
    }
    return 0;
}

int
TestLwCAFreeRequestContext(
    VOID **state
    )
{
    if (pReqCtx)
    {
        LWCA_SAFE_FREE_MEMORY(pReqCtx);
    }
    return 0;
}

DWORD
__wrap_LwCADbAddCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData,
    PCSTR                   pcszParentCA
    )
{
    assert_string_equal(pcszCAId, TEST_ROOT_CA_ID);
    assert_non_null(pCAData);
    return mock();
}

DWORD
__wrap_LwCADbCheckCA(
    PCSTR                   pcszCAId,
    PBOOLEAN                pbExists
    )
{
    assert_non_null(pcszCAId);
    assert_non_null(pbExists);
    *pbExists = bCAExists;
    return mock();
}

DWORD
__wrap_LwCADbGetCACertificates(
    PCSTR                      pcszCAId,
    PLWCA_CERTIFICATE_ARRAY    *ppCertArray
    )
{
    DWORD dwError = 0;
    PSTR *ppCertificates = NULL;

    dwError = LwCAAllocateMemory(1 * sizeof(PSTR), (PVOID*)&ppCertificates);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA("dummyCert", &ppCertificates[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateCertArray(ppCertificates, 1, ppCertArray);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(ppCertificates[0]);
    LWCA_SAFE_FREE_MEMORY(ppCertificates);

    return mock();
}

VOID
Test_LwCACreateRootCA_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate = NULL;

    will_return(__wrap_LwCADbAddCA, 0);
    bCAExists = FALSE;
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCACreateCertificate(TEST_ROOT_CA_CERTIFICATE, &pCertificate);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, 0);

    LwCAFreeCertificate(pCertificate);
}

VOID
Test_LwCACreateRootCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate1 = NULL;
    PLWCA_CERTIFICATE pCertificate2 = NULL;

    // Testcase 1: Certificate is not a CA cert
    bCAExists = FALSE;
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCACreateCertificate(TEST_CLIENT_CERTIFICATE, &pCertificate1);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate1, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, LWCA_NOT_CA_CERT);


    // Testcase 2: Certificate/Key pair mismatch
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCACreateCertificate(TEST_DUMMY_CERTIFICATE, &pCertificate2);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate2, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, LWCA_CERT_PRIVATE_KEY_MISMATCH);


    // Testcase 3: Invalid inputs
    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, NULL, NULL, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    // Testcase 4: CA exists already
    bCAExists = TRUE;
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate2, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, LWCA_CA_ALREADY_EXISTS);

    LwCAFreeCertificate(pCertificate1);
    LwCAFreeCertificate(pCertificate2);
}

VOID
Test_LwCAGetCACertificates_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;

    will_return(__wrap_LwCADbGetCACertificates, 0);
    bCAExists = TRUE;
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCAGetCACertificates(pReqCtx, TEST_ROOT_CA_ID, &pCertificates);
    assert_int_equal(dwError, 0);
    assert_non_null(pCertificates);
    assert_int_equal(pCertificates->dwCount, 1);
    assert_non_null(pCertificates->ppCertificates);
    assert_string_equal(pCertificates->ppCertificates[0], "dummyCert");

    LwCAFreeCertificates(pCertificates);
}

VOID
Test_LwCAGetCACertificates_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;

    // Testcase 1: Invalid input
    dwError = LwCAGetCACertificates(pReqCtx, NULL, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);

    // Testcase 2: CA does not exist
    bCAExists = FALSE;
    will_return(__wrap_LwCADbCheckCA, 0);

    dwError = LwCAGetCACertificates(pReqCtx, TEST_ROOT_CA_ID, &pCertificates);
    assert_int_equal(dwError, LWCA_CA_MISSING);
    assert_null(pCertificates);

    LwCAFreeCertificates(pCertificates);
}
