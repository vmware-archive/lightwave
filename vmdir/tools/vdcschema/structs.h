/*
 * Copyright © 2016 VMware, Inc.  All Rights Reserved.
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


typedef struct _VDC_SCHEMA_CONN
{
    LDAP*   pLd;
    PSTR    pszDomain;
    PSTR    pszHostName;
    PSTR    pszUserName;
    PSTR    pszUPN;
    PSTR    pszPassword;

} VDC_SCHEMA_CONN, *PVDC_SCHEMA_CONN;

typedef enum
{
    OP_UNDEFINED,
    OP_GET_SUPPORTED_SYNTAXES,
    OP_PATCH_SCHEMA_DEFS

} VDC_SCHEMA_OP_CODE;

typedef struct _VDC_SCHEMA_OP_PARAM
{
    VDC_SCHEMA_OP_CODE  opCode;
    PSTR                pszFileName;
    BOOLEAN             bDryrun;

} VDC_SCHEMA_OP_PARAM, *PVDC_SCHEMA_OP_PARAM;
