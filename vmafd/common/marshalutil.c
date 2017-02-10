/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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
static
DWORD
VmAfdCheckMemory(
        size_t size,
        PDWORD pdwResponseSize
        );

static
DWORD
VmAfdCheckType(
                UINT32 uType,
                VMW_IPC_TYPE type
              );

static
DWORD
VmAfdMarshalUINT32 (
                    UINT32 uData,
                    PBYTE pMarshalledBlob,
                    DWORD dwBlobSize,
                    PDWORD pdwBytesUsed
                   );
static
DWORD
VmAfdUnMarshalUINT32 (
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PUINT32 puData,
                      PDWORD pdwBytesUsed
                     );

static
DWORD
VmAfdMarshalStringW (
                      PWSTR pwszString,
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PDWORD pdwBytesUsed
                    );

static
DWORD
VmAfdUnMarshalString (
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PWSTR *ppwszString,
                      PDWORD pdwBytesUsed
                    );


DWORD
VmAfdGetMarshalLength(
	PVMW_TYPE_SPEC pInput,
	DWORD noOfArgs,
	PDWORD pdwResponseSize
	)
{
	DWORD dwError = 0;
	DWORD iArgCounter = 0;
        DWORD dwResponseSize = 0;
        DWORD dwBlobSize = 0;

	if (pdwResponseSize == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}


	dwResponseSize = 2* sizeof(UINT32);
    //For API Type and Version, both of
    //which are of type UINT32
	dwResponseSize += sizeof (DWORD);
    //For NoofArgs
	while (iArgCounter < noOfArgs){
		UINT32 type = pInput[iArgCounter].type;
		PSTR tempString = NULL;
		WCHAR * tempWString = NULL;
                size_t stringLength = 0;
		switch (type){
			case VMW_IPC_TYPE_UINT32:
				dwResponseSize += sizeof (UINT32);
				dwResponseSize += sizeof (UINT32);
				break;
			case VMW_IPC_TYPE_STRING:
				dwResponseSize += sizeof (UINT32);
				tempString = pInput[iArgCounter].data.pString;
				dwResponseSize += sizeof (VMAFD_IPC_SIZE_T);
				if (tempString != NULL){
				  dwResponseSize += strlen(tempString);
				}
				break;
			case VMW_IPC_TYPE_WSTRING:
				dwResponseSize += sizeof(UINT32);
				tempWString = pInput[iArgCounter].data.pWString;
				dwResponseSize += sizeof (VMAFD_IPC_SIZE_T);
				if (tempWString != NULL){
                                        dwError = VmAfdGetStringLengthW (tempWString, &stringLength);
                                        BAIL_ON_VMAFD_ERROR (dwError);
					dwResponseSize += stringLength*sizeof(WCHAR);
				}
				break;
                        case VMW_IPC_TYPE_BLOB:
                                dwBlobSize = sizeof (UINT32);
                                dwResponseSize += sizeof (UINT32);
                                if (pInput[iArgCounter].data.pUint32)
                                {
                                        dwBlobSize = *pInput[iArgCounter].data.pUint32;
                                }
                                dwResponseSize += dwBlobSize;
                                break;
                        case VMW_IPC_TYPE_BLOB_SIZE:
                                dwResponseSize += sizeof (UINT32);
                                break;
                        case VMW_IPC_TYPE_BOOL:
                                dwResponseSize += sizeof (UINT32);
                                dwResponseSize += sizeof (BOOL);
                                break;
                        case VMW_IPC_TYPE_BOOLEAN:
                                dwResponseSize += sizeof (UINT32);
                                dwResponseSize += sizeof (BOOLEAN);
                                break;
			default:
				dwError = ERROR_INVALID_PARAMETER;
				BAIL_ON_VMAFD_ERROR (dwError);
		}
		iArgCounter++;
	}
        *pdwResponseSize = dwResponseSize;
cleanup:
	return dwError;
error:
	dwResponseSize = 0;
    if (pdwResponseSize != NULL){
        *pdwResponseSize = dwResponseSize;
    }
	goto cleanup;
}

