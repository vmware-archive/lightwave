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

static const char* const VALUE_TO_CHAR_MAP = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

static
SSOERROR
SSOBase64UrlCharToValue(
    char c,
    unsigned char* pValue)
{
    SSOERROR e = SSOERROR_NONE;

    unsigned char value = 0;

    if ('A' <= c && c <= 'Z')
    {
        value = c - 'A';
    }
    else if ('a' <= c && c <= 'z')
    {
        value = c - 'a' + 26; // 26: num of chars in [A-Z]
    }
    else if ('0' <= c && c <= '9')
    {
        value = c - '0' + 26 + 26; // num of chars in [A-Z][a-z]
    }
    else if ('-' == c)
    {
        value = 62;
    }
    else if ('_' == c)
    {
        value = 63;
    }
    else
    {
        // input char is not a valid base64url char
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    *pValue = value;

error:

    return e;
}

SSOERROR
SSOBase64UrlEncodeToString(
    const unsigned char* pInput,
    size_t inputLength,
    char** ppszOutput /* OUT */)
{
    SSOERROR e = SSOERROR_NONE;
    char* pszOutput = NULL;
    size_t inputLengthModulo = inputLength % 3;
    size_t inputIndex = 0;
    size_t outputIndex = 0;
    size_t outputLength = ((inputLength / 3) * 4) + 1; // +1 for string terminator

    ASSERT_NOT_NULL(pInput);
    ASSERT_NOT_NULL(ppszOutput);

    if (2 == inputLengthModulo)
    {
        outputLength += 3;
    }
    else if (1 == inputLengthModulo)
    {
        outputLength += 2;
    }

    e = SSOMemoryAllocateArray(outputLength, sizeof(char), (void**) &pszOutput);
    BAIL_ON_ERROR(e);

    for (inputIndex = 0; inputIndex < (inputLength / 3); inputIndex++)
    {
        unsigned char inputOctet1 = pInput[inputIndex * 3 + 0];
        unsigned char inputOctet2 = pInput[inputIndex * 3 + 1];
        unsigned char inputOctet3 = pInput[inputIndex * 3 + 2];

        unsigned char inputSextet1 = inputOctet1 >> 2;
        unsigned char inputSextet2 = ((inputOctet1 & 3) << 4) | (inputOctet2 >> 4);
        unsigned char inputSextet3 = ((inputOctet2 & 15) << 2) | (inputOctet3 >> 6);
        unsigned char inputSextet4 = inputOctet3 & 63;

        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet1];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet2];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet3];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet4];
    }

    if (2 == inputLengthModulo)
    {
        unsigned char inputOctet1 = pInput[inputIndex * 3 + 0];
        unsigned char inputOctet2 = pInput[inputIndex * 3 + 1];

        unsigned char inputSextet1 = inputOctet1 >> 2;
        unsigned char inputSextet2 = ((inputOctet1 & 3) << 4) | (inputOctet2 >> 4);
        unsigned char inputSextet3 = (inputOctet2 & 15) << 2;

        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet1];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet2];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet3];
    }
    else if (1 == inputLengthModulo)
    {
        unsigned char inputOctet1 = pInput[inputIndex * 3 + 0];

        unsigned char inputSextet1 = inputOctet1 >> 2;
        unsigned char inputSextet2 = (inputOctet1 & 3) << 4;

        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet1];
        pszOutput[outputIndex++] = VALUE_TO_CHAR_MAP[inputSextet2];
    }

    *ppszOutput = pszOutput;

error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pszOutput, outputLength);
    }

    return e;
}

SSOERROR
SSOBase64UrlDecodeToString(
    const char* pszInput,
    char** ppszOutput /* OUT */)
{
    size_t outputLength = 0;
    return SSOBase64UrlDecodeToBytes(pszInput, (unsigned char**) ppszOutput, &outputLength);
}

