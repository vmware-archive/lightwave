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

static
DWORD
_LwCALoadCAData(
    PLWCA_DB_CA_DATA    *ppCaData
    );

static
DWORD
_LwCALoadCertData(
    PLWCA_DB_CERT_DATA  *ppCert
    );

VOID
Test_LwCAPostDbInitCtx(
    VOID    **state
    )
{
    DWORD               dwError = 0;
    PLWCA_JSON_OBJECT   pJson = NULL;

    dwError = LwCAJsonLoadObjectFromFile(LWCA_POST_DB_CONFIG, &pJson);
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

    dwError = LwCAJsonLoadObjectFromFile(LWCA_POST_DB_CONFIG, &pJson);
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
Test_LwCAAddCertData(
    VOID **state
    )
{
    PLWCA_TEST_STATE            pState = NULL;
    PSTR                        pszFunc = LWCA_POST_PLUGIN_ADD_CERT;
    PLUGIN_ADD_CERT             pFnAddCert = NULL;
    PLWCA_PLUGIN_HANDLE         pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnAddCert = (PLUGIN_ADD_CERT)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnAddCert);
}

VOID
Test_LwCASerializeCertDataToJson(
    VOID **state
    )
{
    DWORD                       dwError = 0;
    PLWCA_TEST_STATE            pState = NULL;
    PSTR                        pszFunc = LWCA_POST_SERIALIZE_CERT_DATA_TO_JSON;
    SERIALIZE_CERT_DATA_JSON    pFnSerialize = NULL;
    PLWCA_DB_CERT_DATA          pCert = NULL;
    PSTR                        pszSerializedData = NULL;
    PLWCA_PLUGIN_HANDLE         pPluginHandle = NULL;
    PCSTR                       pcszDN = TEST_INTR_CA_DN;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnSerialize = (SERIALIZE_CERT_DATA_JSON)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnSerialize);

    dwError = _LwCALoadCertData(&pCert);
    assert_int_equal(dwError, 0);

    dwError = pFnSerialize(TEST_CA_ID, pCert, pcszDN, &pszSerializedData);
    assert_int_equal(dwError, 0);
    assert_non_null(pszSerializedData);
    assert_string_equal(pszSerializedData, SERIALIZED_CERT_DATA_JSON);

    LWCA_SAFE_FREE_STRINGA(pszSerializedData);
    LwCADbFreeCertData(pCert);
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
    assert_string_equal(pCaData->pszLastCRLUpdate, TEST_LAST_CRL_UPDATE);
    assert_string_equal(pCaData->pszNextCRLUpdate, TEST_NEXT_CRL_UPDATE);
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
    assert_string_equal(pCaData->pszLastCRLUpdate, TEST_LAST_CRL_UPDATE);
    assert_string_equal(pCaData->pszNextCRLUpdate, TEST_NEXT_CRL_UPDATE);
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
Test_LwCAGetCertData(
    VOID **state
    )
{
    PLWCA_TEST_STATE    pState = NULL;
    PSTR                pszFunc = LWCA_POST_GET_CERT;
    PLUGIN_GET_CERT     pFnGetCert = NULL;
    PLWCA_PLUGIN_HANDLE pPluginHandle = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnGetCert = (PLUGIN_GET_CERT)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnGetCert);
}

