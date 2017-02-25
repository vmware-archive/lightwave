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
RestStringDataNew(
    PSTRING* ppOut,
    PCSTRING pIn)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pOut = NULL;

    if (ppOut == NULL || pIn == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOStringAllocate(pIn, &pOut);
    BAIL_ON_ERROR(e);

    *ppOut = pOut;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(pOut);
    }

    return e;
}

void
RestStringDataDelete(
    PSTRING pIn)
{
    SSOStringFree(pIn);
}

SSOERROR
RestStringDataToJson(
    PCSTRING pString,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JSON pJsonString = NULL;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pString != NULL)
    {
        e = SSOJsonStringNew(&pJsonString, pString);
        BAIL_ON_ERROR(e);

        e = SSOJsonReset(pJson, pJsonString);
        BAIL_ON_ERROR(e);
    }

    error:

    // cleanup
    SSOJsonDelete(pJsonString);

    return e;
}

SSOERROR
RestJsonToStringData(
    PCSSO_JSON pJson,
    PSTRING* ppString)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING pString = NULL;
    PSTRING value = NULL;

    if (pJson == NULL || ppString == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOJsonStringValue(pJson, &value);
    BAIL_ON_ERROR(e);

    e = RestStringDataNew(&pString, value);
    BAIL_ON_ERROR(e);

    *ppString = pString;

    error:

    if (e != SSOERROR_NONE)
    {
        RestStringDataDelete(pString);
    }

    SSOStringFree(value);

    return e;
}
