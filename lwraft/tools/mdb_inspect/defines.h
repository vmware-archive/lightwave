/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#define bail_on_error(err) if(err) goto error;

#define MAX_CMD_BUF_LEN 1024
#define MDB_PROMPT "mdb> "

#define E_MDB_EXIT       125
#define E_MDB_CMDPARAM   126

typedef enum
{
    MDB_CMD_INVALID = -1,
    MDB_CMD_OPEN,
    MDB_CMD_STAT,
    MDB_CMD_LIST,
    MDB_CMD_PGINFO,

    MDB_CMD_EXIT /*keep last*/
}MDB_CMD_ID;

typedef enum
{
    VAL_TYPE_START,
    VAL_TYPE_INT,
    VAL_TYPE_STR,
    VAL_TYPE_END
}VAL_TYPE;

typedef enum
{
    CMD_ARG_START,
    CMD_ARG_ALLDBS,
    CMD_ARG_FREE,
    CMD_ARG_SUBDB,
    CMD_ARG_PGNO,
    CMD_ARG_TYPE,
    CMD_ARG_VERBOSE,

    CMD_ARG_END
}CMD_ARG;

typedef enum
{
    ARG_TYPE_INVALID=-1,
    ARG_TYPE_ENV,
    ARG_TYPE_PAGE,
    ARG_TYPE_SUBDB
}ARG_TYPE;
