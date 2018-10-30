/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”) you may not
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

#define LWCA_DB_CONFIG "./test-mutentcadb-config/test-postdb-config.json"

#define LWCA_POST_PLUGIN_ADD_CA             "LwCADbPostPluginAddCA"
#define LWCA_POST_SERIALIZE_CA_TO_JSON      "LwCASerializeCAToJSON"
#define LWCA_POST_SERIALIZE_CONFIG_CA_TO_JSON      "LwCASerializeConfigCAToJSON"
#define LWCA_POST_DESERIALIZE_JSON_TO_CA    "LwCADeserializeJSONToCA"
#define LWCA_POST_CHECK_CA                  "LwCADbPostPluginCheckCA"
#define LWCA_POST_GET_CA                    "LwCADbPostPluginGetCA"
#define LWCA_POST_UPDATE_CA                 "LwCADbPostPluginUpdateCA"
#define LWCA_UPDATE_CA_REQUEST_BODY         "LwCAGenerateCAPatchRequestBody"
#define LWCA_CONFIG_DB_PLUGIN_KEY_NAME   "dbPlugin"
#define LWCA_CONFIG_DB_PLUGIN_PATH       "dbPluginConfigPath"
#define TEST_SUBJECT                "TEST_SUBJECT"
#define TEST_PRIV_KEY               "01000100"
#define TEST_CERT_1                 "10101010"
#define TEST_CERT_2                 "11110000"
#define TEST_CERT_3                 "01010101"
#define TEST_CRL_NUM                "1500"
#define TEST_CA_STATUS              1
#define TEST_CA_ID                  "testId"
#define TEST_PARENT_CA_ID           "testParentId"
#define DUMMY_DOMAIN                "dc=lw-testdom,dc=com"

#define SERIALIZED_ROOT_CA_JSON ("{\n" \
    "    \"dn\": \"cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "    \"attributes\": [\n" \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"vmwCertificationAuthority\",\n" \
    "                \"pkiCA\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\n" \
    "                \"testParentId\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \"TEST_SUBJECT\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \"01000100\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \"1500\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \"10101010\",\n" \
    "                \"11110000\",\n" \
    "                \"01010101\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    ]\n" \
    "}")

#define SERIALIZED_INTERMEDIATE_CA_JSON ("{\n" \
    "    \"dn\": \"cn=testId,cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "    \"attributes\": [\n" \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"vmwCertificationAuthority\",\n" \
    "                \"pkiCA\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\n" \
    "                \"testId\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \"TEST_SUBJECT\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \"01000100\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \"1500\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAParentCAId\",\n" \
    "            \"value\": [\n" \
    "                \"testParentId\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \"10101010\",\n" \
    "                \"11110000\",\n" \
    "                \"01010101\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    ]\n" \
    "}")

#define SERIALIZED_CONFIG_ROOT_CA_JSON ("{\n" \
    "    \"dn\": \"cn=testParentId,cn=Configuration,dc=lw-testdom,dc=com\",\n"  \
    "    \"attributes\": [\n"   \
    "        {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\n" \
    "                \"container\",\n" \
    "                \"top\"\n" \
    "            ]\n" \
    "        },\n" \
    "        {\n"   \
    "            \"type\": \"cn\",\n"   \
    "            \"value\": [\n"    \
    "                \"testParentId\"\n"    \
    "            ]\n"   \
    "        }\n"   \
    "    ]\n"   \
    "}")

#define ROOT_CA_JSON_RESPONSE (\
    "{\n" \
    "    \"result\": [{\n" \
    "        \"dn\": \"cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\"10101010\", \"11110000\", \"01010101\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\"1500\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\"01000100\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\"TEST_SUBJECT\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\"testParentId\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCertificationAuthority\", \"pkiCA\"]\n" \
    "        }]\n" \
    "    }],\n" \
    "    \"result_count\": 1\n" \
    "}")

#define INTR_CA_JSON_RESPONSE (\
    "{\n" \
    "    \"result\": [{\n" \
    "        \"dn\": \"cn=testId,cn=testParentId,cn=Certificate-Authority,dc=lw-testdom,dc=com\",\n" \
    "        \"attributes\": [{\n" \
    "            \"type\": \"nTSecurityDescriptor\",\n" \
    "            \"value\": [\"\\u0001\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\"1\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\"10101010\", \"11110000\", \"01010101\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAParentCAId\",\n" \
    "            \"value\": [\"testParentId\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\"1500\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\"01000100\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\"TEST_SUBJECT\"]\n" \
    "        }, {\n" \
    "            \"type\": \"cn\",\n" \
    "            \"value\": [\"testId\"]\n" \
    "        }, {\n" \
    "            \"type\": \"objectClass\",\n" \
    "            \"value\": [\"vmwCertificationAuthority\", \"pkiCA\"]\n" \
    "        }]\n" \
    "    }],\n" \
    "    \"result_count\": 1\n" \
    "}")

