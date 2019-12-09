/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

SSOERROR
RestAccessTokenNew(
    PREST_ACCESS_TOKEN* ppAccessToken,
    PCSTRING value,
    REST_ACCESS_TOKEN_TYPE type,
    PCSTRING privateKey)
{
    SSOERROR e = SSOERROR_NONE;
    REST_ACCESS_TOKEN* pAccessToken = NULL;

    if (ppAccessToken == NULL || IS_NULL_OR_EMPTY_STRING(value) /* || check type is enum */)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(REST_ACCESS_TOKEN), (void**) &pAccessToken);
    BAIL_ON_ERROR(e);

    e = RestStringDataNew(&(pAccessToken->value), value);
    BAIL_ON_ERROR(e);

    pAccessToken->type = type;

    if (privateKey != NULL)
    {
        e = RestStringDataNew(&(pAccessToken->privateKey), privateKey);
        BAIL_ON_ERROR(e);
    }

    *ppAccessToken = pAccessToken;

    error:

    if (e != SSOERROR_NONE)
    {
        RestAccessTokenDelete(pAccessToken);
    }

    return e;
}

void
RestAccessTokenDelete(
    PREST_ACCESS_TOKEN pAccessToken)
{
    if (pAccessToken != NULL)
    {
        RestStringDataDelete(pAccessToken->value);
        SSOStringFreeAndClear(pAccessToken->privateKey); // zero out privateKey for better security
        SSOMemoryFree(pAccessToken, sizeof(REST_ACCESS_TOKEN));
    }
}
