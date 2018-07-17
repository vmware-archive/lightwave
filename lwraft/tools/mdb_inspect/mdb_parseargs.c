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
#include <getopt.h>

int
process_cmd_line(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    int error = 0;
    typedef int (*PFNPARSEHANDLER)(int, char **, PMDB_INSPECT_CMD *);
    struct _cmdmap
    {
        MDB_CMD_ID cmdid;
        const char *cmdname;
        PFNPARSEHANDLER handler;
    };
    struct _cmdmap cmdmap[] =
    {
        {MDB_CMD_OPEN, "open", mdb_parse_open},
        {MDB_CMD_STAT, "stat", mdb_parse_stat},
        {MDB_CMD_LIST, "list", mdb_parse_list},
        {MDB_CMD_EXIT, "exit", mdb_parse_exit},
    };
    int i = 0;
    int cmdcount = sizeof(cmdmap)/sizeof(cmdmap[0]);
    PMDB_INSPECT_CMD pCmd = NULL;
    struct _cmdmap *match = NULL;

    if(argc < 1 || !argv)
    {
       error = EINVAL;
       bail_on_error(error);
    }

    for(i = 0; i < cmdcount; ++i)
    {
        if(!strcmp(argv[0], cmdmap[i].cmdname))
        {
            match = &cmdmap[i];
            break;
        }
    }

    if(!match)
    {
        error = ENOENT;
        bail_on_error(error);
    }

    error = match->handler(argc, argv, &pCmd);
    bail_on_error(error);

    pCmd->cmdid = match->cmdid;
    *ppCmd = pCmd;

cleanup:
    return error;
error:
    free_inspect_cmd(pCmd);
    goto cleanup;
}

/*
    split a multi part command to argc, argv
    use getopt to parse and return results
*/
int
parse_shell_cmd(
    const char *args,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    int error = 0;
    int argc = 0;
    char **argv = 0;
    PMDB_INSPECT_CMD pCmd = NULL;

    if(!args || !ppCmd)
    {
       error = EINVAL;
       bail_on_error(error);
    }

    error = split_cmd_to_argc_argv(args, &argc, &argv);
    bail_on_error(error);

    if(argc < 1)
    {
       error = EINVAL;
       bail_on_error(error);
    }

    error = process_cmd_line(argc, argv, &pCmd);
    bail_on_error(error);

    *ppCmd = pCmd;
cleanup:
    free_args(argc, argv);
    return error;

error:
    free_inspect_cmd(pCmd);
    goto cleanup;
}

int
set_arg_value(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    PVALUE value
    )
{
    int error = 0;
    PKEYVALUE *keyvalue = NULL;
    PKEYVALUE temp = NULL;
    PKEYVALUE last = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END || !value)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    if(pCmd->options)
    {
        PKEYVALUE options = pCmd->options;
        for(; options; options = options->next)
        {
            if(options->argid == argid)
            {
                error = EALREADY;
                bail_on_error(error);
            }
            last = options;
        }
    }

    temp = (PKEYVALUE)calloc(1, sizeof(KEYVALUE));
    if(!temp)
    {
        error = ENOMEM;
        bail_on_error(error);
    }
    temp->argid = argid;
    temp->value = value;

    if(last)
    {
        last->next = temp;
    }
    else
    {
        pCmd->options = temp;
    }

cleanup:
    return 0;

error:
    goto cleanup;
}

int
set_arg_int(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    int intval
    )
{
    int error = 0;
    PVALUE value = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    value = (PVALUE)calloc(1, sizeof(VALUE));
    value->type = VAL_TYPE_INT;
    value->intval = intval;

    error = set_arg_value(pCmd, argid, value);
    bail_on_error(error);

cleanup:
    return error;
error:
    free_value(value);
    goto cleanup;
}

int
set_arg_str(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    const char *strval
    )
{
    int error = 0;
    PVALUE value = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END || !strval)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    value = (PVALUE)calloc(1, sizeof(VALUE));
    value->type = VAL_TYPE_STR;
    value->strval = strdup(strval);

    error = set_arg_value(pCmd, argid, value);
    bail_on_error(error);

cleanup:
    return error;
error:
    free_value(value);
    goto cleanup;
}

int
get_arg_value(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    PVALUE *ppvalue
    )
{
    int error = 0;
    PVALUE value = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END || !ppvalue)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    if(pCmd->options)
    {
        PKEYVALUE keyval = pCmd->options;
        while(keyval)
        {
            if(keyval->argid == argid)
            {
                value = keyval->value;
                break;
            }
            keyval = keyval->next;
        }
    }

    if(!value)
    {
        error = ENOENT;
        bail_on_error(error);
    }

    *ppvalue = value;
cleanup:
    return error;
error:
    goto cleanup;
}

int
get_arg_int(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    int *intval
    )
{
    int error = 0;
    PVALUE value = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END || !intval)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = get_arg_value(pCmd, argid, &value);
    bail_on_error(error);

    *intval = value->intval;
cleanup:
    return error;
error:
    goto cleanup;
}

int
get_arg_int_opt(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    const int defvalue,
    int *intval
    )
{
    int error = 0;
    error = get_arg_int(pCmd, argid, intval);
    if(error == ENOENT)
    {
        *intval = defvalue;
        error = 0;
    }
    return error;
}

int
get_arg_str_opt(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    const char *defvalue,
    const char **strval
    )
{
    int error = 0;
    error = get_arg_str(pCmd, argid, strval);
    if(error == ENOENT)
    {
        *strval = defvalue;
        error = 0;
    }
    return error;
}

