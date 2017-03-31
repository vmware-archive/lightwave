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

#ifndef _COMMON_TYPES_H_
#define _COMMON_TYPES_H_

typedef void (*GenericDestructorFunction)(void*);

typedef       struct SSO_HTTP_CLIENT*  PSSO_HTTP_CLIENT;
typedef const struct SSO_HTTP_CLIENT* PCSSO_HTTP_CLIENT;

typedef       struct SSO_KEY_VALUE_PAIR*  PSSO_KEY_VALUE_PAIR;
typedef const struct SSO_KEY_VALUE_PAIR* PCSSO_KEY_VALUE_PAIR;

typedef       struct SSO_STRING_BUILDER*  PSSO_STRING_BUILDER;
typedef const struct SSO_STRING_BUILDER* PCSSO_STRING_BUILDER;

typedef       struct SSO_JSON*  PSSO_JSON;
typedef const struct SSO_JSON* PCSSO_JSON;

typedef       struct SSO_JSON_ITERATOR*  PSSO_JSON_ITERATOR;
typedef const struct SSO_JSON_ITERATOR* PCSSO_JSON_ITERATOR;

typedef       struct SSO_JWT*  PSSO_JWT;
typedef const struct SSO_JWT* PCSSO_JWT;

typedef       struct SSO_JWK*  PSSO_JWK;
typedef const struct SSO_JWK* PCSSO_JWK;

typedef       struct SSO_CDC*  PSSO_CDC;
typedef const struct SSO_CDC* PCSSO_CDC;

#ifdef _WIN32
typedef HINSTANCE       PSSO_LIB_HANDLE;
typedef FARPROC         PSSO_FUNC_HANDLE;
#else
#include <dlfcn.h>
typedef void*           PSSO_LIB_HANDLE;
typedef void*           PSSO_FUNC_HANDLE;
#endif

typedef enum
{
    SSO_DIGEST_METHOD_MD5,
    SSO_DIGEST_METHOD_SHA256
} SSO_DIGEST_METHOD;

// for test code

typedef struct
{
    PCSTRING pszFunctionName;
    bool (*function)();
} TEST_CASE;

#endif
