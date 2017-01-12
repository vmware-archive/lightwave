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



#include "includes.h"

//clean up the code once bug#929558 gets fixed
int  ldap_syslog = 0;
#ifndef _WIN32
int  slap_debug = 0;
#endif

PCSTR gGroupWhiteList[] = {
    GROUP_ADMINISTRATORS,
    GROUP_USERS,
    GROUP_ACTASUSERS,
    GROUP_CMADMINISTRATORS,
    GROUP_SOLUTIONUSERS,
    NULL
};

VMDIR_ERROR_CODE_DESC VMDIR_ERROR_Table[] =
                                 VMDIR_ERROR_TABLE_INITIALIZER;
DWORD VMDIR_ERROR_Table_size = sizeof(VMDIR_ERROR_Table)/sizeof(VMDIR_ERROR_Table[0]);

VMDIR_ERROR_CODE_DESC VMDIR_RPC_ERROR_Table[] =
                                 VMDIR_RPC_ERROR_TABLE_INITIALIZER;
DWORD VMDIR_RPC_ERROR_Table_size = sizeof(VMDIR_RPC_ERROR_Table)/sizeof(VMDIR_RPC_ERROR_Table[0]);

VMDIR_DFL_VERSION_MAP VMDIR_DFL_VERSION_Table[] =
    VMDIR_DFL_VERSION_INITIALIZER;
DWORD VMDIR_DFL_VERSION_Table_size = sizeof(VMDIR_DFL_VERSION_Table)/sizeof(VMDIR_DFL_VERSION_Table[0]);