#define CA_GENERATED_PATCH (\
    "[\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACertificateDN\",\n" \
    "            \"value\": [\n" \
    "                \"TEST_SUBJECT\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACertificate\",\n" \
    "            \"value\": [\n" \
    "                \"10101010\",\n" \
    "                \"11110000\",\n" \
    "                \"01010101\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cAEncryptedPrivateKey\",\n" \
    "            \"value\": [\n" \
    "                \"01000100\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cACRLNumber\",\n" \
    "            \"value\": [\n" \
    "                \"1500\"\n" \
    "            ]\n" \
    "        }\n" \
    "    },\n" \
    "    {\n" \
    "        \"operation\": \"replace\",\n" \
    "        \"attribute\": {\n" \
    "            \"type\": \"cAStatus\",\n" \
    "            \"value\": [\n" \
    "                \"1\"\n" \
    "            ]\n" \
    "        }\n" \
    "    }\n" \
    "]")

static
DWORD
_LwCALoadCAData(
    PLWCA_DB_CA_DATA    *ppCaData
    );

VOID
Test_LwCAPostDbInitCtx(
    VOID    **state
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJson = NULL;

    dwError = LwCAJsonLoadObjectFromFile(LWCA_DB_CONFIG, &pJson);
    assert_int_equal(dwError, 0);

    dwError = LwCADbInitCtx(pJson);
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCAPostDbFreeCtx(
    VOID    **state
    )
{
    LwCADbFreeCtx();
}

static
DWORD
_LwCALoadCAData(
    PLWCA_DB_CA_DATA    *ppCaData
    )
{
    DWORD                       dwError = 0;
    PLWCA_DB_CA_DATA            pCaData = NULL;
    int                         certCount = 3;
    PSTR                        pszCertificates[3] = {TEST_CERT_1,
                                                      TEST_CERT_2,
                                                      TEST_CERT_3};

    dwError = LwCAAllocateMemory(sizeof(*pCaData), (PVOID *)&pCaData);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(TEST_SUBJECT, &pCaData->pszSubjectName);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(TEST_CRL_NUM, &pCaData->pszCRLNumber);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateKey(TEST_PRIV_KEY,
                            strlen(TEST_PRIV_KEY),
                            &pCaData->pEncryptedPrivateKey
                            );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCACreateCertArray(pszCertificates,
                                  certCount,
                                  &pCaData->pCertificates
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    pCaData->status = TEST_CA_STATUS;

    *ppCaData = pCaData;

cleanup:
    return dwError;

error:
    LwCADbFreeCAData(pCaData);
    goto cleanup;
}

int
PreTest_LwCAPostPlugin(
    VOID    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_DB_FUNCTION_TABLE pFt = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PLWCA_DB_HANDLE         pDbHandle = NULL;
    PLWCA_TEST_STATE        pState = NULL;
    PLWCA_JSON_OBJECT       pJson = NULL;
    PSTR                    pszPlugin = NULL;
    PSTR                    pszPluginConfigPath = NULL;

    if (!state)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(*pState), (PVOID *)&pState);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonLoadObjectFromFile(LWCA_DB_CONFIG, &pJson);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_CONFIG_DB_PLUGIN_KEY_NAME,
                                       &pszPlugin
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateMemory(sizeof(*pFt), (PVOID *)&pFt);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPluginInitialize(pszPlugin, pFt, &pPluginHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetStringFromKey(pJson,
                                       FALSE,
                                       LWCA_CONFIG_DB_PLUGIN_PATH,
                                       &pszPluginConfigPath
                                       );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = pFt->pFnInit(pszPluginConfigPath, &pDbHandle);
    BAIL_ON_LWCA_ERROR(dwError);

    pState->pPluginHandle = pPluginHandle;
    pState->pDbHandle = pDbHandle;
    pState->pFunctionTable = pFt;
    *state = pState;

cleanup:
    LWCA_SAFE_FREE_STRINGA(pszPluginConfigPath);
    LWCA_SAFE_FREE_STRINGA(pszPlugin);

    return dwError;

error:
    if (pFt)
    {
        pFt->pFnFreeHandle(pDbHandle);
    }
    LwCAPluginDeinitialize(pPluginHandle);
    LWCA_SAFE_FREE_MEMORY(pFt);
    LWCA_SAFE_FREE_MEMORY(pState);

    goto cleanup;
}

int
PostTest_LwCAPostPlugin(
    VOID    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_DB_FUNCTION_TABLE pFt = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PLWCA_DB_HANDLE         pDbHandle = NULL;
    PLWCA_TEST_STATE        pState = NULL;

    if (!state)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    pState = *state;
    pFt = pState->pFunctionTable;
    pPluginHandle = pState->pPluginHandle;
    pDbHandle = pState->pDbHandle;

    if (pFt)
    {
        pFt->pFnFreeHandle(pDbHandle);
    }
    LwCAPluginDeinitialize(pPluginHandle);
    LWCA_SAFE_FREE_MEMORY(pFt);
    LWCA_SAFE_FREE_MEMORY(pState);

    *state = NULL;

error:
    return dwError;
}

VOID
Test_LwCAPostDbAddCA(
    VOID    **state
    )
{
    PLWCA_TEST_STATE    pState = NULL;
    PLUGIN_ADD_CA       pFnAddCA = NULL;
    PSTR                pszFunc = LWCA_POST_PLUGIN_ADD_CA;

    assert_non_null(state);
    pState = *state;

    pFnAddCA = (PLUGIN_ADD_CA)LwCAGetLibSym(pState->pPluginHandle, pszFunc);
    assert_non_null(pFnAddCA);
}

VOID
Test_LwCASerializeRootCAToJson(
    VOID **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_TEST_STATE        pState = NULL;
    PSTR                    pszFunc = LWCA_POST_SERIALIZE_CA_TO_JSON;
    SERIALIZE_CA_JSON       pFnSerialize= NULL;
    PLWCA_DB_CA_DATA        pCaData = NULL;
    PSTR                    pszSerializedData = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnSerialize = (SERIALIZE_CA_JSON)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnSerialize);

    dwError = _LwCALoadCAData(&pCaData);
    assert_int_equal(dwError, 0);

    dwError = pFnSerialize(TEST_PARENT_CA_ID,
                           pCaData,
                           NULL,
                           DUMMY_DOMAIN,
                           &pszSerializedData
                           );
    assert_int_equal(dwError, 0);
    assert_non_null(pszSerializedData);
    assert_string_equal(pszSerializedData, SERIALIZED_ROOT_CA_JSON);

    LWCA_SAFE_FREE_STRINGA(pszSerializedData);
    LwCADbFreeCAData(pCaData);
}

VOID
Test_LwCASerializeIntermediateCAToJson(
    VOID    **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_TEST_STATE        pState = NULL;
    PSTR                    pszFunc = LWCA_POST_SERIALIZE_CA_TO_JSON;
    SERIALIZE_CA_JSON       pFnSerialize = NULL;
    PLWCA_DB_CA_DATA        pCaData = NULL;
    PSTR                    pszSerializedData = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnSerialize = (SERIALIZE_CA_JSON)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnSerialize);

    dwError = _LwCALoadCAData(&pCaData);
    assert_int_equal(dwError, 0);

    dwError = pFnSerialize(TEST_CA_ID,
                           pCaData,
                           TEST_PARENT_CA_ID,
                           DUMMY_DOMAIN,
                           &pszSerializedData
                           );
    assert_int_equal(dwError, 0);
    assert_non_null(pszSerializedData);
    assert_string_equal(pszSerializedData, SERIALIZED_INTERMEDIATE_CA_JSON);

    LWCA_SAFE_FREE_STRINGA(pszSerializedData);
    LwCADbFreeCAData(pCaData);
}

