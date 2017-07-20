/*
 * Copyright © 207 VMware, Inc.  All Rights Reserved.
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


#ifndef _VMDIRCLIENT_TEST_STRUCTS_H_
#define _VMDIRCLIENT_TEST_STRUCTS_H_

typedef struct _VMDIRCLIENT_TEST_CONTEXT
{
    PSTR    pszServerName;
    PSTR    pszDomainName;
    PSTR    pszUserName;
    PSTR    pszPassword;
    PSTR    pszUPN;
    LDAP*   pLd;
} VMDIRCLIENT_TEST_CONTEXT, *PVMDIRCLIENT_TEST_CONTEXT;

#endif
