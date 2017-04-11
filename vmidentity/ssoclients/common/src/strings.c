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
SSOStringAllocate(
    PCSTRING psz,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    size_t length = 0;
    char* p = NULL;

    ASSERT_NOT_NULL(psz);
    ASSERT_NOT_NULL(ppsz);

    length = strlen(psz);
    p = calloc(length + 1, sizeof(char));
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    strcpy(p, psz);
    *ppsz = p;

error:
    return e;
}

SSOERROR
SSOStringAllocateFromInt(
    int i,
    PSTRING* ppsz /* OUT */)
{
    char pszBuffer[20]; // 20 is more than enough to fit the largest unsigned 64 bit integer

    ASSERT_NOT_NULL(ppsz);

    sprintf(pszBuffer, "%d", i);

    return SSOStringAllocate(pszBuffer, ppsz);
}

SSOERROR
SSOStringAllocateSubstring(
    PCSTRING psz,
    size_t startIndex,
    size_t endIndex,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    size_t inputLength = 0;
    size_t outputLength = 0;
    char* p = NULL;

    ASSERT_NOT_NULL(psz);
    ASSERT_NOT_NULL(ppsz);

    inputLength = SSOStringLength(psz);

    if (startIndex > endIndex || startIndex >= inputLength || endIndex >= inputLength)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    outputLength = endIndex - startIndex + 1;
    p = calloc(outputLength + 1, sizeof(char));
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    memcpy(p, psz + startIndex, outputLength);

    *ppsz = p;

error:
    return e;
}

SSOERROR
SSOStringConcatenate(
    PCSTRING psz1,
    PCSTRING psz2,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    size_t length1 = 0;
    size_t length2 = 0;
    char* p = NULL;

    ASSERT_NOT_NULL(psz1);
    ASSERT_NOT_NULL(psz2);
    ASSERT_NOT_NULL(ppsz);

    length1 = strlen(psz1);
    length2 = strlen(psz2);
    p = calloc(length1 + length2 + 1, sizeof(char));
    if (NULL == p)
    {
        e = SSOERROR_OUT_OF_MEMORY;
        BAIL_ON_ERROR(e);
    }

    strcpy(p, psz1);
    strcpy(p + length1, psz2);
    *ppsz = p;

error:
    return e;
}

SSOERROR
SSOStringReplace(
    PCSTRING pszInput,
    PCSTRING pszFind,
    PCSTRING pszReplace,
    PSTRING* ppsz /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING psz = NULL;
    PSTRING pszPrefix = NULL;
    PSTRING pszSuffix = NULL;
    PSTRING pszPrefixPlusReplace = NULL;
    char* pszLocation = strstr(pszInput, pszFind);

    ASSERT_NOT_NULL(pszInput);
    ASSERT_NOT_NULL(pszFind);
    ASSERT_NOT_NULL(pszReplace);
    ASSERT_NOT_NULL(ppsz);

    if (pszLocation != NULL)
    {
        size_t inputLength = strlen(pszInput);
        size_t findLength = strlen(pszFind);
        size_t locationLength = strlen(pszLocation);
        size_t prefixLength = inputLength - locationLength;
        size_t suffixLength = locationLength - findLength;

        if (prefixLength > 0)
        {
            e = SSOStringAllocateSubstring(pszInput, 0, prefixLength - 1, &pszPrefix);
            BAIL_ON_ERROR(e);
        }
        else
        {
            e = SSOStringAllocate("", &pszPrefix);
            BAIL_ON_ERROR(e);
        }

        if (suffixLength > 0)
        {
            e = SSOStringAllocateSubstring(pszInput, inputLength - suffixLength, inputLength - 1, &pszSuffix);
            BAIL_ON_ERROR(e);
        }
        else
        {
            e = SSOStringAllocate("", &pszSuffix);
            BAIL_ON_ERROR(e);
        }

        e = SSOStringConcatenate(pszPrefix, pszReplace, &pszPrefixPlusReplace);
        BAIL_ON_ERROR(e);

        e = SSOStringConcatenate(pszPrefixPlusReplace, pszSuffix, &psz);
        BAIL_ON_ERROR(e);
    }
    else
    {
        // not found, return a copy of the input
        e = SSOStringAllocate(pszInput, &psz);
        BAIL_ON_ERROR(e);
    }

    *ppsz = psz;

error:

    if (e != SSOERROR_NONE)
    {
        SSOStringFree(psz);
    }

    SSOStringFree(pszPrefix);
    SSOStringFree(pszSuffix);
    SSOStringFree(pszPrefixPlusReplace);

    return e;
}

size_t
SSOStringLength(
    PCSTRING psz)
{
    ASSERT_NOT_NULL(psz);
    return strlen(psz);
}

bool
SSOStringEqual(
    PCSTRING psz1, /* OPT */
    PCSTRING psz2  /* OPT */)
{
    bool result;
    if (NULL == psz1 && NULL == psz2)
    {
        result = true;
    }
    else if (NULL != psz1 && NULL != psz2)
    {
        result = (strcmp(psz1, psz2) == 0);
    }
    else
    {
        result = false;
    }
    return result;
}

void
SSOStringFree(
    PSTRING psz /* OPT */)
{
    if (psz != NULL)
    {
        psz[0] = 0; // we don't want to compute strlen but we can at least turn it into an empty string
        free(psz);
    }
}

void
SSOStringFreeAndClear(
    PSTRING psz /* OPT */)
{
    if (psz != NULL)
    {
        memset(psz, 0, strlen(psz));
        free(psz);
    }
}