VOID
Test_LwCASerializeConfigRootCAToJson(
    VOID **state
    )
{
    DWORD                       dwError = 0;
    PLWCA_TEST_STATE            pState = NULL;
    PSTR                        pszFunc = LWCA_POST_SERIALIZE_CONFIG_CA_TO_JSON;
    SERIALIZE_CONFIG_CA_JSON    pFnSerialize= NULL;
    PLWCA_DB_CA_DATA            pCaData = NULL;
    PSTR                        pszSerializedData = NULL;
    PLWCA_PLUGIN_HANDLE         pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnSerialize = (SERIALIZE_CONFIG_CA_JSON)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnSerialize);

    dwError = _LwCALoadCAData(&pCaData);
    assert_int_equal(dwError, 0);

    dwError = pFnSerialize(TEST_PARENT_CA_ID,
                           DUMMY_DOMAIN,
                           &pszSerializedData
                           );
    assert_int_equal(dwError, 0);
    assert_non_null(pszSerializedData);
    assert_string_equal(pszSerializedData, SERIALIZED_CONFIG_ROOT_CA_JSON);

    LWCA_SAFE_FREE_STRINGA(pszSerializedData);
    LwCADbFreeCAData(pCaData);
}

VOID
Test_LwCADeserializeJsonToRootCA(
    VOID **state
    )
{
    DWORD               dwError = 0;
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_DESERIALIZE_JSON_TO_CA;
    DESERIALIZE_JSON_CA pFnDeserialize = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;
    PLWCA_DB_CA_DATA    pCaData = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnDeserialize = (DESERIALIZE_JSON_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnDeserialize);

    dwError = pFnDeserialize(ROOT_CA_JSON_RESPONSE, &pCaData);
    assert_int_equal(dwError, 0);
    assert_non_null(pCaData);

    assert_string_equal(pCaData->pszSubjectName, TEST_SUBJECT);
    assert_string_equal(pCaData->pszCRLNumber, TEST_CRL_NUM);
    assert_int_equal(pCaData->status, TEST_CA_STATUS);
    assert_int_equal(pCaData->pEncryptedPrivateKey->dwLength, strlen(TEST_PRIV_KEY));
    assert_string_equal(pCaData->pEncryptedPrivateKey->pData, TEST_PRIV_KEY);
    assert_int_equal(pCaData->pCertificates->dwCount, 3);
    assert_string_equal(pCaData->pCertificates->ppCertificates[0], TEST_CERT_1);
    assert_string_equal(pCaData->pCertificates->ppCertificates[1], TEST_CERT_2);
    assert_string_equal(pCaData->pCertificates->ppCertificates[2], TEST_CERT_3);
}

