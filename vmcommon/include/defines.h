/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef __VM_COMMON_DEFINE_H__
#define __VM_COMMON_DEFINE_H__

#define BUFFER_SIZE                     1024
#define VM_COMMON_MAX_TIME_BYTES        128
#define VM_COMMON_TIME_FORMAT_RFC_1233      "%a, %d %b %Y %H:%M:%S GMT"
#define VM_COMMON_HTTP_CONTENT_TYPE_KEY     "Content-Type"
#define VM_COMMON_HTTP_CONTENT_TYPE_JSON    "application/json"
#define VM_COMMON_HTTP_DATE                 "Date"

#define HEADER_BEARER_AUTH "Authorization: Bearer %s"
#define HEADER_HOTK_PK_AUTH "Authorization: hotk-pk %s"

#define VM_COMMON_SAFE_FREE_MEMORY(PTR)       \
    do {                                       \
        if ((PTR)) {                           \
            VmFreeMemory(PTR);          \
            (PTR) = NULL;                      \
        }                                      \
    } while(0)

#define VM_COMMON_SAFE_FREE_CURL_MEMORY(PTR)   \
    do {                                       \
        if ((PTR)) {                           \
            curl_free(PTR);          \
            (PTR) = NULL;                      \
        }                                      \
    } while(0)

#define BAIL_ON_VM_COMMON_ERROR(dwError)       \
    if (dwError)                               \
    {                                          \
        goto error;                            \
    }

#define BAIL_WITH_VM_COMMON_ERROR(dwError, ERROR_CODE)  \
    dwError = ERROR_CODE;                               \
    BAIL_ON_VM_COMMON_ERROR(dwError);

#define BAIL_ON_VM_COMMON_INVALID_PARAMETER(ptr, dwError)                           \
    if ((ptr) == NULL)                                                              \
    {                                                                               \
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);      \
    }

#define BAIL_ON_VM_COMMON_INVALID_STR_PARAMETER(ptr, dwError)                       \
    if (IsNullOrEmptyString((ptr)))                                                 \
    {                                                                               \
        BAIL_WITH_VM_COMMON_ERROR(dwError, VM_COMMON_ERROR_INVALID_PARAMETER);      \
    }

#define VM_COMMON_SAFE_FREE_STRINGA(PTR)    \
    do {                                    \
        if ((PTR)) {                        \
            VmFreeStringA(PTR);             \
            (PTR) = NULL;                   \
        }                                   \
    } while(0)

#define VM_COMMON_SAFE_STRING(str) ((str) ? (str) : "")


#define VM_COMMON_ASCII_SPACE(c) \
    ( (c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' )

#define BAIL_AND_LOG_ON_VM_COMMON_ERROR(x, str) \
    if ((x) != 0)                                            \
    {                                                        \
        fprintf(stderr, "error [%u] in file [%s] function [%s] at line [%d] with message [%s]\n", x, __FILE__, __FUNCTION__, __LINE__, VM_COMMON_SAFE_STRING(str)); \
        goto error;                                          \
    }

#define BAIL_AND_LOG_ON_VM_COMMON_CURL_ERROR(x, curlCode) \
    if (curlCode != CURLE_OK)                             \
    {                                                     \
        x = VM_COMMON_ERROR_CURL_FAILURE;                 \
        BAIL_AND_LOG_ON_VM_COMMON_ERROR(x, curl_easy_strerror(curlCode)); \
    }

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !*(str))
#endif

#define VM_COMMON_GET_BASE64_ENCODE_LEN(x) ((x / 3 + 1) * 4 + 1)

#endif /* __VM_COMMON_DEFINE_H__ */