SSOERROR
SSOBase64UrlDecodeToBytes(
    const char* pszInput,
    unsigned char** ppOutput, /* OUT */
    size_t* pOutputLength)
{
    SSOERROR e = SSOERROR_NONE;
    unsigned char* pOutput = NULL;
    size_t inputLength = 0;
    size_t inputLengthModulo = 0;
    size_t inputIndex = 0;
    size_t outputIndex = 0;
    size_t outputLength = 0;

    ASSERT_NOT_NULL(pszInput);
    ASSERT_NOT_NULL(ppOutput);
    ASSERT_NOT_NULL(pOutputLength);

    inputLength = SSOStringLength(pszInput);
    inputLengthModulo = inputLength % 4;
    outputLength = (inputLength / 4) * 3;
    if (3 == inputLengthModulo)
    {
        outputLength += 2;
    }
    else if (2 == inputLengthModulo)
    {
        outputLength += 1;
    }
    else if (1 == inputLengthModulo)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    // outputLength + 1 for when called by SSOBase64UrlDecodeToString
    e = SSOMemoryAllocateArray(outputLength + 1, sizeof(unsigned char), (void**) &pOutput);
    BAIL_ON_ERROR(e);

    for (inputIndex = 0; inputIndex < (inputLength / 4); inputIndex++)
    {
        char inputChar1 = pszInput[inputIndex * 4 + 0];
        char inputChar2 = pszInput[inputIndex * 4 + 1];
        char inputChar3 = pszInput[inputIndex * 4 + 2];
        char inputChar4 = pszInput[inputIndex * 4 + 3];

        unsigned char inputSextet1 = 0;
        unsigned char inputSextet2 = 0;
        unsigned char inputSextet3 = 0;
        unsigned char inputSextet4 = 0;

        e = SSOBase64UrlCharToValue(inputChar1, &inputSextet1);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar2, &inputSextet2);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar3, &inputSextet3);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar4, &inputSextet4);
        BAIL_ON_ERROR(e);

        pOutput[outputIndex++] = (inputSextet1 << 2) | (inputSextet2 >> 4);
        pOutput[outputIndex++] = (inputSextet2 << 4) | (inputSextet3 >> 2);
        pOutput[outputIndex++] = (inputSextet3 << 6) | inputSextet4;
    }

    if (3 == inputLengthModulo)
    {
        char inputChar1 = pszInput[inputIndex * 4 + 0];
        char inputChar2 = pszInput[inputIndex * 4 + 1];
        char inputChar3 = pszInput[inputIndex * 4 + 2];

        unsigned char inputSextet1 = 0;
        unsigned char inputSextet2 = 0;
        unsigned char inputSextet3 = 0;

        e = SSOBase64UrlCharToValue(inputChar1, &inputSextet1);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar2, &inputSextet2);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar3, &inputSextet3);
        BAIL_ON_ERROR(e);

        pOutput[outputIndex++] = (inputSextet1 << 2) | (inputSextet2 >> 4);
        pOutput[outputIndex++] = (inputSextet2 << 4) | (inputSextet3 >> 2);
    }
    else if (2 == inputLengthModulo)
    {
        char inputChar1 = pszInput[inputIndex * 4 + 0];
        char inputChar2 = pszInput[inputIndex * 4 + 1];

        unsigned char inputSextet1 = 0;
        unsigned char inputSextet2 = 0;

        e = SSOBase64UrlCharToValue(inputChar1, &inputSextet1);
        BAIL_ON_ERROR(e);

        e = SSOBase64UrlCharToValue(inputChar2, &inputSextet2);
        BAIL_ON_ERROR(e);

        pOutput[outputIndex++] = (inputSextet1 << 2) | (inputSextet2 >> 4);
    }

    *ppOutput = pOutput;
    *pOutputLength = outputLength;

error:

    if (e != SSOERROR_NONE)
    {
        SSOMemoryFree(pOutput, outputLength);
    }

    return e;
}