VOID
Test_LwCADeserializeJsonToIntrCA(
    VOID **state
    )
{
    DWORD               dwError = 0;
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_DESERIALIZE_JSON_TO_CA;
    DESERIALIZE_JSON_CA pFnDeserialize = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;
    PLWCA_DB_CA_DATA    pCaData = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnDeserialize = (DESERIALIZE_JSON_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnDeserialize);

    // the JSON response contains the Parent ID but the struct doesn't
    dwError = pFnDeserialize(INTR_CA_JSON_RESPONSE, &pCaData);
    assert_int_equal(dwError, 0);
    assert_non_null(pCaData);

    assert_string_equal(pCaData->pszSubjectName, TEST_SUBJECT);
    assert_string_equal(pCaData->pszCRLNumber, TEST_CRL_NUM);
    assert_int_equal(pCaData->status, TEST_CA_STATUS);
    assert_int_equal(pCaData->pEncryptedPrivateKey->dwLength, strlen(TEST_PRIV_KEY));
    assert_string_equal(pCaData->pEncryptedPrivateKey->pData, TEST_PRIV_KEY);
    assert_int_equal(pCaData->pCertificates->dwCount, 3);
    assert_string_equal(pCaData->pCertificates->ppCertificates[0], TEST_CERT_1);
    assert_string_equal(pCaData->pCertificates->ppCertificates[1], TEST_CERT_2);
    assert_string_equal(pCaData->pCertificates->ppCertificates[2], TEST_CERT_3);
}

VOID
Test_LwCAPostDbCheckCA(
    VOID **state
    )
{
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_CHECK_CA;
    PLUGIN_CHECK_CA     pFnCheckCA = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnCheckCA = (PLUGIN_CHECK_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnCheckCA);
}

VOID
Test_LwCAPostDbGetCA(
    VOID **state
    )
{
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_GET_CA;
    PLUGIN_GET_CA       pFnGetCA = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnGetCA = (PLUGIN_GET_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnGetCA);
}

VOID
Test_LwCAPostDbUpdateCA(
    VOID **state
    )
{
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_UPDATE_CA;
    PLUGIN_UPDATE_CA    pFnUpdateCA = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnUpdateCA = (PLUGIN_UPDATE_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnUpdateCA);
}

VOID
Test_LwCAUpdateRootCARequestBody(
    VOID **state
    )
{
    PLWCA_TEST_STATE            pState = NULL;
    PSTR                        pszFunc = LWCA_UPDATE_CA_REQUEST_BODY;
    PLUGIN_UPDATE_CA_REQ_BODY   pFnUpdateCAReqBody = NULL;
    PLWCA_PLUGIN_HANDLE         pPluginHandle = NULL;
    PLWCA_DB_CA_DATA            pCaData = NULL;
    PSTR                        pszReqBody = NULL;
    DWORD                       dwError = 0;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnUpdateCAReqBody = (PLUGIN_UPDATE_CA_REQ_BODY)LwCAGetLibSym(pPluginHandle,
                                                                  pszFunc
                                                                  );
    assert_non_null(pFnUpdateCAReqBody);
    dwError = _LwCALoadCAData(&pCaData);
    assert_int_equal(dwError, 0);
    assert_non_null(pCaData);

    dwError = pFnUpdateCAReqBody(pCaData, &pszReqBody);
    assert_int_equal(dwError, 0);
    assert_non_null(pszReqBody);
    assert_string_equal(pszReqBody, CA_GENERATED_PATCH);

    LWCA_SAFE_FREE_STRINGA(pszReqBody);
}
