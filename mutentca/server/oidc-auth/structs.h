/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LWCA_OIDC_TOKEN_TYPE_UNKNOWN,
    LWCA_OIDC_TOKEN_TYPE_BEARER,
    LWCA_OIDC_TOKEN_TYPE_HOTK
} LWCA_OIDC_TOKEN_TYPE;

typedef struct _LWCA_OIDC_HOTK_VALUES
{
    PSTR        pszReqHOTKPEM;
    PSTR        pszReqHexPOP;
    PSTR        pszReqMethod;
    PSTR        pszReqContentType;
    PSTR        pszReqDate;
    PSTR        pszReqBody;
    PSTR        pszReqURI;
} LWCA_OIDC_HOTK_VALUES, *PLWCA_OIDC_HOTK_VALUES;

typedef struct _LWCA_OIDC_TOKEN
{
    PSTR                        pszReqAuthHdr;
    PSTR                        pszReqOIDCToken;
    LWCA_OIDC_TOKEN_TYPE        oidcTokenType;
    PLWCA_OIDC_HOTK_VALUES      pReqHOTKValues;
    PSTR                        pszReqBindUPN;
    PSTR                        pszReqBindUPNTenant;
    PLWCA_STRING_ARRAY          pReqBindUPNGroups;
} LWCA_OIDC_TOKEN, *PLWCA_OIDC_TOKEN;

#ifdef __cplusplus
}
#endif
