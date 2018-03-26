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

typedef struct _MDB_ENV_
{
    pgno_t pagecount;
    pgno_t root;
    MDB_db *meta[NUM_METAS];
}MDB_ENV, *PMDB_ENV;

typedef struct _MDB_DB_
{
    int nNum;
    char *pszDBPath;
    int fd;
    char *pMapAddr;
    off_t nMapLength;
    MDB_ENV env;
    struct _MDB_DB_ *pNext;
}MDB_DB, *PMDB_DB;

typedef struct _MDB_INSPECT_CONTEXT_
{
    PMDB_DB pDB;
}MDB_INSPECT_CONTEXT, *PMDB_INSPECT_CONTEXT;

typedef struct _VALUE_
{
    ARG_TYPE type;
    union
    {
        int intval;
        char *strval;
    };
}VALUE, *PVALUE;

typedef struct _KEYVALUE_
{
    CMD_ARG argid;
    PVALUE value;
    struct _KEYVALUE_ *next;
}KEYVALUE, *PKEYVALUE;

typedef struct _MDB_INSPECT_CMD_
{
    MDB_CMD_ID cmdid;
    char *cmd;
    int extra_argc;
    char **extra_argv;
    PKEYVALUE options;
}MDB_INSPECT_CMD, *PMDB_INSPECT_CMD;

//misc
typedef int (*PFNVISITCB)(const char item, void *userdata);
typedef struct _args_
{
    char lastchar;
    char buff[MAX_CMD_BUF_LEN];
    int buffindex;
    int argc;
    char **argv;
}CMDARGS;
