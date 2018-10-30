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

static
DWORD
_LwCARegexMapError(
    int status
    );

DWORD
LwCARegexInit(
    PCSTR   pcszPattern,
    PREGEX  *ppRegex
    )
{
    DWORD dwError = 0;
    int status = 0;
    PREGEX pRegex = NULL;

    if (IsNullOrEmptyString(pcszPattern) || !ppRegex)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    dwError = LwCAAllocateMemory(sizeof(REGEX), (PVOID) &pRegex);
    BAIL_ON_LWCA_ERROR(dwError);

    // REG_EXTENDED: Use Extended Regular Expressions
    // REG_NOSUB: Report only success or fail in regexec()
    status = regcomp(pRegex, pcszPattern, REG_EXTENDED|REG_NOSUB);
    dwError = _LwCARegexMapError(status);
    BAIL_ON_LWCA_ERROR(dwError);

    *ppRegex = pRegex;

cleanup:
    return dwError;

error:
    LWCA_SAFE_FREE_MEMORY(pRegex);
    goto cleanup;
}

DWORD
LwCARegexValidate(
    PCSTR       pcszValue,
    PREGEX      pRegex,
    PBOOLEAN    pbIsValid
    )
{
    DWORD dwError = 0;
    int status = 0;
    BOOLEAN bIsValid = FALSE;

    if (IsNullOrEmptyString(pcszValue) || !pbIsValid)
    {
        dwError = LWCA_ERROR_INVALID_PARAMETER;
        BAIL_ON_LWCA_ERROR(dwError);
    }

    status = regexec(pRegex, pcszValue, (size_t) 0, NULL, 0);
    dwError = _LwCARegexMapError(status);

    if (dwError == 0)
    {
        bIsValid = TRUE;
    }
    else if (dwError == LWCA_REGEX_ERROR_NOMATCH)
    {
        dwError = 0;
        bIsValid = FALSE;
    }
    BAIL_ON_LWCA_ERROR(dwError);

    *pbIsValid = bIsValid;

cleanup:
    return dwError;

error:
    *pbIsValid = FALSE;
    goto cleanup;
}

VOID
LwCARegexFree(
    PREGEX  pRegex
    )
{
    if (pRegex)
    {
        regfree(pRegex);
        LWCA_SAFE_FREE_MEMORY(pRegex);
    }
}

static
DWORD
_LwCARegexMapError(
    int status
    )
{
    DWORD dwError = 0;

    switch (status)
    {
    case 0:
        dwError = 0;
        break;

    case REG_NOMATCH:
        dwError = LWCA_REGEX_ERROR_NOMATCH;
        break;

    case REG_BADPAT:
        dwError = LWCA_REGEX_ERROR_BADPAT;
        break;

    case REG_ECOLLATE:
        dwError = LWCA_REGEX_ERROR_ECOLLATE;
        break;

    case REG_ECTYPE:
        dwError = LWCA_REGEX_ERROR_ECTYPE;
        break;

    case REG_EESCAPE:
        dwError = LWCA_REGEX_ERROR_EESCAPE;
        break;

    case REG_ESUBREG:
        dwError = LWCA_REGEX_ERROR_ESUBREG;
        break;

    case REG_EBRACK:
        dwError = LWCA_REGEX_ERROR_EBRACK;
        break;

    case REG_EPAREN:
        dwError = LWCA_REGEX_ERROR_EPAREN;
        break;

    case REG_EBRACE:
        dwError = LWCA_REGEX_ERROR_EBRACE;
        break;

    case REG_BADBR:
        dwError = LWCA_REGEX_ERROR_BADBR;
        break;

    case REG_ERANGE:
        dwError = LWCA_REGEX_ERROR_ERANGE;
        break;

    case REG_ESPACE:
        dwError = LWCA_REGEX_ERROR_ESPACE;
        break;

    case REG_BADRPT:
        dwError = LWCA_REGEX_ERROR_BADRPT;
        break;

    default:
        dwError = LWCA_REGEX_ERROR_UNKNOWN;
        break;
    }

    return dwError;
}
