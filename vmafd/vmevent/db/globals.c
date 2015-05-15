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
 * Module Name: ThinAppRepoService
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Thinapp Repository Database
 *
 * Globals
 *
 */

#include "includes.h"

EVENTLOG_DB_GLOBALS gEventLogDbGlobals =
    {
        VMEVENT_SF_INIT(.mutex, PTHREAD_MUTEX_INITIALIZER),
        VMEVENT_SF_INIT(.bIsDBOpened, FALSE),
        VMEVENT_SF_INIT(.pszDbPath, NULL),
        VMEVENT_SF_INIT(.dwMaxNumCachedContexts, EVENTLOG_DB_MAX_NUM_CACHED_CONTEXTS),
        VMEVENT_SF_INIT(.dwNumCachedContexts, 0),
        VMEVENT_SF_INIT(.pDbContextList, NULL)
    };
