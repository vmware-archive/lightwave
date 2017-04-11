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
 * Module Name: Directory indexer
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

VDIR_INDEX_GLOBALS gVdirIndexGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.mutex, NULL),
        VMDIR_SF_INIT(.cond, NULL),
        VMDIR_SF_INIT(.pIndexCfgMap, NULL),
        VMDIR_SF_INIT(.pIndexUpd, NULL),
        VMDIR_SF_INIT(.bFirstboot, FALSE),
        VMDIR_SF_INIT(.bLegacyDB, FALSE),
        VMDIR_SF_INIT(.offset, 0),
        VMDIR_SF_INIT(.pThrInfo, NULL)
    };
