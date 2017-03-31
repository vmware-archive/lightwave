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
 * Module Name: Directory Middle-layer
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

PVDIR_PASSWORD_HASH_SCHEME gpVdirPasswdSchemeGlobals = NULL;

VDIR_PAGED_SEARCH_CACHE     gPagedSearchCache =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.mutex, NULL),
        VMDIR_SF_INIT(.pHashTbl, NULL),
    };

VDIR_LOCKOUT_CACHE          gVdirLockoutCache =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.mutex, NULL),
        VMDIR_SF_INIT(.pHashTbl, NULL)
    };
