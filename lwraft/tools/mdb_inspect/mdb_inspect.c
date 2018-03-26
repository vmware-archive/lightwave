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

#include "includes.h"
#include <readline/readline.h>
#include <readline/history.h>

static void
sighandle(int sig)
{
    fprintf(stdout, "type exit to exit\n");
}

int main(
    int argc,
    char **argv
    )
{
    signal(SIGINT, sighandle);
    return enter_mdb_shell(argc, argv);
}

void
show_help(
    )
{
    fprintf(stdout, "mdb_inspect tool prints info on an mdb database file\n");
    fprintf(stdout, "available commands:\n");
    fprintf(stdout, "open <path to db file>: opens database. must include full path to mdb file.\n");
    fprintf(stdout, "stat [-f]: show details of main [free] db.\n");
    fprintf(stdout, "list\n");
    fprintf(stdout, "    [-v] -t subdb: show all sub db name [-v show details]\n");
    fprintf(stdout, "    [-v] -t page -p <pgno>: show page info for page no. [-v show data]\n");
    fprintf(stdout, "exit or ctrl+d: exit this program.\n");
    fprintf(stdout, "\n");
}

int
parse_cmd(
    const char *args,
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;
    PMDB_INSPECT_CMD pCmd = NULL;
    typedef int (*PFNCMDHANDLER)(PMDB_INSPECT_CMD, PMDB_INSPECT_CONTEXT);
    PFNCMDHANDLER exec = NULL;
    struct _execmap
    {
        MDB_CMD_ID cmdid;
        PFNCMDHANDLER exec;
    }execmap[] =
    {
        {MDB_CMD_OPEN, mdb_exec_open},
        {MDB_CMD_LIST, mdb_exec_list},
        {MDB_CMD_STAT, mdb_exec_stat}
    };
    int i = 0;
    int execcount = sizeof(execmap) / sizeof(execmap[0]);

    if(!args || !pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = parse_shell_cmd(args, &pCmd);
    bail_on_error(error);

    while(i < execcount)
    {
        if(execmap[i].cmdid == pCmd->cmdid)
        {
            exec = execmap[i].exec;
            break;
        }
        ++i;
    }

    if(!exec)
    {
        error = ENOENT;
        bail_on_error(error);
    }

    error = exec(pCmd, pCtxt);
    bail_on_error(error);

cleanup:
    free_inspect_cmd(pCmd);
    return error;

error:
    if(error != E_MDB_EXIT)
    {
        show_help();
        error = 0;
    }
    goto cleanup;
}

/*
    show mdb> prompt.
    accept commands and keep context
    show help
*/
int
enter_mdb_shell(
        int argc,
        char **argv
    )
{
    int error = 0;
    char *cmd_buf = NULL;
    MDB_INSPECT_CONTEXT ctx = {0};

    while(1)
    {
        cmd_buf = readline(MDB_PROMPT);
        add_history(cmd_buf);

        error = parse_cmd(cmd_buf, &ctx);
        bail_on_error(error);
    }
cleanup:
    free_db(ctx.pDB);
    return error;
error:
    goto cleanup;
}
