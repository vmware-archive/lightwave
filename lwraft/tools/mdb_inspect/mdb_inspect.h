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

int
enter_mdb_shell(
        int argc,
        char **argv
    );

int
mdb_list_open_dbs(
    PMDB_INSPECT_CONTEXT pCtxt
    );

int
mdb_print_page(
    MDB_page *pg
    );

int
mdb_print_page_v(
    MDB_page *pg,
    MDB_node *node,
    int index
    );

int
mdb_print_stat(
    PMDB_INSPECT_CONTEXT pCtxt,
    int dbi
    );

int
mdb_print_subdb(
    MDB_node *node
    );

int
mdb_print_subdb_v(
    MDB_node *node,
    MDB_db *db
    );

int
split_cmd_to_argc_argv(
    const char *cmd,
    int *argc,
    char ***argv
    );

void
free_db(
    PMDB_DB pDB
    );

void
free_inspect_context(
    PMDB_INSPECT_CONTEXT pCtxt
    );

void
free_args(
    int argc,
    char **argv
    );

void
free_value(
    PVALUE value
    );

void
free_keyvalue(
    PKEYVALUE keyvalue
    );

//cmd
int
set_arg_int(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    int intval
    );

int
set_arg_str(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    const char *strval
    );

int
get_arg_int(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG arg,
    int *intval
    );

int
get_arg_int_opt(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG arg,
    const int defval,
    int *intval
    );

int
get_arg_str(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG arg,
    const char **strval
    );

int
get_arg_str_opt(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG arg,
    const char *defval,
    const char **strval
    );

void
free_inspect_cmd(
    PMDB_INSPECT_CMD pCmd
    );

//parse
int
parse_shell_cmd(
    const char *args,
    PMDB_INSPECT_CMD *ppCmd
    );

int
mdb_parse_open(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    );

int
mdb_parse_stat(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    );

int
mdb_parse_list(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    );

int
mdb_parse_pginfo(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    );

int
mdb_parse_exit(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    );

//exec
int
mdb_exec_open(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    );

int
mdb_exec_list(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    );

int
mdb_exec_info(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    );

int
mdb_exec_stat(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    );
