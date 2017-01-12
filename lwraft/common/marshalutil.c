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
VmDirCheckMemory(
        size_t size,
        PDWORD pdwResponseSize
        );

DWORD
VmDirGetMarshalLength(
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
		BAIL_ON_VMDIR_ERROR (dwError);
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
				dwResponseSize += sizeof (size_t);
				if (tempString != NULL){
                                   dwResponseSize += strlen(tempString);
				}
				break;
			case VMW_IPC_TYPE_WSTRING:
				dwResponseSize += sizeof(UINT32);
				tempWString = pInput[iArgCounter].data.pWString;
				dwResponseSize += sizeof (size_t);
				if (tempWString != NULL){
                                        dwError = VmDirGetStringLengthW (tempWString, &stringLength);
                                        BAIL_ON_VMDIR_ERROR (dwError);
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
				BAIL_ON_VMDIR_ERROR (dwError);
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
VmDirMarshal(
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
		BAIL_ON_VMDIR_ERROR (dwError);
	}

	dwError = VmDirAllocateMemory (dwResponseSize, &memGet);
	BAIL_ON_VMDIR_ERROR (dwError);

	pCursor = (PBYTE)memGet;

	pCursorStart = pCursor;
        dwActualResponseSz = dwResponseSize;

        dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
        BAIL_ON_VMDIR_ERROR (dwError);
	*((UINT32 *)pCursor) = apiType;
	pCursor += sizeof(UINT32);
        dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
        BAIL_ON_VMDIR_ERROR (dwError);
	*((UINT32 *)pCursor) = apiVersion;
	pCursor += sizeof(UINT32);
        dwError = VmDirCheckMemory (sizeof (DWORD), &dwActualResponseSz);
        BAIL_ON_VMDIR_ERROR (dwError);
	*((DWORD *)pCursor) = noOfArgs;
	pCursor += sizeof(DWORD);
	while (iArgCounter < noOfArgs){
		size_t stringLength = 0;
		UINT32 type = pInput[iArgCounter].type;
		PSTR tempString = NULL;
		WCHAR * tempWString = NULL;
		switch (type){
			case VMW_IPC_TYPE_UINT32:
                                dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_UINT32;
				pCursor += sizeof (UINT32);
                                dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
                                if (pInput[iArgCounter].data.pUint32)
                                {
                                        *((UINT32 *)pCursor) = *(pInput[iArgCounter].data.pUint32);
                                }
				pCursor += sizeof (UINT32);
				break;
			case VMW_IPC_TYPE_STRING:
                                dwError = VmDirCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_STRING;
				pCursor += sizeof (UINT32);
				tempString = pInput[iArgCounter].data.pString;
				if (tempString == NULL){
                                        dwError = VmDirCheckMemory (sizeof (size_t), &dwActualResponseSz);
                                        BAIL_ON_VMDIR_ERROR (dwError);
					*((size_t *)pCursor) = 0;
					pCursor += sizeof (size_t);
					break;
				}
                                dwError = VmDirCheckMemory (sizeof (size_t), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				stringLength = strlen(tempString);
				*((size_t *)pCursor) = stringLength;
				pCursor += sizeof (size_t);
                                dwError = VmDirCheckMemory (stringLength, &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				strncpy (pCursor, tempString, stringLength);
				pCursor += stringLength;
				break;
			case VMW_IPC_TYPE_WSTRING:
                                dwError = VmDirCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_WSTRING;
				pCursor += sizeof(UINT32);
				tempWString = pInput[iArgCounter].data.pWString;
				if (tempWString == NULL){
                                        dwError = VmDirCheckMemory (sizeof(size_t), &dwActualResponseSz);
                                        BAIL_ON_VMDIR_ERROR (dwError);
					*((size_t *)pCursor) = 0;
					pCursor += sizeof (size_t);
					break;
				}
				dwError = VmDirGetStringLengthW(tempWString, &stringLength);
				BAIL_ON_VMDIR_ERROR (dwError);
                                dwError = VmDirCheckMemory (sizeof(size_t), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((size_t *)pCursor) = stringLength;
				pCursor += sizeof (size_t);
                                dwError = VmDirCheckMemory (sizeof(WCHAR)*stringLength, &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
                                memcpy (pCursor, tempWString, stringLength*sizeof(WCHAR));
				pCursor += stringLength*sizeof(WCHAR);
				break;
                        case VMW_IPC_TYPE_BLOB:
                                dwError =  VmDirCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);

                                *((UINT32 *)pCursor) = VMW_IPC_TYPE_BLOB;
                                pCursor += sizeof (UINT32);

                                if (!pInput[iArgCounter].data.pUint32)
                                {
                                        dwBlobSize = sizeof (UINT32);
                                        dwError = VmDirCheckMemory(dwBlobSize, &dwActualResponseSz);
                                        BAIL_ON_VMDIR_ERROR (dwError);
                                        *((PUINT32)pCursor) = sizeof (UINT32);
                                        pCursor += sizeof (UINT32);
                                        break;
                                }

                                dwBlobSize = *(pInput[iArgCounter].data.pUint32);

                                dwError = VmDirCheckMemory (dwBlobSize, &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);

                                pCursor = memcpy (
                                                (PVOID) pCursor,
                                                (PVOID) pInput[iArgCounter].data.pByte,
                                                dwBlobSize
                                                );

				pCursor += dwBlobSize;
				break;
                        case VMW_IPC_TYPE_BLOB_SIZE:
                                dwError = VmDirCheckMemory (sizeof (UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);

                                *((UINT32 *)pCursor) = VMW_IPC_TYPE_BLOB_SIZE;
                                pCursor += sizeof(UINT32);
                                break;
                       case VMW_IPC_TYPE_BOOL:
                                dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_BOOL;
				pCursor += sizeof (UINT32);
                                dwError = VmDirCheckMemory (sizeof(BOOL), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
                                if(pInput[iArgCounter].data.pBool)
                                {
                                        *((BOOL *)pCursor) = *(pInput[iArgCounter].data.pBool);
                                }
				pCursor += sizeof (BOOL);
				break;
                      case VMW_IPC_TYPE_BOOLEAN:
                                dwError = VmDirCheckMemory (sizeof(UINT32), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
				*((UINT32 *)pCursor) = VMW_IPC_TYPE_BOOLEAN;
				pCursor += sizeof (UINT32);
                                dwError = VmDirCheckMemory (sizeof(BOOLEAN), &dwActualResponseSz);
                                BAIL_ON_VMDIR_ERROR (dwError);
                                if (pInput[iArgCounter].data.pBoolean)
                                {
                                        *((BOOLEAN *)pCursor) = *(pInput[iArgCounter].data.pBoolean);
                                }
				pCursor += sizeof (BOOLEAN);
				break;

			default:
				dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMDIR_ERROR (dwError);
		}
		iArgCounter++;
	}
        pCursor = pCursorStart;

        memcpy (pResponse, pCursorStart, dwResponseSize);


cleanup:
        VMDIR_SAFE_FREE_MEMORY (memGet);
	return dwError;
error:
	goto cleanup;

}


DWORD
VmDirUnMarshal(
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
		BAIL_ON_VMDIR_ERROR (dwError);
	}
	if (pResponse == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMDIR_ERROR (dwError);
	}

	apiTypeInput = *((UINT32 *) pResponse);
	pResponse += sizeof(UINT32);
	apiVersionInput = *((UINT32 *)pResponse);
	pResponse += sizeof(UINT32);
	noOfArgsInput = *((DWORD *) pResponse);
	pResponse += sizeof (DWORD);

	if ((apiTypeInput != apiType) || (apiVersionInput != apiVersion) || (noOfArgs != noOfArgsInput)){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMDIR_ERROR (dwError);
	}
	dwBytesRead = 2* sizeof (UINT32) + sizeof (DWORD);

	while (iArgCounter < noOfArgs){

		UINT32 typeInput = *((UINT32 *)pResponse);
		size_t stringLength;
		PSTR pTempString = NULL;
                PBYTE pTempByte = NULL;
                PWSTR pTempWString = NULL;
		pResponse += sizeof(UINT32);
		dwBytesRead += sizeof (UINT32);
		if (typeInput != pInput[iArgCounter].type){
			dwError = ERROR_INVALID_PARAMETER;
			BAIL_ON_VMDIR_ERROR (dwError);
		}
		switch (typeInput){
			case VMW_IPC_TYPE_UINT32:
				if (pInput[iArgCounter].data.pUint32 == NULL){
					dwError = VmDirAllocateMemory(sizeof(UINT32),(PVOID *)&(pInput[iArgCounter].data.pUint32));
					BAIL_ON_VMDIR_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pUint32) = *((PUINT32)pResponse);
				pResponse += sizeof(UINT32);
				dwBytesRead += sizeof (UINT32);
				break;
			case VMW_IPC_TYPE_STRING:
				stringLength = *((size_t *)pResponse);
				pResponse += sizeof (size_t);
				dwBytesRead += sizeof (size_t);
				if (stringLength == 0){
					pInput[iArgCounter].data.pString = NULL;
					break;
				}
				dwError = VmDirAllocateMemory (stringLength+1,(PVOID *)&pTempString);
				BAIL_ON_VMDIR_ERROR (dwError);

				strncpy(pTempString,pResponse,stringLength);
				pTempString[stringLength] = '\0';
				pResponse += stringLength;
				dwBytesRead += stringLength;
				pInput[iArgCounter].data.pString = pTempString;
				break;
			case VMW_IPC_TYPE_WSTRING:
				stringLength = *((size_t *)pResponse);
				pResponse += sizeof (size_t);
				dwBytesRead += sizeof (size_t);
				if (stringLength == 0){
					pInput[iArgCounter].data.pWString = NULL;
					break;
				}
				dwError = VmDirAllocateMemory ((stringLength+1)*sizeof(WCHAR),(PVOID *)&pTempWString);
				BAIL_ON_VMDIR_ERROR (dwError);
                                memcpy (pTempWString, pResponse, stringLength*sizeof(WCHAR));
				memset ((PVOID)&pTempWString[stringLength], 0, sizeof(WCHAR));
				pResponse += stringLength * sizeof (WCHAR);
				dwBytesRead += stringLength * sizeof(WCHAR);
                                pInput[iArgCounter].data.pWString = pTempWString;
				break;

                        case VMW_IPC_TYPE_BLOB:
                                dwBlobSize = *(PUINT32)pResponse;
                                dwError = VmDirAllocateMemory (
                                                dwBlobSize,
                                                (PVOID *) &pTempByte
                                                );
                                BAIL_ON_VMDIR_ERROR (dwError);

                                memcpy (pTempByte, pResponse, dwBlobSize);

                                pResponse += dwBlobSize;
                                dwBytesRead += dwBlobSize;
                                pInput[iArgCounter].data.pByte = pTempByte;
                                break;
                        case VMW_IPC_TYPE_BLOB_SIZE:
                                if (pInput[iArgCounter].data.pUint32 == NULL)
                                {
                                        dwError = VmDirAllocateMemory (
                                                                sizeof (UINT32),
                                                                (PVOID *) &pInput[iArgCounter].data.pUint32
                                                                );
                                        BAIL_ON_VMDIR_ERROR (dwError);
                                }
                                *(pInput[iArgCounter].data.pUint32) = dwBlobSize;
                                dwBlobSize = 0;
                                break;
                        case VMW_IPC_TYPE_BOOL:
				if (pInput[iArgCounter].data.pBool == NULL){
					dwError = VmDirAllocateMemory(sizeof(BOOL),(PVOID *)&(pInput[iArgCounter].data.pBool));
					BAIL_ON_VMDIR_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pBool) = *((PBOOL)pResponse);
				pResponse += sizeof(BOOL);
				dwBytesRead += sizeof (BOOL);
				break;
                       case VMW_IPC_TYPE_BOOLEAN:
				if (pInput[iArgCounter].data.pBoolean == NULL){
					dwError = VmDirAllocateMemory(sizeof(BOOLEAN),(PVOID *)&(pInput[iArgCounter].data.pBoolean));
					BAIL_ON_VMDIR_ERROR (dwError);
				}
				*(pInput[iArgCounter].data.pBoolean) = *((PBOOLEAN)pResponse);
				pResponse += sizeof(BOOLEAN);
				dwBytesRead += sizeof (BOOLEAN);
				break;

			default:
                                dwError = ERROR_INVALID_PARAMETER;
                                BAIL_ON_VMDIR_ERROR (dwError);
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
VmDirMarshalStringArrayGetSize (
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
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (dwArraySize && !pwszStringArray)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwSizeRequired = 2 * sizeof (UINT32); //Size of the blob + No of Elements

        for (;dwIndex < dwArraySize; dwIndex++)
        {
                size_t dwStringLength = 0;
                dwSizeRequired += sizeof (size_t); //Length of String

                if (pwszStringArray[dwIndex])
                {
                        dwError = VmDirGetStringLengthW (
                                                         pwszStringArray[dwIndex],
                                                         &dwStringLength
                                                        );
                        BAIL_ON_VMDIR_ERROR (dwError);

                        dwSizeRequired += dwStringLength*sizeof(WCHAR);
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
VmDirMarshalStringArray (
                         PWSTR *pwszStringArray,
                         DWORD dwArraySize,
                         DWORD dwBlobSize,
                         PBYTE pMarshalledBlob
                        )
{
        DWORD dwError = 0;
        DWORD dwSizeRemaining = dwBlobSize;
        PVOID pTempMarshal = NULL;
        PBYTE pCursor = NULL;
        DWORD dwIndex = 0;

        if (!pMarshalledBlob)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (dwArraySize && !pwszStringArray)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwError = VmDirAllocateMemory (
                                        dwBlobSize,
                                        &pTempMarshal
                                      );
        BAIL_ON_VMDIR_ERROR (dwError);

        pCursor = (PBYTE)pTempMarshal;

        dwError = VmDirCheckMemory (
                                    sizeof (UINT32),
                                    &dwSizeRemaining
                                   );
        BAIL_ON_VMDIR_ERROR (dwError);

        *((PUINT32)pCursor) = dwBlobSize;

        pCursor += sizeof (UINT32);

        dwError = VmDirCheckMemory (
                                    sizeof (UINT32),
                                    &dwSizeRemaining
                                   );
        BAIL_ON_VMDIR_ERROR (dwError);

        *((PUINT32)pCursor) = dwArraySize;
        pCursor += sizeof (UINT32);

        for (;dwIndex<dwArraySize; dwIndex++)
        {
                size_t dwStringLength = 0;

                if (pwszStringArray[dwIndex])
                {

                        dwError = VmDirGetStringLengthW(
                                                        pwszStringArray[dwIndex],
                                                        &dwStringLength
                                                       );
                        BAIL_ON_VMDIR_ERROR (dwError);

                        dwError = VmDirCheckMemory (
                                                     sizeof (size_t),
                                                     &dwSizeRemaining
                                                   );
                        BAIL_ON_VMDIR_ERROR (dwError);

                        *((size_t *)pCursor) = dwStringLength;
                        pCursor += sizeof (size_t);

                        dwError = VmDirCheckMemory (sizeof(WCHAR)*dwStringLength, &dwSizeRemaining);
                        BAIL_ON_VMDIR_ERROR (dwError);

                        memcpy (pCursor, pwszStringArray[dwIndex], dwStringLength*sizeof(WCHAR));
                        pCursor += dwStringLength*sizeof(WCHAR);
                }
                else
                {
                        dwError = VmDirCheckMemory (
                                                        sizeof (size_t),
                                                        &dwSizeRemaining
                                                   );
                        BAIL_ON_VMDIR_ERROR (dwError);

                        *((size_t *)pCursor) = dwStringLength;
                        pCursor += sizeof(size_t);
                }
        }

        memcpy (pMarshalledBlob, pTempMarshal, dwBlobSize);

cleanup:
        VMDIR_SAFE_FREE_MEMORY (pTempMarshal);

        return dwError;

error:

        goto cleanup;
}

DWORD
VmDirUnMarshalStringArray (
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
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        if (dwBlobSize < (sizeof(UINT32) + sizeof(UINT32)))
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwError = VmDirCheckMemory (
                                    sizeof (UINT32),
                                    &dwBytesAvailable
                                   );
        BAIL_ON_VMDIR_ERROR (dwError);


        dwBlobSizeRead = *((PUINT32)pCursor);
        pCursor += sizeof (UINT32);

        if (dwBlobSizeRead != dwBlobSize)
        {
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        dwError = VmDirCheckMemory (
                                    sizeof (UINT32),
                                    &dwBytesAvailable
                                  );
        BAIL_ON_VMDIR_ERROR (dwError);

        dwArraySize = *((PUINT32)pCursor);
        pCursor += sizeof (UINT32);

        if (dwArraySize)
        {
                dwError = VmDirAllocateMemory (
                                                dwArraySize * sizeof (PWSTR),
                                                (PVOID *) &pwszStringArray
                                              );
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        for (; dwIndex < dwArraySize; dwIndex++)
        {
                size_t stringLength = 0;

                dwError = VmDirCheckMemory (
                                             sizeof (size_t),
                                             &dwBytesAvailable
                                           );
                BAIL_ON_VMDIR_ERROR (dwError);

                stringLength = *((size_t *)pCursor);
                pCursor += sizeof (size_t);

                if (stringLength)
                {
                        dwError = VmDirCheckMemory (
                                                    sizeof(WCHAR) * stringLength,
                                                    &dwBytesAvailable
                                                   );
                        BAIL_ON_VMDIR_ERROR (dwError);

                        dwError = VmDirAllocateMemory (
                                                       (stringLength+1)*sizeof (WCHAR),
                                                       (PVOID *)&pTempString
                                                      );
                        BAIL_ON_VMDIR_ERROR (dwError);

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
                VmDirFreeStringArrayW(
                                      pwszStringArray,
                                      dwArraySize
                                     );
        }

        goto cleanup;
}

DWORD
VmDirMarshalUINT32 (
                    UINT32 uData,
                    PBYTE pMarshalledBlob,
                    DWORD dwBlobSize,
                    PDWORD pdwBytesUsed
                   )
{
    DWORD dwError = 0;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwSizeRemaining = dwBlobSize;
    DWORD dwBytesRead = 0;

    if (!pMarshalledBlob ||
        !pdwBytesUsed
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemaining
                               );
    BAIL_ON_VMDIR_ERROR (dwError);

    *((PUINT32)pCursor) = VMW_IPC_TYPE_UINT32;
    pCursor += sizeof (UINT32);

    dwError = VmDirCheckMemory (
                                sizeof (UINT32),
                                &dwSizeRemaining
                               );
    BAIL_ON_VMDIR_ERROR (dwError);

    *((PUINT32)pCursor) = uData;
    pCursor += sizeof (UINT32);

    dwBytesRead = dwBlobSize - dwSizeRemaining;
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

DWORD
VmDirMarshalContainerLength(
    VMDIR_IPC_DATA_CONTAINER *pContainer,
    PDWORD pdwSizeRequired
)
{
    DWORD dwError = 0;
    DWORD dwSizeRequired = 0;

    if (!pContainer || !pdwSizeRequired)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwSizeRequired += sizeof (UINT32);
    dwSizeRequired += sizeof (UINT32);
    dwSizeRequired += pContainer->dwCount;

    *pdwSizeRequired = dwSizeRequired;

error:

    return dwError;
}

DWORD
VmDirMarshalContainer(
    VMDIR_IPC_DATA_CONTAINER* pContainer,
    DWORD dwBlobSize,
    PBYTE pMarshalledBlob)
{
    DWORD dwError = 0;
    DWORD dwSizeRemaining = dwBlobSize;
    PBYTE pCursor = NULL;
    PBYTE pCursorStart = NULL;

    if (!pMarshalledBlob)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (dwBlobSize < (sizeof (UINT32) + sizeof (UINT32)))
    {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                    dwBlobSize,
                                    (PVOID *)&pCursor
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    pCursorStart = pCursor;

    *((PUINT32)pCursor) = dwBlobSize;
    pCursor += sizeof (UINT32);
    dwSizeRemaining -= sizeof (UINT32);

    *((PUINT32)pCursor) = pContainer->dwCount;
    pCursor += sizeof (UINT32);
    dwSizeRemaining -= sizeof (UINT32);

    if (pContainer->dwCount && pContainer->data)
    {
        dwError = VmDirCopyMemory(
                                   pCursor,
                                   dwSizeRemaining,
                                   pContainer->data,
                                   pContainer->dwCount
                                 );
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirCopyMemory (
                               pMarshalledBlob,
                               dwBlobSize,
                               pCursorStart,
                               dwBlobSize
                              );
    BAIL_ON_VMDIR_ERROR (dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY (pCursorStart);

    return dwError;

error:
    goto cleanup;
}

DWORD
VmDirUnMarshalContainer(
    DWORD dwBlobSize,
    PBYTE pMarshalledBlob,
    PVMDIR_IPC_DATA_CONTAINER *ppContainer)
{
    DWORD dwError = 0;
    VMDIR_IPC_DATA_CONTAINER *pContainer = NULL;
    PBYTE pCursor = pMarshalledBlob;
    DWORD dwBlobSizeRead = 0;

    if (
        !pMarshalledBlob ||
        !ppContainer
       )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    if (dwBlobSize < sizeof (UINT32) + sizeof (UINT32))
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    dwError = VmDirAllocateMemory (
                                    sizeof (VMDIR_IPC_DATA_CONTAINER),
                                    (PVOID *)&pContainer
                                  );
    BAIL_ON_VMDIR_ERROR (dwError);

    dwBlobSizeRead = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    if (dwBlobSizeRead != dwBlobSize)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR (dwError);
    }

    pContainer->dwCount = *((PUINT32)pCursor);
    pCursor += sizeof (UINT32);

    dwError = VmDirAllocateAndCopyMemory(
                             pCursor,
                             pContainer->dwCount,
                             (PVOID*)&pContainer->data);
    BAIL_ON_VMDIR_ERROR (dwError);

    *ppContainer = pContainer;

cleanup:

    return dwError;

error:
    if (ppContainer)
    {
        *ppContainer = NULL;
    }

    goto cleanup;
}

static
DWORD
VmDirCheckMemory(
    size_t size,
    PDWORD pdwResponseSize
    )
{
        DWORD dwError = 0;
        int sResponseSize = 0;
        if (pdwResponseSize == NULL){
                dwError = ERROR_INVALID_PARAMETER;
                BAIL_ON_VMDIR_ERROR (dwError);
        }
        sResponseSize = *pdwResponseSize - size;
        if (sResponseSize < 0){
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                BAIL_ON_VMDIR_ERROR (dwError);
        }

        *pdwResponseSize = (DWORD)sResponseSize;
cleanup:
        return dwError;
error:
        goto cleanup;
}