VOID
Test_LwCADeserializeCertData(
    VOID **state
    )
{
    DWORD                       dwError = 0;
    PLWCA_TEST_STATE            pState = NULL;
    PSTR                        pszFunc = LWCA_POST_DESERIALIZE_JSON_TO_CERT_DATA;
    DESERIALIZE_JSON_CERT_DATA  pFnDeserializeCertData = NULL;
    PLWCA_PLUGIN_HANDLE         pPluginHandle = NULL;
    PLWCA_DB_CERT_DATA_ARRAY    pCertDataArray = NULL;
    PLWCA_DB_CERT_DATA          pCertData = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnDeserializeCertData = (DESERIALIZE_JSON_CERT_DATA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnDeserializeCertData);

    dwError = pFnDeserializeCertData(CERT_DATA_JSON_RESPONSE, &pCertDataArray);
    assert_int_equal(dwError, 0);
    assert_non_null(pCertDataArray);
    assert_int_equal(pCertDataArray->dwCount, 2);

    pCertData = pCertDataArray->ppCertData[0];
    assert_string_equal(pCertData->pszSerialNumber, "2000");
    assert_int_equal(pCertData->revokedReason, TEST_REVOKED_REASON);
    assert_string_equal(pCertData->pszRevokedDate, TEST_REVOKED_DATE);
    assert_string_equal(pCertData->pszTimeValidFrom, TEST_TIME_VALID_FROM);
    assert_string_equal(pCertData->pszTimeValidTo, TEST_TIME_VALID_TO);
    assert_int_equal(pCertData->status, TEST_LWCA_CERT_STATUS);

    pCertData = pCertDataArray->ppCertData[1];
    assert_string_equal(pCertData->pszSerialNumber, TEST_SERIAL_NUMBER);
    assert_int_equal(pCertData->revokedReason, TEST_REVOKED_REASON);
    assert_string_equal(pCertData->pszRevokedDate, TEST_REVOKED_DATE);
    assert_string_equal(pCertData->pszTimeValidFrom, TEST_TIME_VALID_FROM);
    assert_string_equal(pCertData->pszTimeValidTo, TEST_TIME_VALID_TO);
    assert_int_equal(pCertData->status, TEST_LWCA_CERT_STATUS);
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

VOID
Test_LwCAPostGetParentCAId(
    VOID **state
    )
{
    DWORD                   dwError = 0;
    PLWCA_TEST_STATE        pState = NULL;
    PSTR                    pszFunc = LWCA_POST_GET_PARENT_CA_ID;
    PLUGIN_GET_PARENT_CA    pFnGetParentCA = NULL;
    PSTR                    pszJsonFunc = LWCA_GET_STRING_FROM_RESPONSE;
    JSON_GET_STRING_ATTR    pFnJsonGetStringAttr = NULL;
    PLWCA_PLUGIN_HANDLE     pPluginHandle = NULL;
    PSTR                    pszParentCAID = NULL;

    assert_non_null(state);
    pState = *state;

    pPluginHandle = pState->pPluginHandle;

    pFnGetParentCA = (PLUGIN_GET_PARENT_CA)LwCAGetLibSym(pPluginHandle, pszFunc);
    assert_non_null(pFnGetParentCA);

    pFnJsonGetStringAttr = (JSON_GET_STRING_ATTR)LwCAGetLibSym(pPluginHandle,
                                                               pszJsonFunc
                                                               );
    assert_non_null(pFnJsonGetStringAttr);

    dwError = pFnJsonGetStringAttr(ROOT_CA_JSON_RESPONSE,
                                   "cAParentCAId",
                                   &pszParentCAID
                                   );
    assert_int_equal(dwError, LWCA_JSON_PARSE_ERROR);
    assert_null(pszParentCAID);

    dwError = pFnJsonGetStringAttr(INTR_CA_JSON_RESPONSE,
                                   "cAParentCAId",
                                   &pszParentCAID
                                   );

    assert_int_equal(dwError, 0);
    assert_non_null(pszParentCAID);
    assert_string_equal(pszParentCAID, TEST_PARENT_CA_ID);

    LWCA_SAFE_FREE_STRINGA(pszParentCAID);
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

    dwError = LwCAAllocateStringA(TEST_LAST_CRL_UPDATE,
                                  &pCaData->pszLastCRLUpdate
                                  );
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAllocateStringA(TEST_NEXT_CRL_UPDATE,
                                  &pCaData->pszNextCRLUpdate
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

static
DWORD
_LwCALoadCertData(
    PLWCA_DB_CERT_DATA  *ppCert
    )
{
    DWORD               dwError = 0;
    PLWCA_DB_CERT_DATA  pCert = NULL;

    dwError = LwCADbCreateCertData(TEST_SERIAL_NUMBER,
                                   TEST_TIME_VALID_FROM,
                                   TEST_TIME_VALID_TO,
                                   1,
                                   TEST_REVOKED_DATE,
                                   1,
                                   &pCert
                                   );
    BAIL_ON_LWCA_ERROR(dwError);

    *ppCert = pCert;

cleanup:
    return dwError;

error:
    LwCADbFreeCertData(pCert);
    goto cleanup;
}
