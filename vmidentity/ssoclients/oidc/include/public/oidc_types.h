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

#ifndef _OIDC_TYPES_H_
#define _OIDC_TYPES_H_

typedef       struct OIDC_CLIENT*  POIDC_CLIENT;
typedef const struct OIDC_CLIENT* PCOIDC_CLIENT;

typedef       struct OIDC_SERVER_METADATA*  POIDC_SERVER_METADATA;
typedef const struct OIDC_SERVER_METADATA* PCOIDC_SERVER_METADATA;

typedef       struct OIDC_TOKEN_SUCCESS_RESPONSE*  POIDC_TOKEN_SUCCESS_RESPONSE;
typedef const struct OIDC_TOKEN_SUCCESS_RESPONSE* PCOIDC_TOKEN_SUCCESS_RESPONSE;

typedef       struct OIDC_ERROR_RESPONSE*  POIDC_ERROR_RESPONSE;
typedef const struct OIDC_ERROR_RESPONSE* PCOIDC_ERROR_RESPONSE;

typedef       struct OIDC_ID_TOKEN*  POIDC_ID_TOKEN;
typedef const struct OIDC_ID_TOKEN* PCOIDC_ID_TOKEN;

typedef       struct OIDC_ACCESS_TOKEN*  POIDC_ACCESS_TOKEN;
typedef const struct OIDC_ACCESS_TOKEN* PCOIDC_ACCESS_TOKEN;

typedef enum OIDC_TOKEN_TYPE {
    OIDC_TOKEN_TYPE_BEARER,
    OIDC_TOKEN_TYPE_HOK
} OIDC_TOKEN_TYPE;

#endif
