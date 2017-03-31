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



/*
 * Module Name: Directory ldap-head
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

#define OPSTATISTIC_BIND_INIT     {NULL, "bind",   0, 0, 0}
#define OPSTATISTIC_ADD_INIT      {NULL, "add",    0, 0, 0}
#define OPSTATISTIC_SEARCH_INIT   {NULL, "search", 0, 0, 0}
#define OPSTATISTIC_MODIFY_INIT   {NULL, "modify", 0, 0, 0}
#define OPSTATISTIC_DELETE_INIT   {NULL, "delete", 0, 0, 0}
#define OPSTATISTIC_UNBIND_INIT   {NULL, "unbind", 0, 0, 0}

VMDIR_OP_STATISTIC_GLOBALS gVmdirOPStatisticGlobals =
    {
        VMDIR_SF_INIT(.opBind,      OPSTATISTIC_BIND_INIT),
        VMDIR_SF_INIT(.opAdd,       OPSTATISTIC_ADD_INIT),
        VMDIR_SF_INIT(.opSearch,    OPSTATISTIC_SEARCH_INIT),
        VMDIR_SF_INIT(.opModify,    OPSTATISTIC_MODIFY_INIT),
        VMDIR_SF_INIT(.opDelete,    OPSTATISTIC_DELETE_INIT),
        VMDIR_SF_INIT(.opUnbind,    OPSTATISTIC_UNBIND_INIT),
    };

VMDIR_OPENSSL_GLOBALS gVmdirOpensslGlobals =
    {
        VMDIR_SF_INIT(.pMutexBuf,      NULL),
        VMDIR_SF_INIT(.dwMutexBufSize, 0),
        VMDIR_SF_INIT(.bSSLInitialized,FALSE),
    };
