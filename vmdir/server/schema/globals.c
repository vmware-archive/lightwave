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
 * Module Name: Directory Schema
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

VDIR_SCHEMA_GLOBALS gVdirSchemaGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.mutex, NULL),
        VMDIR_SF_INIT(.pszDN, NULL),
        VMDIR_SF_INIT(.pCtx, NULL),
        VMDIR_SF_INIT(.pSchema, NULL),
        VMDIR_SF_INIT(.bHasPendingChange, FALSE),
        VMDIR_SF_INIT(.pLoadFromFileEntry, NULL)
    };

VDIR_SYNTAX_GLOBALS gVdirSyntaxGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.usSize, 0),
        VMDIR_SF_INIT(.pSyntax, NULL)
    };

VDIR_MATCHING_RULE_GLOBALS gVdirMatchingRuleGlobals =
    {
        // NOTE: order of fields MUST stay in sync with struct definition...
        VMDIR_SF_INIT(.usSize, 0),
        VMDIR_SF_INIT(.pMatchingRule, NULL)
    };
