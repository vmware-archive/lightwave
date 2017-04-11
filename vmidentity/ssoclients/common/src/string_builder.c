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
SSOStringBuilderNew(
    PSSO_STRING_BUILDER* pp)
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER p = NULL;

    ASSERT_NOT_NULL(pp);

    e = SSOMemoryAllocate(sizeof(SSO_STRING_BUILDER), (void**) &p);
    BAIL_ON_ERROR(e);

    e = SSOMemoryAllocate(sizeof(char), (void**) &p->pszBuffer);
    BAIL_ON_ERROR(e);

    p->pszBuffer[0] = '\0';
    p->bufferLength = 1;

    *pp = p;

error:
    if (e != SSOERROR_NONE)
    {
        SSOStringBuilderDelete(p);
    }
    return e;
}

void
SSOStringBuilderDelete(
    PSSO_STRING_BUILDER p)
{
    if (p != NULL)
    {
        SSOMemoryFree(p->pszBuffer, p->bufferLength);
        SSOMemoryFree(p, sizeof(SSO_STRING_BUILDER));
    }
}

SSOERROR
SSOStringBuilderAppend(
    PSSO_STRING_BUILDER p,
    PCSTRING pszString)
{
    SSOERROR e = SSOERROR_NONE;
    size_t length = 0;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(pszString);

    length = strlen(pszString);

    e = SSOMemoryReallocate(p->bufferLength + length, (void**) &p->pszBuffer);
    BAIL_ON_ERROR(e);

    strcpy(p->pszBuffer + p->bufferLength - 1, pszString);
    p->bufferLength += length;

error:
    return e;
}

SSOERROR
SSOStringBuilderGetString(
    PCSSO_STRING_BUILDER p,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;

    ASSERT_NOT_NULL(p);
    ASSERT_NOT_NULL(ppsz);

    e = SSOStringAllocate(p->pszBuffer, ppsz);
    BAIL_ON_ERROR(e);

error:
    return e;
}
