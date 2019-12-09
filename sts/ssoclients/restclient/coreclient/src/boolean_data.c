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
RestBooleanDataNew(
    bool** ppOut,
    bool in)
{
    SSOERROR e = SSOERROR_NONE;
    bool* pOut = NULL;

    if (ppOut == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(bool), (void**) &pOut);
    BAIL_ON_ERROR(e);

    *pOut = in;

    *ppOut = pOut;

    error:

    if (e != SSOERROR_NONE)
    {
        RestBooleanDataDelete(pOut);
    }

    return e;
}

void
RestBooleanDataDelete(
    bool* pIn)
{
    SSOMemoryFree(pIn, sizeof(bool));
}

SSOERROR
RestBooleanDataToJson(
    const bool* pBoolean,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pBoolean != NULL)
    {
        e = SSOJsonBooleanNew(&pJson, *pBoolean);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
RestJsonToBooleanData(
    PCSSO_JSON pJson,
    bool** ppBoolean)
{
    SSOERROR e = SSOERROR_NONE;
    bool* pBoolean = NULL;

    if (pJson == NULL || ppBoolean == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(bool), (void**) &pBoolean);
    BAIL_ON_ERROR(e);

    e = SSOJsonBooleanValue(pJson, pBoolean);
    BAIL_ON_ERROR(e);

    *ppBoolean = pBoolean;

    error:

    if (e != SSOERROR_NONE)
    {
        RestBooleanDataDelete(pBoolean);
    }

    return e;
}
