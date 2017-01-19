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

DWORD
VmAfdOpenServerConnection(
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	if (ppConnection == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	dwError = gIPCVtable.pfnOpenServerConnection(ppConnection);
	BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
	return dwError;
error:
	if (ppConnection != NULL){
		*ppConnection = NULL;
	}
	goto cleanup;

}

VOID
VmAfdCloseServerConnection(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection){
		gIPCVtable.pfnCloseServerConnection(pConnection);

	}
}

VOID
VmAfdFreeServerConnection(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection){
                VmAfdCloseServerConnection (pConnection);
		gIPCVtable.pfnFreeConnection(pConnection);
	}
}

DWORD
VmAfdOpenClientConnection(
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	if (ppConnection == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	dwError = gIPCVtable.pfnOpenClientConnection(ppConnection);
	BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
	return dwError;
error:
	if (ppConnection != NULL){
		*ppConnection = NULL;
	}
	goto cleanup;
}

VOID
VmAfdCloseClientConnection(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection){
		gIPCVtable.pfnCloseClientConnection(pConnection);

	}
}

VOID
VmAfdFreeClientConnection(
	PVM_AFD_CONNECTION pConnection
	)
{
	if (pConnection){
                VmAfdCloseClientConnection (pConnection);
		gIPCVtable.pfnFreeConnection(pConnection);
        }
}

DWORD
VmAfdAcceptConnection(
	PVM_AFD_CONNECTION pConnection,
	PVM_AFD_CONNECTION *ppConnection
	)
{
	DWORD dwError = 0;
	if (pConnection == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	dwError = gIPCVtable.pfnAcceptConnection(pConnection,ppConnection);
	BAIL_ON_VMAFD_ERROR(dwError);
cleanup:
	return dwError;
error:
	goto cleanup;
}

DWORD
VmAfdReadData(
	PVM_AFD_CONNECTION pConnection,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
    )
{
	DWORD dwError = 0;
	if (ppResponse == NULL || pConnection == NULL || pdwResponseSize == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR(dwError);
	}
	dwError = gIPCVtable.pfnReadData(pConnection,ppResponse,pdwResponseSize);
	BAIL_ON_VMAFD_ERROR_NO_LOG (dwError);
cleanup:
	return dwError;
error:
	if (ppResponse != NULL){
		*ppResponse = NULL;
	}
	if (pdwResponseSize != NULL){
		*pdwResponseSize = 0;
	}
	goto cleanup;
}

DWORD
VmAfdWriteData(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize
	)
{
	DWORD dwError = 0;
	if (pConnection == NULL || pRequest == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR(dwError);
	}
	if (dwRequestSize > 0){
		dwError = gIPCVtable.pfnWriteData (pConnection, pRequest, dwRequestSize);
	}
cleanup:
	return dwError;
error:
	goto cleanup;
}

DWORD
VmAfdMakeServerRequest(
	PVM_AFD_CONNECTION pConnection,
	PBYTE pRequest,
	DWORD dwRequestSize,
	PBYTE *ppResponse,
	PDWORD pdwResponseSize
	)
{
	DWORD dwError = 0;
	if (pConnection == NULL || pRequest == NULL){
		dwError = ERROR_INVALID_PARAMETER;
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	if (dwRequestSize > 0){
		dwError = VmAfdWriteData(pConnection, pRequest, dwRequestSize);
		BAIL_ON_VMAFD_ERROR (dwError);
	}
	dwError = VmAfdReadData (pConnection, ppResponse, pdwResponseSize);
	BAIL_ON_VMAFD_ERROR (dwError);
cleanup:
	return dwError;
error:
	if (ppResponse != NULL){
		*ppResponse = NULL;
	}
	if (pdwResponseSize != NULL){
		*pdwResponseSize = 0;
	}
	goto cleanup;
}

/*
 *  Quick and dirty function to verify format - colons and digits/a-f/A-F only
 */
BOOLEAN
VmAfdIsIPV6AddrFormat(
    PCSTR   pszAddr
    )
{
    BOOLEAN     bIsIPV6 = pszAddr ? TRUE : FALSE;
    size_t      iSize = 0;
    size_t      iCnt = 0;
    size_t      iColonCnt = 0;

    if ( pszAddr != NULL )
    {
        iSize = VmAfdStringLenA(pszAddr);
        for (iCnt=0; bIsIPV6 && iCnt < iSize; iCnt++)
        {
            if ( pszAddr[iCnt] == ':' )
            {
                iColonCnt++;
            }
            else if ( VMAFD_ASCII_DIGIT( pszAddr[iCnt] )
                      ||
                      VMAFD_ASCII_aTof( pszAddr[iCnt] )
                      ||
                      VMAFD_ASCII_AToF( pszAddr[iCnt] )
                    )
            {
            }
            else
            {
                bIsIPV6 = FALSE;
            }
        }

        // should not count on iColonCnt == 7
        if ( iColonCnt < 2 )
        {
            bIsIPV6 = FALSE;
        }
    }

    return bIsIPV6;
}
