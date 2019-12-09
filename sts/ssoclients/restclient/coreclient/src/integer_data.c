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
RestIntegerDataNew(
    INTEGER** ppOut,
    INTEGER in)
{
    SSOERROR e = SSOERROR_NONE;
    INTEGER* pOut = NULL;

    if (ppOut == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(INTEGER), (void**) &pOut);
    BAIL_ON_ERROR(e);

    *pOut = in;

    *ppOut = pOut;

    error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pOut, sizeof(INTEGER));
    }

    return e;
}

void
RestIntegerDataDelete(
    INTEGER* pIn)
{
    SSOMemoryFree(pIn, sizeof(INTEGER));
}
