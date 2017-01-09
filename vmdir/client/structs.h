/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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


#ifndef _CLIENT_STRUCTS_H_
#define _CLIENT_STRUCTS_H_

/* definitions for public incomplete types */
struct _VMDIR_SERVER_CONTEXT
{
    handle_t hBinding;
};

typedef struct _VMDIR_CONNECTION
{
    LDAP* pLd;
    PSTR  pszDomain;
} VMDIR_CONNECTION;

typedef struct _REPLICATION_INFO
{
    CHAR       pszURI[VMDIR_MAX_LDAP_URI_LEN];
} REPLICATION_INFO, *PREPLICATION_INFO;

typedef struct _INTERNAL_SERVER_INFO
{
    CHAR       pszServerDN[VMDIR_MAX_DN_LEN];
} INTERNAL_SERVER_INFO, *PINTERNAL_SERVER_INFO;

typedef struct _VMDIR_REPLICATION_COOKIE
{
    PSTR pszUtdVector;
    PSTR pszInvocationId;
    USN   lastLocalUsnProcessed;
} VMDIR_REPLICATION_COOKIE, *PVMDIR_REPLICATION_COOKIE;

typedef struct _VMDIR_ERROR_CODE_DESC
{
    DWORD code;
    PCSTR desc;
} VMDIR_ERROR_CODE_DESC, *PVMDIR_ERROR_CODE_DESC;

#endif