int
get_arg_str(
    PMDB_INSPECT_CMD pCmd,
    CMD_ARG argid,
    const char **strval
    )
{
    int error = 0;
    PVALUE value = NULL;

    if(!pCmd || argid <= CMD_ARG_START || argid >= CMD_ARG_END || !strval)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = get_arg_value(pCmd, argid, &value);
    bail_on_error(error);

    *strval = value->strval;
cleanup:
    return error;
error:
    goto cleanup;
}

int
mdb_parse_open(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    int error = 0;
    PMDB_INSPECT_CMD pCmd = NULL;

    if(!argv || !ppCmd)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    //open requires 2 args
    if(argc != 2)
    {
        error = E_MDB_CMDPARAM;
        bail_on_error(error);
    }

    pCmd = (PMDB_INSPECT_CMD)calloc(1, sizeof(MDB_INSPECT_CMD));
    if(!pCmd)
    {
        error = ENOMEM;
        bail_on_error(error);
    }

    pCmd->cmd = strdup(argv[0]);

    pCmd->extra_argv = (char **)calloc(1, sizeof(char *));
    pCmd->extra_argv[0] = strdup(argv[1]);
    pCmd->extra_argc = 1;

    *ppCmd = pCmd;
cleanup:
    return error;
error:
    free_inspect_cmd(pCmd);
    goto cleanup;
}

int
translate_type(
    const char *arg,
    ARG_TYPE *type
    )
{
    int error = 0;
    int i = 0;
    struct _translate
    {
        const char * arg;
        ARG_TYPE type;
    }translate[] =
    {
        {"env", ARG_TYPE_ENV},
        {"page", ARG_TYPE_PAGE},
        {"subdb", ARG_TYPE_SUBDB},
    };
    struct _translate *found = NULL;

    if(!arg || !type)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    i = sizeof(translate)/sizeof(translate[0]);
    while(i > 0)
    {
        if(!strcmp(arg, translate[i-1].arg))
        {
            found = &translate[i-1];
            break;
        }
        --i;
    }

    if(!found)
    {
        error = ENOENT;
        bail_on_error(error);
    }

    *type = found->type;
error:
    return error;
}

int
mdb_parse_list(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    int error = 0;
    int i = 0;
    int pgno = 0;
    PMDB_INSPECT_CMD pCmd = NULL;
    int index = 0;
    ARG_TYPE type = ARG_TYPE_INVALID;
    optind = 0;
    opterr = 0;
    optopt = 0;

    if(!argv || !ppCmd)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pCmd = (PMDB_INSPECT_CMD)calloc(1, sizeof(MDB_INSPECT_CMD));
    if(!pCmd)
    {
        error = ENOMEM;
        bail_on_error(error);
    }

    while(1)
    {
        struct option options[] =
        {
            {"pgno", required_argument, 0, 'p'},
            {"subdb", required_argument, 0, 's'},
            {"type", required_argument, 0, 't'},
            {"verbose", required_argument, 0, 'v'},
            {0, 0, 0, 0}
        };
        i = getopt_long(argc, argv, "p:t:v", options, &index);
        if(i == -1)
            break;
        switch(i)
        {
            case 'p':
                pgno = strtol(optarg, NULL, 10);
                error = errno;
                bail_on_error(error);

                error = set_arg_int(pCmd, CMD_ARG_PGNO, pgno);
                bail_on_error(error);
            break;
            case 's':
                error = set_arg_str(pCmd, CMD_ARG_SUBDB, optarg);
                bail_on_error(error);
            break;
            case 't':
                error = translate_type(optarg, &type);
                bail_on_error(error);

                error = set_arg_int(pCmd, CMD_ARG_TYPE, type);
                bail_on_error(error);
            break;
            case 'v':
                error = set_arg_int(pCmd, CMD_ARG_VERBOSE, 1);
                bail_on_error(error);
            break;
        }
    }

    pCmd->cmd = strdup(argv[0]);

    *ppCmd = pCmd;
cleanup:
    return error;
error:
    fprintf(stdout, "parse list failed: %d\n", error);
    free_inspect_cmd(pCmd);
    goto cleanup;
}

int
mdb_parse_stat(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    int error = 0;
    int i = 0;
    PMDB_INSPECT_CMD pCmd = NULL;
    int index = 0;
    optind = 0;
    opterr = 0;
    optopt = 0;

    if(!argv || !ppCmd)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pCmd = (PMDB_INSPECT_CMD)calloc(1, sizeof(MDB_INSPECT_CMD));
    if(!pCmd)
    {
        error = ENOMEM;
        bail_on_error(error);
    }

    while(1)
    {
        struct option options[] =
        {
            {"all", no_argument, 0, 'a'},
            {"free", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };
        i = getopt_long(argc, argv, "afs:", options, &index);
        if(i == -1)
            break;
        switch(i)
        {
            case 'a':
                error = set_arg_int(pCmd, CMD_ARG_ALLDBS, 1);
                bail_on_error(error);
            break;
            case 'f':
                error = set_arg_int(pCmd, CMD_ARG_FREE, 1);
                bail_on_error(error);
            break;
        }
    }

    pCmd->cmd = strdup(argv[0]);

    *ppCmd = pCmd;
cleanup:
    return error;
error:
    fprintf(stdout, "parse stat failed: %d\n", error);
    free_inspect_cmd(pCmd);
    goto cleanup;
}

int
mdb_parse_exit(
    int argc,
    char **argv,
    PMDB_INSPECT_CMD *ppCmd
    )
{
    return E_MDB_EXIT;
}
