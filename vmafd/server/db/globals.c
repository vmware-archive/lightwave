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
 * Module Name: VMware Certificate Server
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * VMware Certificate Server Database
 *
 * Globals
 *
 */

#include "includes.h"

#ifndef _WIN32
VECS_DB_GLOBALS gVecsDbGlobals =
    {
        .mutex                  = PTHREAD_MUTEX_INITIALIZER,
        .pszDbPath              = NULL,
        .dwMaxNumCachedContexts = VECS_DB_MAX_NUM_CACHED_CONTEXTS,
        .dwNumCachedContexts    = 0,
        .pDbContextList         = NULL
    };
#else
VECS_DB_GLOBALS gVecsDbGlobals =
    {
        VECS_DB_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
        VECS_DB_INIT(.pszDbPath, NULL),
        VECS_DB_INIT(.dwMaxNumCachedContexts, VECS_DB_MAX_NUM_CACHED_CONTEXTS),
        VECS_DB_INIT(.dwNumCachedContexts, 0),
        VECS_DB_INIT(.pDbContextList, NULL)
    };

#endif


