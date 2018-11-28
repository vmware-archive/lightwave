/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

DWORD
LwCARestMakeGetCAJsonResponse(
    PLWCA_CERTIFICATE_ARRAY     pCACerts,
    PLWCA_STRING_ARRAY          pCRLs,
    BOOLEAN                     bDetail,
    PLWCA_JSON_OBJECT           *ppJsonRespArray
    )
{
    DWORD                       dwError             = 0;
    DWORD                       dwIdx               = 0;
    PLWCA_JSON_OBJECT           pJsonEntry          = NULL;
    PLWCA_JSON_OBJECT           pJsonRespArray      = NULL;

    if (!pCACerts || !ppJsonRespArray)
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    if (bDetail && (!pCRLs || pCRLs->dwCount != pCACerts->dwCount))
    {
        BAIL_WITH_LWCA_ERROR(dwError, LWCA_ERROR_INVALID_PARAMETER);
    }

    dwError = LwCAJsonArrayCreate(&pJsonRespArray);
    BAIL_ON_LWCA_ERROR(dwError);

    for (; dwIdx < pCACerts->dwCount; ++dwIdx)
    {
        dwError = LwCAJsonObjectCreate(&pJsonEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        dwError = LwCAJsonSetStringToObject(
                            pJsonEntry,
                            LWCA_JSON_KEY_CERT,
                            (PSTR)pCACerts->ppCertificates[dwIdx]);
        BAIL_ON_LWCA_ERROR(dwError);

        if (bDetail)
        {
            dwError = LwCAJsonSetStringToObject(pJsonEntry, LWCA_JSON_KEY_CRL, pCRLs->ppData[dwIdx]);
            BAIL_ON_LWCA_ERROR(dwError);
        }

        dwError = LwCAJsonAppendJsonToArray(pJsonRespArray, pJsonEntry);
        BAIL_ON_LWCA_ERROR(dwError);

        LWCA_SAFE_JSON_DECREF(pJsonEntry);
    }

    *ppJsonRespArray = pJsonRespArray;


cleanup:

    LWCA_SAFE_JSON_DECREF(pJsonEntry);

    return dwError;

error:

    LWCA_SAFE_JSON_DECREF(pJsonRespArray);
    if (ppJsonRespArray)
    {
        *ppJsonRespArray = NULL;
    }

    goto cleanup;
}
