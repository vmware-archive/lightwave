/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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
 * Module Name: vmcadb
 *
 * Filename: errorCodeMap.c
 *
 * Abstract:
 *
 * Convert error code to name/description.
 *
 */
#include "includes.h"

const char*
VmcaDbErrorCodeToName(
    int code
    )
{
    int i = 0;
    VMCA_DB_ERROR_CODE_NAME_DESC SQLITE3_Table[] = 
                                 VMCA_ERROR_SQLITE_TABLE_INITIALIZER;

    for (i=0; i<sizeof(SQLITE3_Table)/sizeof(SQLITE3_Table[0]); i++)
    {
        if ( code == SQLITE3_Table[i].code)
        {
            return SQLITE3_Table[i].name;
        }
    }

    return UNKNOWN_STRING;
}
