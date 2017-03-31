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
 * Module Name: Directory ACL
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

VDIR_SID_GEN_STATE gSidGenState =
{
    // NOTE: order of fields MUST stay in sync with struct definition...
    VMDIR_SF_INIT(.mutex, NULL),
    VMDIR_SF_INIT(.pHashtable, NULL),
    VMDIR_SF_INIT(.pRIDSyncThr, NULL),
    VMDIR_SF_INIT(.pStack, NULL)
};

GENERIC_MAPPING gVmDirEntryGenericMapping = {
    VMDIR_SF_INIT(.GenericRead, VMDIR_RIGHT_DS_READ_PROP),
    VMDIR_SF_INIT(.GenericWrite, VMDIR_RIGHT_DS_WRITE_PROP),
    VMDIR_SF_INIT(.GenericExecute, VMDIR_ENTRY_GENERIC_EXECUTE),
    VMDIR_SF_INIT(.GenericAll, VMDIR_ENTRY_ALL_ACCESS)
};


