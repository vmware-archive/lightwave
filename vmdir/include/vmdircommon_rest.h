/*
 * Copyright © 2019 VMware, Inc.  All Rights Reserved.
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

#ifndef VMDIR_COMMON_REST_H_
#define VMDIR_COMMON_REST_H_

#define OIDC_TOKEN_SCOPE_VMDIR   "openid rs_vmdir"
#define OIDC_DEFAULT_PORT          443

typedef enum _VMDIR_OIDC_ACQUIRE_TOKEN_METHOD
{
    METHOD_PASSWORD = 1,
    METHOD_REFRESH_TOKEN,
    METHOD_CERTIFICATE
} VMDIR_OIDC_ACQUIRE_TOKEN_METHOD;

typedef struct _VMDIR_OIDC_ACQUIRE_TOKEN_INFO
{
    VMDIR_OIDC_ACQUIRE_TOKEN_METHOD method;
    PCSTR    pszLocalTLSCertPath;
    PCSTR    pszDomain;
    PCSTR    pszScope;
    PCSTR    pszUPN;
    PCSTR    pszPassword;
    PCSTR    pszRefreshToken;
    PCSTR    pszCertSubjectDN;
    PCSTR    pszCertPrivateKeyPEM;
} VMDIR_OIDC_ACQUIRE_TOKEN_INFO, *PVMDIR_OIDC_ACQUIRE_TOKEN_INFO;

typedef struct _VMDIR_JSON_RESULT_LDAP_SEARCH_RESPONSE
{
    int  iLdapCode;
    PSTR pszLdapMsg;
    PSTR pszPagedCookies;
    int  iResultCount;

} VMDIR_JSON_RESULT_LDAP_SEARCH_RESPONSE, *PVMDIR_JSON_RESULT_LDAP_SEARCH_RESPONSE;

#endif /* VMDIR_COMMON_REST_H_ */
