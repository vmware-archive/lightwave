/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : ipclocalapi.c
 *
 * Abstract :
 *
 */
#include "includes.h"

#define LOG_URESULT_ERROR(uResult)                 \
    if (uResult)                                   \
    {                                              \
                                                   \
        VmAfdLog (VMAFD_DEBUG_ERROR,               \
            "ERROR! [%s] is returning  [%d]",      \
            __FUNCTION__, uResult                  \
           );                                      \
    }

static
VOID
VmAfdHandleError(
    UINT32 apiType,
    DWORD dwInputError,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    );

static
DWORD
VecsMarshalResponse (
                     UINT32 apiType,
                     VMW_TYPE_SPEC *output_spec,
                     DWORD noOfArgsOut,
                     PBYTE *ppResponse,
                     PDWORD pdwResponseSize
                    );

DWORD
VecsIpcCreateCertStore(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_CREATE_CERTSTORE;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        //PVECS_SERV_STORE pStore = NULL;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PBYTE pStoreBlob = NULL;
        PWSTR pwszStoreName = NULL;
        PWSTR pwszPassword = NULL;

	VMW_TYPE_SPEC input_spec[] = CREATE_STORE_REQUEST_PARAMS;
	VMW_TYPE_SPEC output_spec[] = OPEN_STORE_OUTPUT_PARAMS;

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
	BAIL_ON_VMAFD_ERROR (dwError);

        pwszStoreName = input_spec[0].data.pWString;
        pwszPassword = input_spec[1].data.pWString;

        if (IsNullOrEmptyString(pwszStoreName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvCreateCertStoreWithAuth (
                                 pwszStoreName,
                                 pwszPassword,
                                 pConnectionContext,
                                 &pStore
                                 );
        LOG_URESULT_ERROR(uResult);
	//
	// Allocate a buffer, marshall the response
	//
	output_spec[0].data.pUint32 = &uResult;

        dwError = VmAfdEncodeVecsStoreHandle (
                                        pStore,
                                        &pStoreBlob
                                       );
        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[1].data.pByte = (PBYTE)pStoreBlob;

        dwError = VecsMarshalResponse (
                                        apiType,
                                        output_spec,
                                        noOfArgsOut,
                                        &pResponse,
                                        &dwResponseSize
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        pConnectionContext->pStoreHandle = pStore;
        pStore = NULL;
cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VMAFD_SAFE_FREE_MEMORY (pStoreBlob);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        if (pStore)
        {
                VmAfdReleaseStoreHandle (pStore);
                pStore = NULL;
        }
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );
        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcEnumStores (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_ENUM_STORES;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PWSTR *pwszStoreNameArray = NULL;
        DWORD dwStoreCount = 0;
        PBYTE pStoreNameArrayBlob = NULL;
        DWORD dwStoreNameBlobSize = 0;

        VMW_TYPE_SPEC output_spec[] = ENUM_STORE_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        /*uResult = VecsSrvEnumFilteredStores (
                                             pSecurityContext,
                                             &pwszStoreNameArray,
                                             &dwStoreCount
                                            );*/

        uResult = VecsSrvEnumCertStore (
                                        &pwszStoreNameArray,
                                        &dwStoreCount
                                       );
        LOG_URESULT_ERROR(uResult);

        dwError = VmAfdMarshalStringArrayGetSize (
                                                  pwszStoreNameArray,
                                                  dwStoreCount,
                                                  &dwStoreNameBlobSize
                                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                        dwStoreNameBlobSize,
                                        (PVOID *)&pStoreNameArrayBlob
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdMarshalStringArray (
                                           pwszStoreNameArray,
                                           dwStoreCount,
                                           dwStoreNameBlobSize,
                                           pStoreNameArrayBlob
                                        );

        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pByte = (PBYTE) pStoreNameArrayBlob;

        dwError = VecsMarshalResponse (
                                        apiType,
                                        output_spec,
                                        noOfArgsOut,
                                        &pResponse,
                                        &dwResponseSize
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdFreeStringArrayW (pwszStoreNameArray, dwStoreCount);

        VMAFD_SAFE_FREE_MEMORY (pStoreNameArrayBlob);


        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}



DWORD
VecsIpcOpenCertStore (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_OPEN_CERTSTORE;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        //PVECS_SERV_STORE pStore = NULL;
        PBYTE pStoreBlob = NULL;
        PWSTR pwszStoreName = NULL;
        PWSTR pwszPassword = NULL;

        VMW_TYPE_SPEC input_spec[] = CREATE_STORE_REQUEST_PARAMS;
        VMW_TYPE_SPEC output_spec[] = OPEN_STORE_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        pwszStoreName = input_spec[0].data.pWString;
        pwszPassword = input_spec[1].data.pWString;

        if (IsNullOrEmptyString (pwszStoreName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvOpenCertStoreWithAuth (
                                pwszStoreName,
                                pwszPassword,
                                pConnectionContext,
                                &pStore
                                );
        LOG_URESULT_ERROR(uResult);
        output_spec[0].data.pUint32 = &uResult;
        dwError = VmAfdEncodeVecsStoreHandle (
                                        pStore,
                                        &pStoreBlob
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);
        output_spec[1].data.pByte = (PBYTE)pStoreBlob;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

        pConnectionContext->pStoreHandle = pStore;
        pStore = NULL;
cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VMAFD_SAFE_FREE_MEMORY (pStoreBlob);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        if (pStore)
        {
                VmAfdReleaseStoreHandle (pStore);
                pStore = NULL;
        }

        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcBeginEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_BEGIN_ENUM_ENTRIES;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
        PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext = NULL;
        PBYTE pEnumContextBlob = NULL;
        DWORD dwLimit = 0;
        DWORD dwinfoLevel = 0;


        VMW_TYPE_SPEC input_spec[] = BEGIN_ENUM_INPUT_PARAMS;
        VMW_TYPE_SPEC output_spec[] = BEGIN_ENUM_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwLimit = *input_spec[2].data.pUint32;
        dwinfoLevel = *input_spec[3].data.pUint32;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdAccessCheckWithHandle(
                                   pStore,
                                   pConnectionContext,
                                   READ_STORE
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VecsSrvValidateInfoLevel (
                                            dwinfoLevel,
                                            &infoLevel
                                           );
        BAIL_ON_VMAFD_ERROR (dwError);


        uResult = VecsSrvAllocateCertEnumContextHandle (
                                                  pStore,
                                                  dwLimit,
                                                  infoLevel,
                                                  &pEnumContext
                                                );

        LOG_URESULT_ERROR(uResult);

        dwError = VmAfdEncodeEnumContextHandle (
                                          pEnumContext,
                                          &pEnumContextBlob
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pUint32 = &pEnumContext->dwLimit;
        output_spec[2].data.pByte = (PBYTE)pEnumContextBlob;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);
        VMAFD_SAFE_FREE_MEMORY (pEnumContext);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VMAFD_SAFE_FREE_MEMORY (pEnumContextBlob);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_ENUM_ENTRIES;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext = NULL;
        PVMAFD_CERT_ARRAY pCertArray = NULL;
        DWORD dwSizeOfEntryArray = 0;
        PBYTE pCertArrayBlob = NULL;
        PBYTE pEnumContextBlob = NULL;


        VMW_TYPE_SPEC input_spec[] = ENUM_ENTRIES_INPUT_PARAMS;
        VMW_TYPE_SPEC output_spec[] = ENUM_ENTRIES_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdDecodeEnumContextHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pEnumContext
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAccessCheckWithHandle(
                                   pEnumContext->pStore,
                                   pConnectionContext,
                                   READ_STORE
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvEnumCertsHandle (
                                    pEnumContext,
                                    pConnectionContext,
                                    &pCertArray
                                   );

        LOG_URESULT_ERROR(uResult);

        dwError = VmAfdMarshalEntryArrayLength (
                                                 pCertArray,
                                                 &dwSizeOfEntryArray
                                               );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                        dwSizeOfEntryArray,
                                        (PVOID *) &pCertArrayBlob
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdMarshalEntryArray (
                                          pCertArray,
                                          dwSizeOfEntryArray,
                                          pCertArrayBlob
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);

        if (!uResult && pCertArray)
        {
                pEnumContext->dwIndex += pCertArray->dwCount;
        }

        dwError = VmAfdEncodeEnumContextHandle (
                                          pEnumContext,
                                          &pEnumContextBlob
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pByte = (PBYTE) pCertArrayBlob;
        output_spec[3].data.pByte = (PBYTE) pEnumContextBlob;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        if (pEnumContext)
        {
                if (pEnumContext->pStore)
                {
                        VMAFD_SAFE_FREE_MEMORY (pEnumContext->pStore);
                }
                VMAFD_SAFE_FREE_MEMORY (pEnumContext);
        }

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        if (pCertArray)
        {
                VecsFreeCertArray (pCertArray);
        }
        VMAFD_SAFE_FREE_MEMORY (pCertArrayBlob);
        VMAFD_SAFE_FREE_MEMORY (pEnumContextBlob);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}

DWORD
VecsIpcEndEnumEntries (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_END_ENUM_ENTRIES;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PVECS_SRV_ENUM_CONTEXT_HANDLE pEnumContext = NULL;
        //PVECS_SERV_STORE pStore = NULL;

        VMW_TYPE_SPEC input_spec[] = END_ENUM_REQUEST_PARAMS;
        VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdDecodeEnumContextHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pEnumContext
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvEndEnumContextHandle (
                                pEnumContext,
                                pConnectionContext
                                );

        LOG_URESULT_ERROR(uResult);

        output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        if (pEnumContext)
        {
          VMAFD_SAFE_FREE_MEMORY (pEnumContext->pStore);

          VMAFD_SAFE_FREE_MEMORY (pEnumContext);
        }

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}


DWORD
VecsIpcGetEntryByAlias (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_GET_ENTRY_BY_ALIAS;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SERV_STORE pStoreInstance = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        ENTRY_INFO_LEVEL infoLevel = ENTRY_INFO_LEVEL_UNDEFINED;
        PVMAFD_CERT_ARRAY pCertArray = NULL;
        PWSTR pszAlias = NULL;
        DWORD dwInfoLevel = 0;


        VMW_TYPE_SPEC input_spec[] = GET_ENTRY_BY_ALIAS_INPUT_PARAMS;
        VMW_TYPE_SPEC output_spec[] = GET_ENTRY_BY_ALIAS_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwInfoLevel = *input_spec[3].data.pUint32;
        pszAlias = input_spec[2].data.pWString;

        if (IsNullOrEmptyString (pszAlias))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }


        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAccessCheckWithHandle(
                                   pStore,
                                   pConnectionContext,
                                   READ_STORE
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsSrvValidateInfoLevel (
                                            dwInfoLevel,
                                            &infoLevel
                                           );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError =  VmAfdGetStoreFromHandle (
                                            pStore,
                                            pConnectionContext->pSecurityContext,
                                            &pStoreInstance
                                           );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvGetEntryByAlias (
                                          pStoreInstance,
                                          pszAlias,
                                          infoLevel,
                                          &pCertArray
                                          );


        LOG_URESULT_ERROR(uResult);


        output_spec[0].data.pUint32 = &uResult;

        if (!uResult && pCertArray->dwCount)
        {
                PVMAFD_CERT_CONTAINER pCursor = &pCertArray->certificates[0];
                output_spec[1].data.pUint32 = &(pCursor->dwStoreType);
                output_spec[2].data.pUint32 = &(pCursor->dwDate);
                output_spec[3].data.pWString = pCursor->pCert;
        }

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStoreInstance);
        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);

        if (pCertArray)
        {
          VecsFreeCertArray (pCertArray);
        }
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}


DWORD
VecsIpcGetKeyByAlias (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_GET_KEY_BY_ALIAS;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SERV_STORE pStoreInstance = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PWSTR pszKey = NULL;
        PWSTR pszAlias = NULL;
        PWSTR pszPassword = NULL;


        VMW_TYPE_SPEC input_spec[] = GET_KEY_BY_ALIAS_INPUT_PARAMS;
        VMW_TYPE_SPEC output_spec[] = GET_KEY_BY_ALIAS_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        pszAlias = input_spec[2].data.pWString;
        pszPassword = input_spec[3].data.pWString;

        if (IsNullOrEmptyString (pszAlias))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }


        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAccessCheckWithHandle(
                                   pStore,
                                   pConnectionContext,
                                   READ_STORE
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdGetStoreFromHandle (
                                            pStore,
                                            pConnectionContext->pSecurityContext,
                                            &pStoreInstance
                                          );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvGetPrivateKeyByAlias (
                                          pStoreInstance,
                                          pszAlias,
                                          pszPassword,
                                          &pszKey
                                          );

        LOG_URESULT_ERROR(uResult);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pWString = pszKey;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);
        VMAFD_SAFE_FREE_MEMORY (pStoreInstance);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VMAFD_SAFE_FREE_MEMORY (pszKey);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcGetEntryCount (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_GET_ENTRY_COUNT;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PVECS_SERV_STORE pStoreInstance = NULL;
        DWORD dwEntryCount = 0;


        VMW_TYPE_SPEC input_spec[] = GET_ENTRY_COUNT_INPUT_PARAMS;
        VMW_TYPE_SPEC output_spec[] = GET_ENTRY_COUNT_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAccessCheckWithHandle(
                                   pStore,
                                   pConnectionContext,
                                   READ_STORE
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdGetStoreFromHandle(
                                          pStore,
                                          pConnectionContext->pSecurityContext,
                                          &pStoreInstance
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvGetEntryCount (
                                          pStoreInstance,
                                          &dwEntryCount
                                       );


        LOG_URESULT_ERROR(uResult);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pUint32 = &dwEntryCount;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);
        VMAFD_SAFE_FREE_MEMORY (pStoreInstance);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}



DWORD
VecsIpcDeleteCertStore(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_DELETE_CERTSTORE;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PWSTR pwszStoreName = NULL;


	VMW_TYPE_SPEC input_spec[] = DELETE_STORE_REQUEST_PARAMS;
	VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
	BAIL_ON_VMAFD_ERROR (dwError);

        pwszStoreName = input_spec[0].data.pWString;

        if (IsNullOrEmptyString(pwszStoreName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvDeleteCertStoreWithAuth (
                                       pwszStoreName,
                                       pConnectionContext
                                       );


        LOG_URESULT_ERROR(uResult);

	// Allocate a buffer, marshall the response
	//
	output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcSetPermission(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_SET_PERMISSION;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
	VMW_TYPE_SPEC input_spec[] = PERMISSIONS_SET_INPUT_PARAMS;
	VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;
        PWSTR pszUserName = NULL;
        DWORD dwDesiredAccess = 0;
        VMAFD_ACE_TYPE aceType = VMAFD_ACE_TYPE_ALLOWED;

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
	BAIL_ON_VMAFD_ERROR (dwError);

        pszUserName = input_spec[2].data.pWString;
        dwDesiredAccess = *input_spec[3].data.pUint32;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        if (IsNullOrEmptyString(pszUserName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvSetPermission (
                                       pStore,
                                       pszUserName,
                                       dwDesiredAccess,
                                       aceType,
                                       pConnectionContext
                                       );

        LOG_URESULT_ERROR(uResult);

	//
	// Allocate a buffer, marshall the response
	//
	output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcRevokePermission(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_REVOKE_PERMISSION;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
	VMW_TYPE_SPEC input_spec[] = PERMISSIONS_SET_INPUT_PARAMS;
	VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;
        PWSTR pszUserName = NULL;
        DWORD dwDesiredAccess = 0;
        VMAFD_ACE_TYPE aceType = VMAFD_ACE_TYPE_ALLOWED;

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
	BAIL_ON_VMAFD_ERROR (dwError);

        pszUserName = input_spec[2].data.pWString;
        dwDesiredAccess = *input_spec[3].data.pUint32;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        if (IsNullOrEmptyString(pszUserName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvRevokePermission (
                                       pStore,
                                       pszUserName,
                                       dwDesiredAccess,
                                       aceType,
                                       pConnectionContext
                                       );


        LOG_URESULT_ERROR(uResult);
	//
	// Allocate a buffer, marshall the response
	//
	output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}

DWORD
VecsIpcGetPermissions(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_GET_PERMISSIONS;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
	VMW_TYPE_SPEC input_spec[] = PERMISSIONS_GET_INPUT_PARAMS;
	VMW_TYPE_SPEC output_spec[] = PERMISSIONS_GET_OUTPUT_PARAMS;
        PWSTR pszOwnerName = NULL;
        PVECS_STORE_PERMISSION_W pStorePermissions = NULL;
        PBYTE pStorePermissionsBlob = NULL;
        DWORD dwStorePermissionBlobSize = 0;
        DWORD dwUserCount = 0;

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
	BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvGetPermissions (
                                       pStore,
                                       pConnectionContext,
                                       &pszOwnerName,
                                       &dwUserCount,
                                       &pStorePermissions
                                       );
        LOG_URESULT_ERROR(uResult);
	//
	// Allocate a buffer, marshall the response
	//
	output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pWString = pszOwnerName;

        dwError = VmAfdMarshalPermissionArrayLength (
                                pStorePermissions,
                                dwUserCount,
                                &dwStorePermissionBlobSize
                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory(
                                dwStorePermissionBlobSize,
                                (PVOID *)&pStorePermissionsBlob
                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdMarshalPermissionArray (
                               pStorePermissions,
                               dwUserCount,
                               dwStorePermissionBlobSize,
                               pStorePermissionsBlob
                               );
        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[2].data.pByte = pStorePermissionsBlob;


        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        if (pStorePermissions)
        {
                VmAfdFreeStorePermissionArray(
                                        pStorePermissions,
                                        dwUserCount
                                        );
        }

        VMAFD_SAFE_FREE_MEMORY (pszOwnerName);

        VMAFD_SAFE_FREE_MEMORY (pStore);
        VMAFD_SAFE_FREE_MEMORY (pStorePermissionsBlob);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
	return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
	goto cleanup;
}


DWORD
VecsIpcChangeOwner(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
	UINT32 apiType = VECS_IPC_CHANGE_OWNER;
	DWORD noOfArgsIn=0;
	DWORD noOfArgsOut=0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
	VMW_TYPE_SPEC input_spec[] = CHANGE_OWNER_INPUT_PARAMS;
	VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;
        PWSTR pszUserName = NULL;

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);


	//
	// Unmarshall the request buffer to the format
	// that the API actually has
	//
	noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
	noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
	dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

        pszUserName = input_spec[2].data.pWString;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        if (IsNullOrEmptyString(pszUserName))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        uResult = VecsSrvChangeOwner (
                                       pStore,
                                       pszUserName,
                                       pConnectionContext
                                       );



        LOG_URESULT_ERROR(uResult);
    //
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}


DWORD
VecsIpcAddEntry(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
    UINT32 apiType = VECS_IPC_ADD_ENTRY;
    DWORD noOfArgsIn=0;
    DWORD noOfArgsOut=0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        CERT_ENTRY_TYPE entryType = CERT_ENTRY_TYPE_UNKNOWN;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PVECS_SERV_STORE pStoreInstance = NULL;

        PWSTR pszAlias = NULL;
        PWSTR pszCertificate = NULL;
        PWSTR pszPrivateKey = NULL;
        PWSTR pszPassword= NULL;
        DWORD dwEntryType = 0;
        BOOLEAN bAutoRefresh = 0;

    VMW_TYPE_SPEC input_spec[] = ADD_ENTRY_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }


    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

        dwEntryType = *input_spec[2].data.pUint32;
        pszAlias = input_spec[3].data.pWString;
        pszCertificate = input_spec[4].data.pWString;
        pszPrivateKey = input_spec[5].data.pWString;
        pszPassword = input_spec[6].data.pWString;
        bAutoRefresh = *input_spec[7].data.pBoolean;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VecsSrvValidateEntryType (
                                            dwEntryType,
                                            &entryType
                                           );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdAccessCheckWithHandle (
                                    pStore,
                                    pConnectionContext,
                                    WRITE_STORE
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdGetStoreFromHandle (
                                           pStore,
                                           pConnectionContext->pSecurityContext,
                                           &pStoreInstance
                                          );
        BAIL_ON_VMAFD_ERROR (dwError);


        uResult = VecsSrvAddCertificate (
                                        pStoreInstance,
                                        entryType,
                                        pszAlias,
                                        pszCertificate,
                                        pszPrivateKey,
                                        pszPassword,
                                        bAutoRefresh
                                       );

        LOG_URESULT_ERROR(uResult);
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStore);
        VMAFD_SAFE_FREE_MEMORY (pStoreInstance);
        /*if (pStore)
        {
                VecsSrvReleaseCertStore (pStore);
        }*/

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}


DWORD
VecsIpcDeleteEntry(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{

        DWORD dwError = 0;
        UINT32 uResult = 0;
    UINT32 apiType = VECS_IPC_DELETE_ENTRY;
    DWORD noOfArgsIn=0;
    DWORD noOfArgsOut=0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        //PVECS_SERV_STORE pStore = NULL;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        PVECS_SERV_STORE pStoreInstance = NULL;

        PWSTR pszAlias = NULL;

    VMW_TYPE_SPEC input_spec[] = DELETE_ENTRY_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }


    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

        pszAlias = input_spec[2].data.pWString;

        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);


        if (IsNullOrEmptyString(pszAlias))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VmAfdAccessCheckWithHandle (
                                    pStore,
                                    pConnectionContext,
                                    WRITE_STORE
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdGetStoreFromHandle(
                                          pStore,
                                          pConnectionContext->pSecurityContext,
                                          &pStoreInstance
                                         );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvDeleteCertificate (
                                            pStoreInstance,
                                            pszAlias
                                           );



        LOG_URESULT_ERROR(uResult);
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VMAFD_SAFE_FREE_MEMORY (pStoreInstance);
        VMAFD_SAFE_FREE_MEMORY (pStore);

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}

DWORD
VecsIpcCloseCertStore (
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VECS_IPC_CLOSE_CERTSTORE;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PVECS_SRV_STORE_HANDLE pStore = NULL;
        //PVECS_SERV_STORE pStore = NULL;

        VMW_TYPE_SPEC input_spec[] = CLOSE_STORE_REQUEST_PARAMS;
        VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              input_spec
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwError = VmAfdDecodeVecsStoreHandle (
                                        input_spec[0].data.pByte,
                                        *input_spec[1].data.pUint32,
                                        &pStore
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        uResult = VecsSrvCloseCertStoreHandle (
                                pStore,
                                pConnectionContext
                                );


        LOG_URESULT_ERROR(uResult);
        pStore = NULL;
        output_spec[0].data.pUint32 = &uResult;

        dwError = VecsMarshalResponse(
                                      apiType,
                                      output_spec,
                                      noOfArgsOut,
                                      &pResponse,
                                      &dwResponseSize
                                     );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdLog (
                  VMAFD_DEBUG_ERROR,
                  "ERROR! %s failed. Exiting with error : [%d]",
                  __FUNCTION__, dwError
                 );

        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
        VMAFD_SAFE_FREE_MEMORY (pStore);
    goto cleanup;
}

DWORD
VmAfdIpcGetStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    VMAFD_STATUS afdStatus = VMAFD_STATUS_UNKNOWN;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_STATUS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_STATUS_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    afdStatus = VmAfdSrvGetStatus();

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &afdStatus;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetDomainName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszDomainName = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_DOMAIN_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_NAME_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetDomainName(&pwszDomainName);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszDomainName;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY (pwszDomainName);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetDomainName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_DOMAIN_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDomainName = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_DOMAIN_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszDomainName = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetDomainName(pwszDomainName);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetDomainState(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_DOMAIN_STATE;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_STATE_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetDomainState(&domainState);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = (UINT32 *) &domainState;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
     VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetLDU(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszLDU = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_LDU;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_LDU_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetLDU(&pwszLDU);

    LOG_URESULT_ERROR(uResult);
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszLDU;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszLDU);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetLDU(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_LDU;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszLDU = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_LDU_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszLDU = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszLDU))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetLDU(pwszLDU);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetRHTTPProxyPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_RHTTPPROXY_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_RHTTPPROXY_PORT_OUTPUT_PARAMS;
    DWORD dwPort = 0;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetRHTTPProxyPort(&dwPort);
    LOG_URESULT_ERROR(uResult);
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &dwPort;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:

    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);

    return dwError;

error:

    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;

    goto cleanup;
}

DWORD
VmAfdIpcSetRHTTPProxyPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_RHTTPPROXY_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    DWORD dwPort = 0;

    VMW_TYPE_SPEC input_spec[] = SET_RHTTPPROXY_PORT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwPort = *(input_spec[0].data.pUint32);

    uResult = VmAfSrvSetRHTTPProxyPort(dwPort);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetDCPort(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_DC_PORT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    DWORD dwPort = 0;

    VMW_TYPE_SPEC input_spec[] = SET_DC_PORT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwPort = *(input_spec[0].data.pUint32);

    uResult = VmAfSrvSetDCPort(dwPort);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetCMLocation(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszCMLocation = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_CM_LOCATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_CM_LOCATION_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetCMLocation(&pwszCMLocation);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszCMLocation;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszCMLocation);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetLSLocation(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszLSLocation = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_LS_LOCATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_LS_LOCATION_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetLSLocation(&pwszLSLocation);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszLSLocation;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszLSLocation);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszDCName = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_DC_NAME_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetDCName(&pwszDCName);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszDCName;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszDCName);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDCName = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_DC_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszDCName = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszDCName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetDCName(pwszDCName);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetMachineAccountInfo(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszAccount = NULL;
    PWSTR pwszPassword = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_MACHINE_ACCOUNT_INFO;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_MACHINE_ACCOUNT_INFO_OUTPUT_PARAMS;
    BOOL bIsAllowed = FALSE;
    PSTR pszSecurity = NULL;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    bIsAllowed = VmAfdIsRootSecurityContext(pConnectionContext);
#ifndef _WIN32
    if (!bIsAllowed)
    {
        dwError = VmAfSrvGetRegKeySecurity(
                    VMAFD_VMDIR_CONFIG_PARAMETER_KEY_PATH,
                    &pszSecurity);
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdCheckAclContext(
                    pConnectionContext,
                    pszSecurity,
                    &bIsAllowed);
        BAIL_ON_VMAFD_ERROR (dwError);
    }
#endif

    if (!bIsAllowed)
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvGetMachineAccountInfo(
                        &pwszAccount,
                        &pwszPassword,
                        NULL,
                        NULL);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszAccount;
    output_spec[2].data.pWString = pwszPassword;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszAccount);
    VMAFD_SAFE_FREE_MEMORY(pwszPassword);
    VMAFD_SAFE_FREE_MEMORY(pszSecurity);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetSiteGUID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszSiteGUID = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_SITE_GUID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_SITE_GUID_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetSiteGUID(&pwszSiteGUID);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszSiteGUID;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszSiteGUID);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetSiteName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszSiteName = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_SITE_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_SITE_NAME_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetSiteName(&pwszSiteName);

    LOG_URESULT_ERROR(uResult);
    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszSiteName;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszSiteName);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetMachineID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszMachineID = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_MACHINE_ID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_MACHINE_ID_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvCfgGetMachineID(&pwszMachineID);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszMachineID;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszMachineID);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetMachineID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_MACHINE_ID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszMachineID = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_MACHINE_ID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszMachineID = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszMachineID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetMachineID(pwszMachineID);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcPromoteVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_PROMOTE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    PWSTR pwszServerName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszSiteName = NULL;
    PWSTR pwszPartnerHostName = NULL;

    VMW_TYPE_SPEC input_spec[] = PROMOTE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszServerName      = input_spec[0].data.pWString;
    pwszDomainName      = input_spec[1].data.pWString;
    pwszUserName        = input_spec[2].data.pWString;
    pwszPassword        = input_spec[3].data.pWString;
    pwszSiteName        = input_spec[4].data.pWString;
    pwszPartnerHostName = input_spec[5].data.pWString;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvPromoteVmDir(
                      pwszServerName,
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword,
                      pwszSiteName,
                      pwszPartnerHostName
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcDemoteVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_DEMOTE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    VMW_TYPE_SPEC input_spec[] = DEMOTE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszServerName  = input_spec[0].data.pWString;
    pwszUserName    = input_spec[1].data.pWString;
    pwszPassword    = input_spec[2].data.pWString;

    if ( IsNullOrEmptyString(pwszUserName) ||
     IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvDemoteVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcJoinValidateCredentials(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VALIDATE_CREDENTIALS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;

    VMW_TYPE_SPEC input_spec[] = JOIN_VALIDATE_CREDENTIALS_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszDomainName  = input_spec[0].data.pWString;
    pwszUserName    = input_spec[1].data.pWString;
    pwszPassword    = input_spec[2].data.pWString;

    if (IsNullOrEmptyString(pwszDomainName) ||
        IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvJoinValidateCredentials(
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcJoinVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;

    VMW_TYPE_SPEC input_spec[] = JOIN_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszServerName  = input_spec[0].data.pWString;
    pwszUserName    = input_spec[1].data.pWString;
    pwszPassword    = input_spec[2].data.pWString;
    pwszMachineName = input_spec[3].data.pWString;
    pwszDomainName  = input_spec[4].data.pWString;
    pwszOrgUnit     = input_spec[5].data.pWString;

    if (IsNullOrEmptyString(pwszServerName) ||
        IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszMachineName) ||
        IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvJoinVmDir(
                      pwszServerName,
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszDomainName,
                      pwszOrgUnit
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcJoinVmDir2(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_VMDIR_2;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;
    VMAFD_JOIN_FLAGS dwFlags = 0;
    int idx = 0;

    VMW_TYPE_SPEC input_spec[] = JOIN_VMDIR_2_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszDomainName  = input_spec[idx++].data.pWString;
    pwszUserName    = input_spec[idx++].data.pWString;
    pwszPassword    = input_spec[idx++].data.pWString;
    pwszMachineName = input_spec[idx++].data.pWString;
    pwszOrgUnit     = input_spec[idx++].data.pWString;
    dwFlags         = *input_spec[idx++].data.pUint32;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvJoinVmDir2(
                      pwszDomainName,
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszOrgUnit,
                      dwFlags);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcLeaveVmDir(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_LEAVE_VMDIR;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszServerName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PUINT32 pdwLeaveFlags = NULL;
    VMW_TYPE_SPEC input_spec[] = LEAVE_VMDIR_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszServerName  = input_spec[0].data.pWString;
    pwszUserName    = input_spec[1].data.pWString;
    pwszPassword    = input_spec[2].data.pWString;
    pdwLeaveFlags   = input_spec[3].data.pUint32;

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvLeaveVmDir(
                      pwszUserName,
                      pwszPassword,
                      *pdwLeaveFlags
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcCreateComputerAccount(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_CREATE_COMPUTER_ACCOUNT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;

    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszMachineName = NULL;
    PWSTR pwszOrgUnit = NULL;
    PWSTR pwszOutPassword = NULL;
    int idx = 0;

    VMW_TYPE_SPEC input_spec[] = CREATE_COMPUTER_ACCOUNT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = CREATE_COMPUTER_ACCOUNT_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszUserName    = input_spec[idx++].data.pWString;
    pwszPassword    = input_spec[idx++].data.pWString;
    pwszMachineName = input_spec[idx++].data.pWString;
    pwszOrgUnit     = input_spec[idx++].data.pWString;

    if (IsNullOrEmptyString(pwszMachineName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvCreateComputerAccount(
                      pwszUserName,
                      pwszPassword,
                      pwszMachineName,
                      pwszOrgUnit,
                      &pwszOutPassword);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszOutPassword;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszOutPassword);
    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcJoinAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_JOIN_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszOrgUnit = NULL;
    VMW_TYPE_SPEC input_spec[] = JOIN_AD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszUserName    = input_spec[0].data.pWString;
    pwszPassword    = input_spec[1].data.pWString;
    pwszDomainName  = input_spec[2].data.pWString;
    pwszOrgUnit     = input_spec[3].data.pWString;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword) ||
        IsNullOrEmptyString(pwszDomainName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvJoinAD(
                      pwszUserName,
                      pwszPassword,
                      pwszDomainName,
                      pwszOrgUnit
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcLeaveAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_LEAVE_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    VMW_TYPE_SPEC input_spec[] = LEAVE_AD_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszUserName    = input_spec[0].data.pWString;
    pwszPassword    = input_spec[1].data.pWString;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvLeaveAD(
                      pwszUserName,
                      pwszPassword
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcQueryAD(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszComputer = NULL;
    PWSTR pwszDomain = NULL;
    PWSTR pwszDistinguishedName = NULL;
    PWSTR pwszNetbiosName = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_QUERY_AD;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = QUERY_AD_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvQueryAD(
                        &pwszComputer,
                        &pwszDomain,
                        &pwszDistinguishedName,
                        &pwszNetbiosName);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszComputer;
    output_spec[2].data.pWString = pwszDomain;
    output_spec[3].data.pWString = pwszDistinguishedName;
    output_spec[4].data.pWString = pwszNetbiosName;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszComputer);
    VMAFD_SAFE_FREE_MEMORY(pwszDomain);
    VMAFD_SAFE_FREE_MEMORY(pwszDistinguishedName);
    VMAFD_SAFE_FREE_MEMORY(pwszNetbiosName);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcForceReplication(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_FORCE_REPLICATION;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszServerName = NULL;
    VMW_TYPE_SPEC input_spec[] = FORCE_REPLICATION_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszServerName = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszServerName))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvForceReplication(pwszServerName);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetPNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszPNID = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_PNID_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetPNID(&pwszPNID);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszPNID;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetPNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszPNID = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_PNID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszPNID = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszPNID))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetPNID(pwszPNID);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcGetCAPath(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PWSTR pwszCAPath = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_CA_PATH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_CA_PATH_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvGetCAPath(&pwszCAPath);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pWString = pwszCAPath;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VMAFD_SAFE_FREE_MEMORY(pwszCAPath);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcSetCAPath(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_SET_CA_PATH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszCAPath = NULL;

    VMW_TYPE_SPEC input_spec[] = SET_CA_PATH_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                          apiType,
                          VER1_INPUT,
                          noOfArgsIn,
                          pRequest,
                          dwRequestSize,
                          input_spec
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    pwszCAPath = input_spec[0].data.pWString;

    if (IsNullOrEmptyString (pwszCAPath))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvSetCAPath(pwszCAPath);
    LOG_URESULT_ERROR(uResult);

    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                          apiType,
                          output_spec,
                          noOfArgsOut,
                          &pResponse,
                          &dwResponseSize
                          );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdLog (
              VMAFD_DEBUG_ERROR,
              "ERROR! %s failed. Exiting with error : [%d]",
               __FUNCTION__, dwError
             );

    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcTriggerRootCertsRefresh(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_TRIGGER_ROOT_CERTS_REFRESH;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfdRootFetchTask(TRUE);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcRefreshSiteName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_REFRESH_SITE_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = VmAfSrvRefreshSiteName();
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcConfigureDNS(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_CONFIGURE_DNS;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMAFD_DOMAIN_STATE domainState = VMAFD_DOMAIN_STATE_NONE;
    PWSTR pwszDomainName = NULL;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszPNID = NULL;
    VMW_TYPE_SPEC input_spec[] = DNS_CONFIG_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszUserName        = input_spec[0].data.pWString;
    pwszPassword        = input_spec[1].data.pWString;

    if (IsNullOrEmptyString(pwszUserName) ||
        IsNullOrEmptyString(pwszPassword))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfSrvGetDomainState(&domainState);
    BAIL_ON_VMAFD_ERROR(dwError);

    if (domainState != VMAFD_DOMAIN_STATE_CONTROLLER)
    {
        dwError = ERROR_INVALID_STATE;
        BAIL_ON_VMAFD_ERROR(dwError);
    }

    dwError = VmAfSrvGetDomainName(&pwszDomainName);
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfSrvGetPNID(&pwszPNID);
    BAIL_ON_VMAFD_ERROR(dwError);

    uResult = VmAfSrvConfigureDNSW(
                   pwszPNID,
                   pwszDomainName,
                   pwszUserName,
                   pwszPassword);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);

    VMAFD_SAFE_FREE_MEMORY(pwszDomainName);
    VMAFD_SAFE_FREE_MEMORY(pwszPNID);

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcEnableDefaultHA(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = CDC_IPC_ENABLE_DEFAULT_HA;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = CdcSrvEnableDefaultHA(gVmafdGlobals.pCdcContext);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcEnableLegacyModeHA(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = CDC_IPC_ENABLE_LEGACY_HA;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = CdcSrvEnableLegacyModeHA(gVmafdGlobals.pCdcContext);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcGetDCName(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    PCDC_DC_INFO_W pAffinitizedDC = NULL;
    UINT32 uResult = 0;
    UINT32 apiType = CDC_IPC_GET_DC_NAME;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDomainName = NULL;
    DWORD dwFlags = FALSE;
    VMW_TYPE_SPEC input_spec[] = GET_CDC_NAME_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_CDC_NAME_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszDomainName = input_spec[0].data.pWString;

    dwFlags = *input_spec[3].data.pUint32;

    uResult = CdcSrvGetDCName(pwszDomainName, dwFlags, &pAffinitizedDC);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    if (pAffinitizedDC)
    {
        output_spec[1].data.pWString = pAffinitizedDC->pszDCName;
        output_spec[4].data.pWString = pAffinitizedDC->pszDomainName;
        output_spec[5].data.pWString = pAffinitizedDC->pszDcSiteName;
    }

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);

    if (pAffinitizedDC)
    {
        VmAfdFreeDomainControllerInfoW(pAffinitizedDC);
    }

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcGetCurrentState(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = CDC_IPC_GET_CDC_STATE;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_STATE_OUTPUT_PARAMS;
    CDC_DC_STATE cdcState = CDC_DC_STATE_UNDEFINED;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        NULL
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    uResult = CdcSrvGetCurrentState(&cdcState);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = (PUINT32)&cdcState;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcEnumDCEntries(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = CDC_IPC_ENUM_DC_ENTRIES;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PWSTR *pwszDCEntriesArray = NULL;
        DWORD dwDCEntriesCount = 0;
        PBYTE pDCEntriesArrayBlob = NULL;
        DWORD dwDCEntriesBlobSize = 0;

        VMW_TYPE_SPEC output_spec[] = ENUM_STORE_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        uResult = CdcSrvEnumDCEntries (
                                        &pwszDCEntriesArray,
                                        &dwDCEntriesCount
                                       );
        LOG_URESULT_ERROR(uResult);

        dwError = VmAfdMarshalStringArrayGetSize (
                                                  pwszDCEntriesArray,
                                                  dwDCEntriesCount,
                                                  &dwDCEntriesBlobSize
                                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                        dwDCEntriesBlobSize,
                                        (PVOID *)&pDCEntriesArrayBlob
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdMarshalStringArray (
                                           pwszDCEntriesArray,
                                           dwDCEntriesCount,
                                           dwDCEntriesBlobSize,
                                           pDCEntriesArrayBlob
                                        );

        BAIL_ON_VMAFD_ERROR (dwError);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pByte = (PBYTE) pDCEntriesArrayBlob;

        dwError = VecsMarshalResponse (
                                        apiType,
                                        output_spec,
                                        noOfArgsOut,
                                        &pResponse,
                                        &dwResponseSize
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdFreeStringArrayW (pwszDCEntriesArray, dwDCEntriesCount);

        VMAFD_SAFE_FREE_MEMORY (pDCEntriesArrayBlob);


        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}

DWORD
CdcIpcGetDCStatusInfo(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = CDC_IPC_GET_DC_STATUS_INFO;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszDCName = NULL;
    PWSTR pwszDomainName = NULL;

    PCDC_DC_STATUS_INFO_W pDCStatusInfo = NULL;
    PVMAFD_HB_STATUS_W pHbStatus = NULL;
    DWORD dwHeartbeatStatusBlobSize = 0;
    PBYTE pHeartbeatStatusBlob = NULL;

    VMW_TYPE_SPEC input_spec[] = GET_CDC_STATUS_INFO_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_CDC_STATUS_INFO_OUTPUT_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszDCName = input_spec[0].data.pWString;
    pwszDomainName = input_spec[1].data.pWString;


    uResult = CdcSrvGetDCStatusInfo(
                          pwszDCName,
                          pwszDomainName,
                          &pDCStatusInfo,
                          &pHbStatus
                          );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    if (pDCStatusInfo)
    {
        output_spec[1].data.pUint32 = &pDCStatusInfo->dwLastPing;
        output_spec[2].data.pUint32 = &pDCStatusInfo->dwLastResponseTime;
        output_spec[3].data.pUint32 = &pDCStatusInfo->dwLastError;
        output_spec[4].data.pUint32 = &pDCStatusInfo->bIsAlive;
        output_spec[5].data.pWString = pDCStatusInfo->pwszSiteName;
    }

    if (pHbStatus)
    {
            dwError = VmAfdMarshalHeartbeatStatusArrLength (
                                              pHbStatus->pHeartbeatInfoArr,
                                              pHbStatus->dwCount,
                                              &dwHeartbeatStatusBlobSize
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            dwError = VmAfdAllocateMemory (
                                    dwHeartbeatStatusBlobSize,
                                    (PVOID *)&pHeartbeatStatusBlob
                                    );
            BAIL_ON_VMAFD_ERROR (dwError);

            dwError = VmAfdMarshalHeartbeatStatusArray (
                                       pHbStatus->pHeartbeatInfoArr,
                                       pHbStatus->dwCount,
                                       dwHeartbeatStatusBlobSize,
                                       pHeartbeatStatusBlob
                                    );

            BAIL_ON_VMAFD_ERROR (dwError);

            output_spec[6].data.pByte = (PBYTE) pHeartbeatStatusBlob;
    }


    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;


    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VMAFD_SAFE_FREE_MEMORY(pHeartbeatStatusBlob);

    if (pDCStatusInfo)
    {
        VmAfdFreeCdcStatusInfoW(pDCStatusInfo);
    }
    if (pHbStatus)
    {
        VmAfdFreeHbStatusW(pHbStatus);
    }
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}


DWORD
VmAfdIpcPostHeartbeatStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_POST_HEARTBEAT;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszServiceName = NULL;
    DWORD dwPort = 0;
    VMW_TYPE_SPEC input_spec[] = POST_HEARTBEAT_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszServiceName = input_spec[0].data.pWString;

    dwPort = *input_spec[1].data.pUint32;


    uResult = VmAfSrvPostHeartbeat(pwszServiceName, dwPort);
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;


    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}


DWORD
VmAfdIpcGetHeartbeatStatus(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        UINT32 uResult = 0;
        UINT32 apiType = VMAFD_IPC_GET_HEARBEAT_STATUS;
        DWORD noOfArgsIn = 0;
        DWORD noOfArgsOut = 0;
        PBYTE pResponse = NULL;
        DWORD dwResponseSize = 0;
        PVMAFD_HB_STATUS_W pHeartbeatStatus = NULL;
        PBYTE pHeartbeatStatusBlob = NULL;
        DWORD dwHeartbeatStatusBlobSize = 0;

        VMW_TYPE_SPEC output_spec[] = GET_HEARTBEAT_STATUS_OUTPUT_PARAMS;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

        if (!pConnectionContext)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

        dwError = VmAfdUnMarshal (
                              apiType,
                              VER1_INPUT,
                              noOfArgsIn,
                              pRequest,
                              dwRequestSize,
                              NULL
                              );
        BAIL_ON_VMAFD_ERROR (dwError);


        uResult = VmAfSrvGetHeartbeatStatus (&pHeartbeatStatus);

        LOG_URESULT_ERROR(uResult);

        output_spec[0].data.pUint32 = &uResult;
        output_spec[1].data.pUint32 = &pHeartbeatStatus->bIsAlive;

        if (pHeartbeatStatus)
        {
                dwError = VmAfdMarshalHeartbeatStatusArrLength (
                                                  pHeartbeatStatus->pHeartbeatInfoArr,
                                                  pHeartbeatStatus->dwCount,
                                                  &dwHeartbeatStatusBlobSize
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwError = VmAfdAllocateMemory (
                                        dwHeartbeatStatusBlobSize,
                                        (PVOID *)&pHeartbeatStatusBlob
                                        );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwError = VmAfdMarshalHeartbeatStatusArray (
                                           pHeartbeatStatus->pHeartbeatInfoArr,
                                           pHeartbeatStatus->dwCount,
                                           dwHeartbeatStatusBlobSize,
                                           pHeartbeatStatusBlob
                                        );

                BAIL_ON_VMAFD_ERROR (dwError);

                output_spec[2].data.pByte = (PBYTE) pHeartbeatStatusBlob;
        }

        dwError = VecsMarshalResponse (
                                        apiType,
                                        output_spec,
                                        noOfArgsOut,
                                        &pResponse,
                                        &dwResponseSize
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdFreeHbStatusW (pHeartbeatStatus);

        VMAFD_SAFE_FREE_MEMORY (pHeartbeatStatusBlob);


        VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
        dwError = 0;
    goto cleanup;
}

DWORD
VmAfdIpcChangePNID(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE * ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_CHANGE_PNID;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PWSTR pwszUserName = NULL;
    PWSTR pwszPassword = NULL;
    PWSTR pwszPNID = NULL;
    VMW_TYPE_SPEC input_spec[] = CHANGE_PNID_INPUT_PARAMS;
    VMW_TYPE_SPEC output_spec[] = RESPONSE_PARAMS;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);

    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    //
    // Unmarshall the request buffer to the format
    // that the API actually has
    //
    noOfArgsIn = sizeof (input_spec) / sizeof (VMW_TYPE_SPEC);
    noOfArgsOut = sizeof (output_spec) / sizeof (VMW_TYPE_SPEC);
    dwError = VmAfdUnMarshal (
                        apiType,
                        VER1_INPUT,
                        noOfArgsIn,
                        pRequest,
                        dwRequestSize,
                        input_spec
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    pwszUserName    = input_spec[0].data.pWString;
    pwszPassword    = input_spec[1].data.pWString;
    pwszPNID        = input_spec[2].data.pWString;

    if (!VmAfdIsRootSecurityContext(pConnectionContext))
    {
        VmAfdLog (VMAFD_DEBUG_ANY, "%s: Access Denied", __FUNCTION__);
        dwError = ERROR_ACCESS_DENIED;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    uResult = VmAfSrvChangePNID(
                      pwszUserName,
                      pwszPassword,
                      pwszPNID
                      );
    LOG_URESULT_ERROR(uResult);

    // Allocate a buffer, marshall the response
    //
    output_spec[0].data.pUint32 = &uResult;

    dwError = VecsMarshalResponse(
                            apiType,
                            output_spec,
                            noOfArgsOut,
                            &pResponse,
                            &dwResponseSize
                            );
    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdFreeTypeSpecContent (input_spec, noOfArgsIn);
    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;

error:
    VmAfdHandleError(
            apiType,
            dwError,
            output_spec,
            noOfArgsOut,
            &pResponse,
            &dwResponseSize
            );
    dwError = 0;
    goto cleanup;
}


DWORD
VmAfdIpcGetDCList(
    PVM_AFD_CONNECTION_CONTEXT pConnectionContext,
    PBYTE pRequest,
    DWORD dwRequestSize,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
    DWORD dwError = 0;
    UINT32 uResult = 0;
    UINT32 apiType = VMAFD_IPC_GET_DC_LIST;
    DWORD noOfArgsIn = 0;
    DWORD noOfArgsOut = 0;
    PBYTE pResponse = NULL;
    DWORD dwResponseSize = 0;
    PSTR pszDomain = NULL;
    DWORD dwSizeRequired = 0;
    VMW_TYPE_SPEC input_spec[] = GET_DOMAIN_NAME_LIST_PARAMS;
    VMW_TYPE_SPEC output_spec[] = GET_DOMAIN_LIST_OUTPUT_PARAMS;
    PVMAFD_DC_INFO_W pVmAfdDCInfoList = NULL;
    PBYTE pDCListResponseBlob = NULL;
    DWORD dwCount = 0;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "Entering %s", __FUNCTION__);
    if (!pConnectionContext)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    noOfArgsIn = sizeof (input_spec) / sizeof (input_spec[0]);
    noOfArgsOut = sizeof (output_spec) / sizeof (output_spec[0]);

    dwError = VmAfdUnMarshal (
                         apiType,
                         VER1_INPUT,
                         noOfArgsIn,
                         pRequest,
                         dwRequestSize,
                         input_spec
                         );
    BAIL_ON_VMAFD_ERROR (dwError);

    pszDomain   = input_spec[0].data.pString;
    uResult = VmAfdGetDomainControllerList(
                        pszDomain,
                        &pVmAfdDCInfoList,
                        &dwCount
			);
    LOG_URESULT_ERROR(uResult);


    output_spec[0].data.pUint32 = &uResult;
    output_spec[1].data.pUint32 = &dwCount;

    dwError = VmAfdMarshalGetDCListArrLength(
                        pVmAfdDCInfoList,
                        dwCount,
                        &dwSizeRequired
                        );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdAllocateMemory (
                        dwSizeRequired,
                        (PVOID *) &pDCListResponseBlob
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdMarshalGetDCList(
                         dwCount,
                         pVmAfdDCInfoList,
                         dwSizeRequired,
                         pDCListResponseBlob
                       );
    BAIL_ON_VMAFD_ERROR(dwError);

    output_spec[2].data.pByte   = pDCListResponseBlob;
    output_spec[3].data.pUint32 = &dwSizeRequired;

    dwError = VecsMarshalResponse(
                       apiType,
                       output_spec,
                       noOfArgsOut,
                       &pResponse,
                       &dwResponseSize
                       );

    BAIL_ON_VMAFD_ERROR (dwError);

cleanup:
    *ppResponse = pResponse;
    *pdwResponseSize = dwResponseSize;

    VmAfdLog (VMAFD_DEBUG_DEBUG, "End of %s", __FUNCTION__);
    return dwError;
error:
        VmAfdHandleError(
                apiType,
                dwError,
                output_spec,
                noOfArgsOut,
                &pResponse,
                &dwResponseSize
                );
    goto cleanup;
}


static
VOID
VmAfdHandleError(
    UINT32 apiType,
    DWORD dwInputError,
    VMW_TYPE_SPEC *output_spec,
    DWORD noOfArgsOut,
    PBYTE *ppResponse,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        DWORD dwResponseSize = 0;
        PBYTE pResponse = NULL;


        output_spec[0].data.pUint32 = &dwInputError;

        dwError = VmAfdGetMarshalLength(
                                output_spec,
                                noOfArgsOut,
                                &dwResponseSize
                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory(
                                dwResponseSize,
                                (PVOID *) &pResponse
                                );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdMarshal(
                           apiType,
                           VER1_OUTPUT,
                           noOfArgsOut,
                           output_spec,
                           pResponse,
                           dwResponseSize
                           );
        BAIL_ON_VMAFD_ERROR (dwError);

        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

cleanup:

        return;
error:
        if (ppResponse)
        {
                *ppResponse = NULL;
        }
        if (pdwResponseSize)
        {
                *pdwResponseSize = 0;
        }
        if (pResponse)
        {
                VMAFD_SAFE_FREE_MEMORY (pResponse);
        }

        goto cleanup;
}

static
DWORD
VecsMarshalResponse (
                     UINT32 apiType,
                     VMW_TYPE_SPEC *output_spec,
                     DWORD noOfArgsOut,
                     PBYTE *ppResponse,
                     PDWORD pdwResponseSize
                    )
{
        DWORD dwError = 0;
        DWORD dwResponseSize = 0;
        PBYTE pResponse = NULL;

        dwError = VmAfdGetMarshalLength (
            output_spec,
            noOfArgsOut,
            &dwResponseSize
            );
    BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                        dwResponseSize,
                        (PVOID *) &pResponse
                        );
        BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdMarshal (
            apiType,
            VER1_OUTPUT,
            noOfArgsOut,
            output_spec,
            pResponse,
            dwResponseSize
            );
    BAIL_ON_VMAFD_ERROR(dwError);

        *ppResponse = pResponse;
        *pdwResponseSize = dwResponseSize;

        VmAfdLog (VMAFD_DEBUG_DEBUG, "Marshaling the result: Successful");
cleanup:
        return dwError;

error:
        if (ppResponse)
        {
                *ppResponse = NULL;
        }
        if (pdwResponseSize)
        {
                *pdwResponseSize = 0;
        }

        VMAFD_SAFE_FREE_MEMORY (pResponse);

        goto cleanup;
}