DWORD
VmAfdMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PVMW_TYPE_SPEC pInput,
	PBYTE pResponse,
	DWORD dwResponseSize
	)
{
	DWORD dwError = 0;
	DWORD iArgCounter = 0;
	PBYTE pCursor = NULL;
	PBYTE pCursorStart = NULL;
	PVOID memGet = NULL;
	DWORD dwActualResponseSz = 0;
        DWORD dwBlobSize = 0;


	if (pResponse == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}

	dwError = VmAfdAllocateMemory (dwResponseSize, &memGet);
	BAIL_ON_VMAFD_ERROR (dwError);

	pCursor = (PBYTE)memGet;

	pCursorStart = pCursor;
        dwActualResponseSz = dwResponseSize;

        dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
        BAIL_ON_VMAFD_ERROR (dwError);
	*((UINT32 *)pCursor) = apiType;
	pCursor += sizeof(UINT32);
        dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
        BAIL_ON_VMAFD_ERROR (dwError);
	*((UINT32 *)pCursor) = apiVersion;
	pCursor += sizeof(UINT32);
        dwError = VmAfdCheckMemory (sizeof (DWORD), &dwActualResponseSz);
        BAIL_ON_VMAFD_ERROR (dwError);
	*((DWORD *)pCursor) = noOfArgs;
	pCursor += sizeof(DWORD);
	while (iArgCounter < noOfArgs){
		size_t stringLength = 0;
		UINT32 type = pInput[iArgCounter].type;
		PSTR tempString = NULL;
		WCHAR * tempWString = NULL;
		switch (type){
			case VMW_IPC_TYPE_UINT32:
                                dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_UINT32;
				pCursor += sizeof (UINT32);
                                dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
                                if (pInput[iArgCounter].data.pUint32)
                                {
                                        *((UINT32 *)pCursor) = *(pInput[iArgCounter].data.pUint32);
                                }
				pCursor += sizeof (UINT32);
				break;
			case VMW_IPC_TYPE_STRING:
                                dwError = VmAfdCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_STRING;
				pCursor += sizeof (UINT32);
				tempString = pInput[iArgCounter].data.pString;
				if (tempString == NULL){
                                        dwError = VmAfdCheckMemory (sizeof (VMAFD_IPC_SIZE_T), &dwActualResponseSz);
                                        BAIL_ON_VMAFD_ERROR (dwError);
					*((VMAFD_IPC_SIZE_T *)pCursor) = 0;
					pCursor += sizeof (VMAFD_IPC_SIZE_T);
					break;
				}
                                dwError = VmAfdCheckMemory (sizeof (VMAFD_IPC_SIZE_T), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				stringLength = strlen(tempString);
				*((VMAFD_IPC_SIZE_T *)pCursor) = (VMAFD_IPC_SIZE_T)stringLength;
				pCursor += sizeof (VMAFD_IPC_SIZE_T);
                                dwError = VmAfdCheckMemory (stringLength, &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				strncpy (pCursor, tempString, stringLength);
				pCursor += stringLength;
				break;
			case VMW_IPC_TYPE_WSTRING:
                                dwError = VmAfdCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_WSTRING;
				pCursor += sizeof(UINT32);
				tempWString = pInput[iArgCounter].data.pWString;
				if (tempWString == NULL){
                                        dwError = VmAfdCheckMemory (sizeof(VMAFD_IPC_SIZE_T), &dwActualResponseSz);
                                        BAIL_ON_VMAFD_ERROR (dwError);
					*((VMAFD_IPC_SIZE_T *)pCursor) = 0;
					pCursor += sizeof (VMAFD_IPC_SIZE_T);
					break;
				}
				dwError = VmAfdGetStringLengthW(tempWString, &stringLength);
				BAIL_ON_VMAFD_ERROR (dwError);
                                dwError = VmAfdCheckMemory (sizeof(VMAFD_IPC_SIZE_T), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((VMAFD_IPC_SIZE_T *)pCursor) = (VMAFD_IPC_SIZE_T)stringLength;
				pCursor += sizeof (VMAFD_IPC_SIZE_T);
                                dwError = VmAfdCheckMemory (sizeof(WCHAR)*stringLength, &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
                                memcpy (pCursor, tempWString, stringLength*sizeof(WCHAR));
				pCursor += stringLength*sizeof(WCHAR);
				break;
                        case VMW_IPC_TYPE_BLOB:
                                dwError =  VmAfdCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);

                                *((UINT32 *)pCursor) = VMW_IPC_TYPE_BLOB;
                                pCursor += sizeof (UINT32);

                                if (!pInput[iArgCounter].data.pUint32)
                                {
                                        dwBlobSize = sizeof (UINT32);
                                        dwError = VmAfdCheckMemory(dwBlobSize, &dwActualResponseSz);
                                        BAIL_ON_VMAFD_ERROR (dwError);
                                        *((PUINT32)pCursor) = sizeof (UINT32);
                                        pCursor += sizeof (UINT32);
                                        break;
                                }

                                dwBlobSize = *(pInput[iArgCounter].data.pUint32);

                                dwError = VmAfdCheckMemory (dwBlobSize, &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);

                                pCursor = memcpy (
                                                (PVOID) pCursor,
                                                (PVOID) pInput[iArgCounter].data.pByte,
                                                dwBlobSize
                                                );

				pCursor += dwBlobSize;
				break;
                        case VMW_IPC_TYPE_BLOB_SIZE:
                                dwError = VmAfdCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);

                                *((UINT32 *)pCursor) = VMW_IPC_TYPE_BLOB_SIZE;
                                pCursor += sizeof(UINT32);
                                break;
                       case VMW_IPC_TYPE_BOOL:
                                dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_BOOL;
				pCursor += sizeof (UINT32);
                                dwError = VmAfdCheckMemory (sizeof(BOOL), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
                                if(pInput[iArgCounter].data.pBool)
                                {
                                        *((BOOL *)pCursor) = *(pInput[iArgCounter].data.pBool);
                                }
				pCursor += sizeof (BOOL);
				break;
                      case VMW_IPC_TYPE_BOOLEAN:
                                dwError = VmAfdCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_BOOLEAN;
				pCursor += sizeof (UINT32);
                                dwError = VmAfdCheckMemory (sizeof(BOOLEAN), &dwActualResponseSz);
                                BAIL_ON_VMAFD_ERROR (dwError);
                                if (pInput[iArgCounter].data.pBoolean)
                                {
                                        *((BOOLEAN *)pCursor) = *(pInput[iArgCounter].data.pBoolean);
                                }
				pCursor += sizeof (BOOLEAN);
				break;

			default:
				dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMAFD_ERROR (dwError);
		}
		iArgCounter++;
	}
        pCursor = pCursorStart;

        memcpy (pResponse, pCursorStart, dwResponseSize);


cleanup:
        VMAFD_SAFE_FREE_MEMORY (memGet);
	return dwError;
error:
	goto cleanup;

}


DWORD
VmAfdUnMarshal(
	UINT32 apiType,
	UINT32 apiVersion,
	DWORD noOfArgs,
	PBYTE pResponse,
	DWORD dwResponseSize,
	PVMW_TYPE_SPEC pInput)
{
	DWORD dwError = 0;
	DWORD iArgCounter = 0;
	DWORD noOfArgsInput = 0;
	UINT32 apiTypeInput = 0;
	UINT32 apiVersionInput = 0;
	DWORD dwBytesRead = 0;
        DWORD dwBlobSize = 0;

	if (dwResponseSize == 0){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	if (pResponse == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}

	apiTypeInput = *((UINT32 *) pResponse);
	pResponse += sizeof(UINT32);
	apiVersionInput = *((UINT32 *)pResponse);
	pResponse += sizeof(UINT32);
	noOfArgsInput = *((DWORD *) pResponse);
	pResponse += sizeof (DWORD);

	if ((apiTypeInput != apiType) || (apiVersionInput != apiVersion) || (noOfArgs != noOfArgsInput)){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	dwBytesRead = 2* sizeof (UINT32) + sizeof (DWORD);

	while (iArgCounter < noOfArgs){

		UINT32 typeInput = *((UINT32 *)pResponse);
		size_t stringLength = 0;
		PSTR pTempString = NULL;
                PBYTE pTempByte = NULL;
                PWSTR pTempWString = NULL;
		pResponse += sizeof(UINT32);
		dwBytesRead += sizeof (UINT32);
		if (typeInput != pInput[iArgCounter].type){
			dwError = ERROR_INVALID_PARAMETER;
			BAIL_ON_VMAFD_ERROR (dwError);
		}
		switch (typeInput){
			case VMW_IPC_TYPE_UINT32:
				if (pInput[iArgCounter].data.pUint32 == NULL){
					dwError = VmAfdAllocateMemory(sizeof(UINT32),(PVOID *)&(pInput[iArgCounter].data.pUint32));
					BAIL_ON_VMAFD_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pUint32) = *((PUINT32)pResponse);
				pResponse += sizeof(UINT32);
				dwBytesRead += sizeof (UINT32);
				break;
			case VMW_IPC_TYPE_STRING:
				stringLength = *((VMAFD_IPC_SIZE_T *)pResponse);
				pResponse += sizeof (VMAFD_IPC_SIZE_T);
				dwBytesRead += sizeof (VMAFD_IPC_SIZE_T);
				if (stringLength == 0){
					pInput[iArgCounter].data.pString = NULL;
					break;
				}
				dwError = VmAfdAllocateMemory (stringLength+1,(PVOID *)&pTempString);
				BAIL_ON_VMAFD_ERROR (dwError);

				strncpy(pTempString,pResponse,stringLength);
				pTempString[stringLength] = '\0';
				pResponse += stringLength;
				dwBytesRead += stringLength;
				pInput[iArgCounter].data.pString = pTempString;
				break;
			case VMW_IPC_TYPE_WSTRING:
				stringLength = *((VMAFD_IPC_SIZE_T *)pResponse);
				pResponse += sizeof (VMAFD_IPC_SIZE_T);
				dwBytesRead += sizeof (VMAFD_IPC_SIZE_T);
				if (stringLength == 0){
					pInput[iArgCounter].data.pWString = NULL;
					break;
				}
				dwError = VmAfdAllocateMemory ((stringLength+1)*sizeof(WCHAR),(PVOID *)&pTempWString);
				BAIL_ON_VMAFD_ERROR (dwError);
                                memcpy (pTempWString, pResponse, stringLength*sizeof(WCHAR));
				memset ((PVOID)&pTempWString[stringLength], 0, sizeof(WCHAR));
				pResponse += stringLength * sizeof (WCHAR);
				dwBytesRead += stringLength * sizeof(WCHAR);
                                pInput[iArgCounter].data.pWString = pTempWString;
				break;

                        case VMW_IPC_TYPE_BLOB:
                                dwBlobSize = *(PUINT32)pResponse;
                                dwError = VmAfdAllocateMemory (
                                                dwBlobSize,
                                                (PVOID *) &pTempByte
                                                );
                                BAIL_ON_VMAFD_ERROR (dwError);

                                memcpy (pTempByte, pResponse, dwBlobSize);

                                pResponse += dwBlobSize;
                                dwBytesRead += dwBlobSize;
                                pInput[iArgCounter].data.pByte = pTempByte;
                                break;
                        case VMW_IPC_TYPE_BLOB_SIZE:
                                if (pInput[iArgCounter].data.pUint32 == NULL)
                                {
                                        dwError = VmAfdAllocateMemory (
                                                                sizeof (UINT32),
                                                                (PVOID *) &pInput[iArgCounter].data.pUint32
                                                                );
                                        BAIL_ON_VMAFD_ERROR (dwError);
                                }
                                *(pInput[iArgCounter].data.pUint32) = dwBlobSize;
                                dwBlobSize = 0;
                                break;
                        case VMW_IPC_TYPE_BOOL:
				if (pInput[iArgCounter].data.pBool == NULL){
					dwError = VmAfdAllocateMemory(sizeof(BOOL),(PVOID *)&(pInput[iArgCounter].data.pBool));
					BAIL_ON_VMAFD_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pBool) = *((PBOOL)pResponse);
				pResponse += sizeof(BOOL);
				dwBytesRead += sizeof (BOOL);
				break;
                       case VMW_IPC_TYPE_BOOLEAN:
				if (pInput[iArgCounter].data.pBoolean == NULL){
					dwError = VmAfdAllocateMemory(sizeof(BOOLEAN),(PVOID *)&(pInput[iArgCounter].data.pBoolean));
					BAIL_ON_VMAFD_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pBoolean) = *((PBOOLEAN)pResponse);
				pResponse += sizeof(BOOLEAN);
				dwBytesRead += sizeof (BOOLEAN);
				break;

			default:
                                dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMAFD_ERROR (dwError);
				break;
		}
		iArgCounter++;
	}

cleanup:
	return dwError;
error:
	goto cleanup;
}

DWORD
VmAfdMarshalStringArrayGetSize (
                               PWSTR *pwszStringArray,
                               DWORD dwArraySize,
                               PDWORD pdwSizeRequired
                              )
{
        DWORD dwError = 0;
        DWORD dwSizeRequired = 0;
        DWORD dwIndex = 0;

        if (!pdwSizeRequired)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (dwArraySize && !pwszStringArray)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwSizeRequired = 2 * sizeof (UINT32); //Size of the blob + No of Elements

        for (;dwIndex < dwArraySize; dwIndex++)
        {
                size_t stringLength = 0;
                dwSizeRequired += sizeof (VMAFD_IPC_SIZE_T); //Length of String

                if (pwszStringArray[dwIndex])
                {
                        dwError = VmAfdGetStringLengthW (
                                                         pwszStringArray[dwIndex],
                                                         &stringLength
                                                        );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        dwSizeRequired += stringLength*sizeof(WCHAR);
                }
        }

        *pdwSizeRequired = dwSizeRequired;

cleanup:

        return dwError;

error:
        if (pdwSizeRequired)
        {
                *pdwSizeRequired = 0;
        }

        goto cleanup;
}

DWORD
VmAfdMarshalStringArray (
                         PWSTR *pwszStringArray,
                         DWORD dwArraySize,
                         DWORD dwBlobSize,
                         PBYTE pMarshalledBlob
                        )
{
        DWORD dwError = 0;
        DWORD dwSizeRemainining = dwBlobSize;
        PVOID pTempMarshal = NULL;
        PBYTE pCursor = NULL;
        DWORD dwIndex = 0;

        if (!pMarshalledBlob)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (dwArraySize && !pwszStringArray)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VmAfdAllocateMemory (
                                        dwBlobSize,
                                        &pTempMarshal
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        pCursor = (PBYTE)pTempMarshal;

        dwError = VmAfdCheckMemory (
                                    sizeof (UINT32),
                                    &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        *((PUINT32)pCursor) = dwBlobSize;

        pCursor += sizeof (UINT32);

        dwError = VmAfdCheckMemory (
                                    sizeof (UINT32),
                                    &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        *((PUINT32)pCursor) = dwArraySize;
        pCursor += sizeof (UINT32);

        for (;dwIndex<dwArraySize; dwIndex++)
        {
                size_t stringLength = 0;

                if (pwszStringArray[dwIndex])
                {

                        dwError = VmAfdGetStringLengthW(
                                                        pwszStringArray[dwIndex],
                                                        &stringLength
                                                       );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        dwError = VmAfdCheckMemory (
                                                     sizeof (VMAFD_IPC_SIZE_T),
                                                     &dwSizeRemainining
                                                   );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        *((VMAFD_IPC_SIZE_T *)pCursor) = (VMAFD_IPC_SIZE_T)stringLength;
                        pCursor += sizeof (VMAFD_IPC_SIZE_T);

                        dwError = VmAfdCheckMemory (sizeof(WCHAR)*stringLength, &dwSizeRemainining);
                        BAIL_ON_VMAFD_ERROR (dwError);

                        memcpy (pCursor, pwszStringArray[dwIndex], stringLength*sizeof(WCHAR));
                        pCursor += stringLength*sizeof(WCHAR);
                }
                else
                {
                        dwError = VmAfdCheckMemory (
                                                        sizeof (VMAFD_IPC_SIZE_T),
                                                        &dwSizeRemainining
                                                   );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        *((VMAFD_IPC_SIZE_T *)pCursor) = (VMAFD_IPC_SIZE_T)stringLength;
                        pCursor += sizeof(VMAFD_IPC_SIZE_T);
                }
        }

        memcpy (pMarshalledBlob, pTempMarshal, dwBlobSize);

cleanup:
        VMAFD_SAFE_FREE_MEMORY (pTempMarshal);

        return dwError;

error:

        goto cleanup;
}

DWORD
VmAfdUnMarshalStringArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PWSTR **ppwszStringArray,
                           PDWORD pdwArraySize
                          )
{
        DWORD dwError = 0;
        PWSTR *pwszStringArray = NULL;
        DWORD dwArraySize = 0;
        PBYTE pCursor = pMarshalledBlob;
        DWORD dwBytesAvailable = dwBlobSize;
        DWORD dwBlobSizeRead = 0;
        DWORD dwIndex = 0;
        PWSTR pTempString = NULL;

        if (
            !pMarshalledBlob ||
            !ppwszStringArray ||
            !pdwArraySize
           )
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        if (dwBlobSize < (sizeof(UINT32) + sizeof(UINT32)))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VmAfdCheckMemory (
                                    sizeof (UINT32),
                                    &dwBytesAvailable
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);


        dwBlobSizeRead = *((PUINT32)pCursor);
        pCursor += sizeof (UINT32);

        if (dwBlobSizeRead != dwBlobSize)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        dwError = VmAfdCheckMemory (
                                    sizeof (UINT32),
                                    &dwBytesAvailable
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwArraySize = *((PUINT32)pCursor);
        pCursor += sizeof (UINT32);

        if (dwArraySize)
        {
                dwError = VmAfdAllocateMemory (
                                                dwArraySize * sizeof (PWSTR),
                                                (PVOID *) &pwszStringArray
                                              );
                BAIL_ON_VMAFD_ERROR (dwError);
        }

        for (; dwIndex < dwArraySize; dwIndex++)
        {
                size_t stringLength = 0;

                dwError = VmAfdCheckMemory (
                                             sizeof (VMAFD_IPC_SIZE_T),
                                             &dwBytesAvailable
                                           );
                BAIL_ON_VMAFD_ERROR (dwError);

                stringLength = *((VMAFD_IPC_SIZE_T *)pCursor);
                pCursor += sizeof (VMAFD_IPC_SIZE_T);

                if (stringLength)
                {
                        dwError = VmAfdCheckMemory (
                                                    sizeof(WCHAR) * stringLength,
                                                    &dwBytesAvailable
                                                   );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        dwError = VmAfdAllocateMemory (
                                                       (stringLength+1)*sizeof (WCHAR),
                                                       (PVOID *)&pTempString
                                                      );
                        BAIL_ON_VMAFD_ERROR (dwError);

                        memcpy (pTempString, pCursor, stringLength*sizeof(WCHAR));

                        pCursor += stringLength*sizeof (WCHAR);

                        memset ((PVOID)&pTempString[stringLength], 0, sizeof (WCHAR));

                        pwszStringArray[dwIndex] = pTempString;

                        pTempString = NULL;
                }
        }

        *ppwszStringArray = pwszStringArray;
        *pdwArraySize = dwArraySize;

cleanup:
        return dwError;

error:
        if (ppwszStringArray)
        {
                *ppwszStringArray = 0;
        }
        if (pdwArraySize)
        {
                *pdwArraySize = 0;
        }
        if (pwszStringArray)
        {
                VmAfdFreeStringArrayW(
                                      pwszStringArray,
                                      dwArraySize
                                     );
        }

        goto cleanup;
}

DWORD
VmAfdMarshalEntryArrayLength (
                             PVMAFD_CERT_ARRAY pCertArray,
                             PDWORD pdwSizeRequired
                             )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSizeRequired = 0;

    if (!pdwSizeRequired)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSizeRequired = sizeof (UINT32) + sizeof (UINT32);
    //For Size and Number of Elements

    for (;pCertArray && dwIndex < pCertArray->dwCount; dwIndex++)
    {
        size_t stringLength = 0;
        PVMAFD_CERT_CONTAINER pCertCursor = &pCertArray->certificates[dwIndex];

        if (pCertCursor)
        {
            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of entryType and the actual data

            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of dwDate and the actual dwDate

            dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);
            //For Storing type of pszAlias and the length of the alias

            if (pCertCursor->pAlias)
            {
                dwError = VmAfdGetStringLengthW (
                                                  pCertCursor->pAlias,
                                                  &stringLength
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwSizeRequired += stringLength*sizeof (WCHAR);
                stringLength = 0;
            }

            dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);
            //For storing type of pszCertificate and the lenght of cert

            if (pCertCursor->pCert)
            {
                dwError = VmAfdGetStringLengthW (
                                                  pCertCursor->pCert,
                                                  &stringLength
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwSizeRequired += stringLength * sizeof (WCHAR);
                stringLength = 0;
            }

            dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);
            //For storing type of pszKey and the length of key

            if (pCertCursor->pPassword)
            {
                dwError = VmAfdGetStringLengthW (
                                                  pCertCursor->pPassword,
                                                  &stringLength
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwSizeRequired += sizeof (UINT32) * sizeof (WCHAR);
            }
        }
    }

    *pdwSizeRequired = dwSizeRequired;

cleanup:
    return dwError;

error:
    if (pdwSizeRequired)
    {
        *pdwSizeRequired = 0;
    }

    goto cleanup;
}

DWORD
VmAfdMarshalEntryArray (
                        PVMAFD_CERT_ARRAY pCertArray,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                       )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    PBYTE pCursor = NULL;
    PBYTE pCursorStart = NULL;
    DWORD dwIndex = 0;

    if (!pMarshalledBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < (sizeof (UINT32) + sizeof (UINT32)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    dwBlobSize,
                                    (PVOID *)&pCursor
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCursorStart = pCursor;

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *((PUINT32)pCursor) = dwBlobSize;
    pCursor += sizeof (UINT32);

    if (pCertArray)
    {
        dwError = VmAfdCheckMemory (
                                      sizeof (UINT32),
                                      &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        *((PUINT32) pCursor) = pCertArray->dwCount;
        pCursor += sizeof (UINT32);

        for (; dwIndex < pCertArray->dwCount; dwIndex++)
        {
            PVMAFD_CERT_CONTAINER pCertCursor = &pCertArray->certificates[dwIndex];
            DWORD dwBytesToMove = 0;

            if (!pCertCursor)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
            }

            dwError = VmAfdMarshalUINT32 (
                                           pCertCursor->dwStoreType,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalUINT32 (
                                           pCertCursor->dwDate,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalStringW (
                                           pCertCursor->pAlias,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalStringW (
                                           pCertCursor->pCert,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalStringW (
                                           pCertCursor->pPassword,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;
        }
    }

    memcpy (
            pMarshalledBlob,
            pCursorStart,
            dwBlobSize
           );

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pCursorStart);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdUnMarshalEntryArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PVMAFD_CERT_ARRAY *ppCertArray
                         )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    DWORD dwIndex = 0;
    PVMAFD_CERT_ARRAY pCertArray = NULL;
    PVMAFD_CERT_CONTAINER pCertContainer = NULL;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwBlobSizeRead = 0;
    PWSTR pTempString = NULL;

    if (
        !pMarshalledBlob ||
        !ppCertArray
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < sizeof (UINT32) + sizeof (UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    sizeof (VMAFD_CERT_ARRAY),
                                    (PVOID *)&pCertArray
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwBlobSizeRead = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    if (dwBlobSizeRead != dwBlobSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                  sizeof (UINT32),
                                  &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCertArray->dwCount = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    if (pCertArray->dwCount)
    {
        dwError = VmAfdAllocateMemory (
                                        sizeof (VMAFD_CERT_CONTAINER) * pCertArray->dwCount,
                                        (PVOID *) &pCertContainer
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        for (; dwIndex < pCertArray->dwCount; dwIndex++)
        {
            DWORD dwBytesToMove = 0;

            dwError = VmAfdUnMarshalUINT32 (
                                                pCursor,
                                                dwSizeRemainining,
                                                &pCertContainer[dwIndex].dwStoreType,
                                                &dwBytesToMove
                                           );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalUINT32 (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pCertContainer[dwIndex].dwDate,
                                            &dwBytesToMove
                                           );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalString (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pCertContainer[dwIndex].pAlias,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalString (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pCertContainer[dwIndex].pCert,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalString (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pCertContainer[dwIndex].pPassword,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;
        }
    }

    pCertArray->certificates = pCertContainer;

    *ppCertArray = pCertArray;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pTempString);

    return dwError;

error:
    if (ppCertArray)
    {
        *ppCertArray = NULL;
    }

    if (pCertArray)
    {
      //TODO: Find or write a function to free cert array.
    }

    goto cleanup;
}

DWORD
VmAfdMarshalPermissionArrayLength (
                             PVECS_STORE_PERMISSION_W pPermArray,
                             DWORD dwCount,
                             PDWORD pdwSizeRequired
                             )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSizeRequired = 0;

    if (!pdwSizeRequired)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSizeRequired = sizeof (UINT32) + sizeof (UINT32);
    //For Size and Number of Elements

    for (;pPermArray && dwIndex < dwCount; dwIndex++)
    {
        size_t stringLength = 0;
        PVECS_STORE_PERMISSION_W pCursor = &pPermArray[dwIndex];

        if (pPermArray)
        {
            dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);
            //For Storing type of pszUserName and the length of the userName

            if (pCursor->pszUserName)
            {
                dwError = VmAfdGetStringLengthW (
                                                  pCursor->pszUserName,
                                                  &stringLength
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwSizeRequired += stringLength*sizeof (WCHAR);
                stringLength = 0;
            }

            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of dwAccessMask and the actual dwAccessMask
        }
    }

    *pdwSizeRequired = dwSizeRequired;

cleanup:
    return dwError;

error:
    if (pdwSizeRequired)
    {
        *pdwSizeRequired = 0;
    }

    goto cleanup;
}

DWORD
VmAfdMarshalPermissionArray (
                        PVECS_STORE_PERMISSION_W pPermArray,
                        DWORD dwCount,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                       )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    PBYTE pCursor = NULL;
    PBYTE pCursorStart = NULL;
    DWORD dwIndex = 0;

    if (!pMarshalledBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < (sizeof (UINT32) + sizeof (UINT32)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    dwBlobSize,
                                    (PVOID *)&pCursor
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCursorStart = pCursor;

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *((PUINT32)pCursor) = dwBlobSize;
    pCursor += sizeof (UINT32);

    if (pPermArray)
    {
        dwError = VmAfdCheckMemory (
                                      sizeof (UINT32),
                                      &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        *((PUINT32) pCursor) = dwCount;
        pCursor += sizeof (UINT32);

        for (; dwIndex < dwCount; dwIndex++)
        {
            PVECS_STORE_PERMISSION_W pPermCursor = &pPermArray[dwIndex];
            DWORD dwBytesToMove = 0;

            if (!pPermCursor)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
            }


            dwError = VmAfdMarshalStringW (
                                           pPermCursor->pszUserName,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalUINT32 (
                                           pPermCursor->dwAccessMask,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;
        }
    }

    memcpy (
            pMarshalledBlob,
            pCursorStart,
            dwBlobSize
           );

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pCursorStart);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdUnMarshalPermissionArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PDWORD pdwCount,
                           PVECS_STORE_PERMISSION_W *ppPermArray
                         )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    DWORD dwIndex = 0;
    PVECS_STORE_PERMISSION_W pPermArray = NULL;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwBlobSizeRead = 0;
    PWSTR pTempString = NULL;
    DWORD dwCount = 0;

    if (
        !pMarshalledBlob ||
        !ppPermArray
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < sizeof (UINT32) + sizeof (UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwBlobSizeRead = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    if (dwBlobSizeRead != dwBlobSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                  sizeof (UINT32),
                                  &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwCount = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);


    if (dwCount)
    {
        dwError = VmAfdAllocateMemory (
                                    dwCount*sizeof (VECS_STORE_PERMISSION_W),
                                    (PVOID *)&pPermArray
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        for (; dwIndex < dwCount; dwIndex++)
        {
            DWORD dwBytesToMove = 0;


            dwError = VmAfdUnMarshalString (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pPermArray[dwIndex].pszUserName,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;


            dwError = VmAfdUnMarshalUINT32 (
                                                pCursor,
                                                dwSizeRemainining,
                                                &pPermArray[dwIndex].dwAccessMask,
                                                &dwBytesToMove
                                           );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

         }
    }

    *ppPermArray = pPermArray;
    *pdwCount = dwCount;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pTempString);

    return dwError;

error:
    if (ppPermArray)
    {
        *ppPermArray = NULL;
    }

    if (pPermArray)
    {
        VmAfdFreeStorePermissionArray(
                          pPermArray,
                          dwCount
                          );
    }

    goto cleanup;
}

DWORD
VmAfdMarshalHeartbeatStatusArrLength (
                             PVMAFD_HB_INFO_W pInfoArray,
                             DWORD dwCount,
                             PDWORD pdwSizeRequired
                             )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSizeRequired = 0;

    if (!pdwSizeRequired)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwSizeRequired = sizeof (UINT32) + sizeof (UINT32);
    //For Size and Number of Elements

    for (;pInfoArray && dwIndex < dwCount; dwIndex++)
    {
        size_t stringLength = 0;
        PVMAFD_HB_INFO_W pCursor = &pInfoArray[dwIndex];

        if (pInfoArray)
        {
            dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);

            if (pCursor->pszServiceName)
            {
                dwError = VmAfdGetStringLengthW (
                                                  pCursor->pszServiceName,
                                                  &stringLength
                                                );
                BAIL_ON_VMAFD_ERROR (dwError);

                dwSizeRequired += stringLength*sizeof (WCHAR);
                stringLength = 0;
            }

            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of dwPort and the actual dwPort

            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of dwLastPing and the actual dwLastPing

            dwSizeRequired += sizeof (UINT32) + sizeof (UINT32);
            //For storing type of bIsAlive and the actual bIsAlive

        }
    }

    *pdwSizeRequired = dwSizeRequired;

cleanup:
    return dwError;

error:
    if (pdwSizeRequired)
    {
        *pdwSizeRequired = 0;
    }

    goto cleanup;
}

DWORD
VmAfdMarshalHeartbeatStatusArray (
                        PVMAFD_HB_INFO_W pInfoArray,
                        DWORD dwCount,
                        DWORD dwBlobSize,
                        PBYTE pMarshalledBlob
                       )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    PBYTE pCursor = NULL;
    PBYTE pCursorStart = NULL;
    DWORD dwIndex = 0;
    DWORD dwIsAlive = 0;

    if (!pMarshalledBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < (sizeof (UINT32) + sizeof (UINT32)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                    dwBlobSize,
                                    (PVOID *)&pCursor
                                  );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCursorStart = pCursor;

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCopyMemory(
                              pCursor,
                              sizeof(UINT32),
                              &dwBlobSize,
                              sizeof(dwBlobSize)
                             );
    BAIL_ON_VMAFD_ERROR(dwError);

    pCursor += sizeof (UINT32);

    if (pInfoArray)
    {
        dwError = VmAfdCheckMemory (
                                      sizeof (UINT32),
                                      &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdCopyMemory(
                                  pCursor,
                                  sizeof(UINT32),
                                  &dwCount,
                                  sizeof(UINT32)
                                 );
        BAIL_ON_VMAFD_ERROR(dwError);

        pCursor += sizeof (UINT32);

        for (; dwIndex < dwCount; dwIndex++)
        {
            PVMAFD_HB_INFO_W pInfoCursor = &pInfoArray[dwIndex];
            DWORD dwBytesToMove = 0;

            if (!pInfoCursor)
            {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMAFD_ERROR (dwError);
            }


            dwError = VmAfdMarshalStringW (
                                           pInfoCursor->pszServiceName,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                          );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalUINT32 (
                                           pInfoCursor->dwPort,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdMarshalUINT32 (
                                           pInfoCursor->dwLastHeartbeat,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwIsAlive = pInfoCursor->bIsAlive?1:0;

            dwError = VmAfdMarshalUINT32 (
                                           dwIsAlive,
                                           pCursor,
                                           dwSizeRemainining,
                                           &dwBytesToMove
                                         );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;
        }
    }

    memcpy (
            pMarshalledBlob,
            pCursorStart,
            dwBlobSize
           );

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pCursorStart);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmAfdUnMarshalHeartbeatStatusArray (
                           DWORD dwBlobSize,
                           PBYTE pMarshalledBlob,
                           PDWORD pdwCount,
                           PVMAFD_HB_INFO_W *ppInfoArray
                         )
{
    DWORD dwError = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    DWORD dwIndex = 0;
    PVMAFD_HB_INFO_W pInfoArray = NULL;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwBlobSizeRead = 0;
    PWSTR pTempString = NULL;
    DWORD dwCount = 0;
    DWORD dwIsAlive = 0;

    if (
        !pMarshalledBlob ||
        !ppInfoArray
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < sizeof (UINT32) + sizeof (UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwBlobSizeRead = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    if (dwBlobSizeRead != dwBlobSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                  sizeof (UINT32),
                                  &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwCount = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);


    if (dwCount)
    {
        dwError = VmAfdAllocateMemory (
                                    dwCount*sizeof (VMAFD_HB_INFO_W),
                                    (PVOID *)&pInfoArray
                                  );
        BAIL_ON_VMAFD_ERROR (dwError);

        for (; dwIndex < dwCount; dwIndex++)
        {
            DWORD dwBytesToMove = 0;


            dwError = VmAfdUnMarshalString (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pInfoArray[dwIndex].pszServiceName,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;


            dwError = VmAfdUnMarshalUINT32 (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pInfoArray[dwIndex].dwPort,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalUINT32 (
                                            pCursor,
                                            dwSizeRemainining,
                                            &pInfoArray[dwIndex].dwLastHeartbeat,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalUINT32 (
                                            pCursor,
                                            dwSizeRemainining,
                                            &dwIsAlive,
                                            &dwBytesToMove
                                            );
            BAIL_ON_VMAFD_ERROR (dwError);

            pCursor += dwBytesToMove;
            dwSizeRemainining -= dwBytesToMove;
            dwBytesToMove = 0;

            pInfoArray[dwIndex].bIsAlive = dwIsAlive? TRUE: FALSE;
         }
    }

    *ppInfoArray = pInfoArray;
    *pdwCount = dwCount;

cleanup:
    VMAFD_SAFE_FREE_MEMORY (pTempString);

    return dwError;

error:
    if (ppInfoArray)
    {
        *ppInfoArray = NULL;
    }

    if (pInfoArray)
    {
        VmAfdFreeHbInfoArrayW(pInfoArray, dwCount);
    }

    goto cleanup;
}

DWORD
VmAfdMarshalGetDCListArrLength(
    PVMAFD_DC_INFO_W pVmAfdDCInfoList,
    DWORD dwCount,
    PDWORD pdwSizeRequired
    )
{
    DWORD dwError = 0;
    DWORD dwIndex = 0;
    DWORD dwSizeRequired = 0;
    if (!pdwSizeRequired || !pVmAfdDCInfoList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR(dwError);
    }
    // Size and number of elements
    dwSizeRequired = sizeof(UINT32) + sizeof(UINT32);

    for ( ; dwIndex < dwCount ; dwIndex++)
    {
        size_t stringLength = 0;
        PVMAFD_DC_INFO_W pDCInfo = &pVmAfdDCInfoList[dwIndex];
        if (!pDCInfo)
        {
           dwError = ERROR_INVALID_PARAMETER;
           BAIL_ON_VMAFD_ERROR(dwError);
        }
        dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);

        dwError = VmAfdGetStringLengthW ( pDCInfo->pwszHostName,
                                          &stringLength
                                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwSizeRequired += stringLength * sizeof(WCHAR);
        stringLength = 0;

        dwSizeRequired += sizeof (UINT32) + sizeof (VMAFD_IPC_SIZE_T);

        dwError = VmAfdGetStringLengthW ( pDCInfo->pwszAddress,
                                          &stringLength
                                        );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwSizeRequired += stringLength * sizeof(WCHAR);
        stringLength = 0;
    }
    *pdwSizeRequired = dwSizeRequired;
cleanup:
    return dwError;
error:
    if (pdwSizeRequired)
    {
        *pdwSizeRequired = 0;
    }
    goto cleanup;
}

DWORD
VmAfdMarshalGetDCList(
    DWORD dwCount,
    PVMAFD_DC_INFO_W pVmAfdDCInfoList,
    DWORD dwBlobSize,
    PBYTE pMarshaledBlob
    )
{

    DWORD dwError = 0;
    DWORD dwSizeRemaining = dwBlobSize;
    PBYTE pCursor = NULL;
    PBYTE pCursorStart = NULL;
    DWORD dwIndex = 0;

    if (!pMarshaledBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if ( dwBlobSize < (sizeof(UINT32) + sizeof(UINT32)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdAllocateMemory (
                                dwBlobSize,
                                (PVOID *) &pCursor
                                );
    BAIL_ON_VMAFD_ERROR (dwError);

    pCursorStart = pCursor;

    dwError = VmAfdCheckMemory (
                               sizeof(UINT32),
                               &dwSizeRemaining
                               );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwError = VmAfdCopyMemory (
                              pCursor,
                              sizeof(UINT32),
                              &dwBlobSize,
                              sizeof(dwBlobSize)
                              );
    BAIL_ON_VMAFD_ERROR(dwError);

    pCursor += sizeof(UINT32);

    if (pVmAfdDCInfoList)
    {
        dwError = VmAfdCheckMemory (
                                   sizeof(UINT32),
                                   &dwSizeRemaining
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        dwError = VmAfdCopyMemory (
                                  pCursor,
                                  sizeof(UINT32),
                                  &dwCount,
                                  sizeof(dwCount)
                                  );
        BAIL_ON_VMAFD_ERROR(dwError);

        pCursor += sizeof(UINT32);

        for (; dwIndex < dwCount ; dwIndex++)
        {
           PVMAFD_DC_INFO_W pDCInfo = &pVmAfdDCInfoList[dwIndex];
           DWORD dwBytesToMove = 0;

           if (!pDCInfo)
           {
               dwError = ERROR_INVALID_PARAMETER;
               BAIL_ON_VMAFD_ERROR (dwError);
           }

           dwError = VmAfdMarshalStringW (
                                          pDCInfo->pwszHostName,
                                          pCursor,
                                          dwSizeRemaining,
                                          &dwBytesToMove
                                          );
           BAIL_ON_VMAFD_ERROR(dwError);

           pCursor += dwBytesToMove;
           dwSizeRemaining -= dwBytesToMove;
           dwBytesToMove = 0;

           dwError = VmAfdMarshalStringW (
                                          pDCInfo->pwszAddress,
                                          pCursor,
                                          dwSizeRemaining,
                                          &dwBytesToMove
                                          );
           BAIL_ON_VMAFD_ERROR(dwError);

           pCursor += dwBytesToMove;
           dwSizeRemaining -= dwBytesToMove;
           dwBytesToMove = 0;
        }

    }

    memcpy (
            pMarshaledBlob,
            pCursorStart,
            dwBlobSize
           );
cleanup:

    VMAFD_SAFE_FREE_MEMORY(pCursorStart);

    return dwError;
error:
    goto cleanup;
}

DWORD
VmAfdUnMarshalGetDCList(
    DWORD dwServerCount,
    DWORD dwBlobSize,
    PBYTE pMarshaledBlob,
    PVMAFD_DC_INFO_W *ppVmAfdDCInfoList
    )
{
    DWORD dwError = 0;
    DWORD dwSizeRemaining = dwBlobSize;
    DWORD dwIndex = 0;
    DWORD dwBlobSizeRead = 0;
    DWORD dwCount = 0;
    PVMAFD_DC_INFO_W pVmAfdDCInfo = NULL;
    PBYTE pCursor = pMarshaledBlob;

    if (!pMarshaledBlob || !ppVmAfdDCInfoList)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    if (dwBlobSize < sizeof(UINT32) + sizeof(UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                              sizeof(UINT32),
                              &dwSizeRemaining
                              );

    BAIL_ON_VMAFD_ERROR (dwError);

    dwBlobSizeRead = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    dwError = VmAfdCheckMemory(
                        sizeof(UINT32),
                        &dwSizeRemaining
                        );
    BAIL_ON_VMAFD_ERROR(dwError);

    dwCount = *((PUINT32)pCursor);
    pCursor += sizeof(UINT32);
    if (dwCount > 0 )
    {
        dwError = VmAfdAllocateMemory (
                         dwCount * sizeof(VMAFD_DC_INFO_W),
                         (PVOID *)&pVmAfdDCInfo
                         );
        BAIL_ON_VMAFD_ERROR(dwError);

        for (; dwIndex < dwCount; dwIndex++)
        {
            DWORD dwBytesToMove = 0;

            dwError = VmAfdUnMarshalString(
                                  pCursor,
                                  dwSizeRemaining,
                                  &pVmAfdDCInfo[dwIndex].pwszHostName,
                                  &dwBytesToMove
                                  );
            BAIL_ON_VMAFD_ERROR(dwError);

            pCursor += dwBytesToMove;
            dwSizeRemaining -= dwBytesToMove;
            dwBytesToMove = 0;

            dwError = VmAfdUnMarshalString(
                                  pCursor,
                                  dwSizeRemaining,
                                  &pVmAfdDCInfo[dwIndex].pwszAddress,
                                  &dwBytesToMove
                                  );
            BAIL_ON_VMAFD_ERROR(dwError);

            pCursor += dwBytesToMove;
            dwSizeRemaining -= dwBytesToMove;
            dwBytesToMove = 0;
        }
    }
    *ppVmAfdDCInfoList = pVmAfdDCInfo;
cleanup:

    return dwError;

error:
   VMAFD_SAFE_FREE_MEMORY(pVmAfdDCInfo);
   goto cleanup;

}

static
DWORD
VmAfdCheckMemory(
    size_t size,
    PDWORD pdwResponseSize
    )
{
    int sResponseSize = 0;
    DWORD dwError = 0;
    if (pdwResponseSize == NULL){
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    sResponseSize = *pdwResponseSize - size;
    if (sResponseSize < 0){
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMAFD_ERROR (dwError);
    }
    *pdwResponseSize = sResponseSize;
cleanup:
    return dwError;
error:
    goto cleanup;
}

static
DWORD
VmAfdCheckType(
                UINT32 uType,
                VMW_IPC_TYPE type
              )
{
    DWORD dwError = 0;

    if ((VMW_IPC_TYPE)uType != type)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

error:
    return dwError;
}

static
DWORD
VmAfdMarshalUINT32 (
                    UINT32 uData,
                    PBYTE pMarshalledBlob,
                    DWORD dwBlobSize,
                    PDWORD pdwBytesUsed
                   )
{
    DWORD dwError = 0;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwSizeRemainining = dwBlobSize;
    DWORD dwBytesRead = 0;

    if (!pMarshalledBlob ||
        !pdwBytesUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *((PUINT32)pCursor) = VMW_IPC_TYPE_UINT32;
    pCursor += sizeof (UINT32);

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *((PUINT32)pCursor) = uData;
    pCursor += sizeof (UINT32);

    dwBytesRead = dwBlobSize - dwSizeRemainining;
    *pdwBytesUsed = dwBytesRead;

cleanup:
    return dwError;
error:
    if (pdwBytesUsed)
    {
        *pdwBytesUsed = 0;
    }

    goto cleanup;
}

static
DWORD
VmAfdUnMarshalUINT32 (
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PUINT32 puData,
                      PDWORD pdwBytesUsed
                     )
{
    DWORD dwError = 0;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwSizeRemainining = dwBlobSize;
    UINT32 uData = 0;
    UINT32 uType = 0;
    DWORD dwBytesRead = 0;

    if (
        !pMarshalledBlob ||
        !puData ||
        !pdwBytesUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                  sizeof (UINT32),
                                  &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    uType = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    dwError = VmAfdCheckType (
                              uType,
                              VMW_IPC_TYPE_UINT32
                             );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCheckMemory (
                                  sizeof (UINT32),
                                  &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    uData = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    dwBytesRead = dwBlobSize - dwSizeRemainining;
    *pdwBytesUsed = dwBytesRead;
    *puData = uData;
cleanup:
    return dwError;
error:
    if (pdwBytesUsed)
    {
        *pdwBytesUsed = 0;
    }
    if (puData)
    {
        *puData = 0;
    }

    goto cleanup;
}

static
DWORD
VmAfdMarshalStringW (
                      PWSTR pwszString,
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PDWORD pdwBytesUsed
                    )
{
    DWORD dwError = 0;
    size_t stringLength = 0;
    DWORD dwSizeRemainining = dwBlobSize;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwBytesRead = 0;

    if (
        !pMarshalledBlob ||
        !pdwBytesUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    *((PUINT32)pCursor) = VMW_IPC_TYPE_WSTRING;

    pCursor += sizeof (UINT32);

    dwError = VmAfdCheckMemory (
                                sizeof (VMAFD_IPC_SIZE_T),
                                &dwSizeRemainining
                               );

    BAIL_ON_VMAFD_ERROR (dwError);

    if (pwszString)
    {
        dwError = VmAfdGetStringLengthW (
                                          pwszString,
                                          &stringLength
                                        );
        BAIL_ON_VMAFD_ERROR (dwError);

        *((VMAFD_IPC_SIZE_T *)pCursor) = (VMAFD_IPC_SIZE_T)stringLength;
        pCursor += sizeof (VMAFD_IPC_SIZE_T);

        dwError = VmAfdCheckMemory (
                                    stringLength * sizeof (WCHAR),
                                    &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        memcpy (
                  pCursor,
                  pwszString,
                  stringLength * sizeof (WCHAR)
               );

        pCursor += stringLength * sizeof (WCHAR);
    }
    else
    {
        *((VMAFD_IPC_SIZE_T *)pCursor) = stringLength;
        pCursor += sizeof (VMAFD_IPC_SIZE_T);
    }

    dwBytesRead = dwBlobSize - dwSizeRemainining;

    *pdwBytesUsed = dwBytesRead;

cleanup:
    return dwError;

error:
    if (pdwBytesUsed)
    {
        *pdwBytesUsed = 0;
    }

    goto cleanup;
}

static
DWORD
VmAfdUnMarshalString (
                      PBYTE pMarshalledBlob,
                      DWORD dwBlobSize,
                      PWSTR *ppwszString,
                      PDWORD pdwBytesUsed
                    )
{
    DWORD dwError = 0;
    PWSTR pwszString = NULL;
    DWORD dwSizeRemainining = dwBlobSize;
    DWORD dwBytesRead = 0;
    UINT32 uType = 0;
    PBYTE  pCursor = pMarshalledBlob;
    size_t stringLength = 0;

    if (
        !pMarshalledBlob ||
        !ppwszString ||
        !pdwBytesUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMAFD_ERROR (dwError);
    }

    dwError = VmAfdCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemainining
                               );
    BAIL_ON_VMAFD_ERROR (dwError);

    uType = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    dwError = VmAfdCheckType (
                              uType,
                              VMW_IPC_TYPE_WSTRING
                             );
    BAIL_ON_VMAFD_ERROR (dwError);

    dwError = VmAfdCheckMemory (
                                sizeof (VMAFD_IPC_SIZE_T),
                                &dwSizeRemainining
                             );
    BAIL_ON_VMAFD_ERROR (dwError);

    stringLength = *((VMAFD_IPC_SIZE_T *)pCursor);
    pCursor += sizeof (VMAFD_IPC_SIZE_T);

    if (stringLength)
    {
        dwError = VmAfdCheckMemory (
                                      sizeof (WCHAR) * stringLength,
                                      &dwSizeRemainining
                                   );
        BAIL_ON_VMAFD_ERROR (dwError);

        dwError = VmAfdAllocateMemory (
                                        sizeof (WCHAR) * (stringLength +1),
                                        (PVOID *) & pwszString
                                      );
        BAIL_ON_VMAFD_ERROR (dwError);

        memcpy (
                pwszString,
                pCursor,
                sizeof (WCHAR) * stringLength
               );

        memset (&pwszString[stringLength], 0, sizeof (WCHAR));
    }

    *ppwszString = pwszString;

    dwBytesRead = dwBlobSize - dwSizeRemainining;

    *pdwBytesUsed = dwBytesRead;

cleanup:
    return dwError;

error:
    if (ppwszString)
    {
        *ppwszString = NULL;
    }
    VMAFD_SAFE_FREE_MEMORY (pwszString);

    goto cleanup;
}
