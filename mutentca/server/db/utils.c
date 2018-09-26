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

DWORD
LwCADbCopyEncryptedKey(
    PLWCA_DB_ENCRYPTED_KEY pEncryptedKey,
    PLWCA_DB_ENCRYPTED_KEY *ppEncryptedKey
    )
{
    DWORD dwError = 0;
    PLWCA_DB_ENCRYPTED_KEY pTempEncryptedKey = NULL;

    if (!pEncryptedKey || !ppEncryptedKey)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCADbCreateEncryptedKey(pEncryptedKey->pData, pEncryptedKey->dwLength, &pTempEncryptedKey);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppEncryptedKey = pTempEncryptedKey;

cleanup:
    return dwError;

error:
    LwCADbFreeEncryptedKey(pTempEncryptedKey);
    if (ppEncryptedKey)
    {
        *ppEncryptedKey = NULL;
    }

    goto cleanup;
}
