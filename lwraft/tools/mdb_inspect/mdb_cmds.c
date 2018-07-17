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

int
mdb_exec_open(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;

    if(!pCmd || !pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = mdb_open(pCmd->extra_argv[0], pCtxt);
    bail_on_error(error);

    fprintf(stdout, "opened file: %s\n", pCmd->extra_argv[0]);
cleanup:
    return error;

error:
    goto cleanup;
}

int
mdb_exec_stat(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;
    int alldbs = 0;
    int freeinfo = 0;

    if(!pCmd || !pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = get_arg_int_opt(pCmd, CMD_ARG_ALLDBS, 0, &alldbs);
    bail_on_error(error);

    error = get_arg_int_opt(pCmd, CMD_ARG_FREE, 0, &freeinfo);
    bail_on_error(error);

    error = mdb_read_env(pCtxt->pDB);
    bail_on_error(error);

    mdb_print_stat(pCtxt, freeinfo?FREE_DBI:MAIN_DBI);
cleanup:
    return error;

error:
    goto cleanup;
}

int
mdb_exec_list(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;
    ARG_TYPE type = ARG_TYPE_INVALID;
    int pgno = 0;
    int verbose = 0;
    int free = 0;
    const char *subdb = NULL;

    if(!pCmd || !pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = get_arg_int(pCmd, CMD_ARG_TYPE, &type);
    bail_on_error(error);

    error = get_arg_int_opt(pCmd, CMD_ARG_VERBOSE, 0, &verbose);
    bail_on_error(error);

    switch(type)
    {
        case ARG_TYPE_ENV:
            error = mdb_list_open_dbs(pCtxt);
            bail_on_error(error);
        break;
        case ARG_TYPE_PAGE:
            error = get_arg_int(pCmd, CMD_ARG_PGNO, &pgno);
            bail_on_error(error);

            if(verbose)
            {
                error = mdb_page_get_v(
                            pCtxt->pDB,
                            pgno,
                            pgno,
                            mdb_print_page,
                            mdb_print_page_v);
                bail_on_error(error);
            }
            else
            {
                error = mdb_page_get(pCtxt->pDB, pgno, pgno, mdb_print_page);
                bail_on_error(error);
            }
        break;
        case ARG_TYPE_SUBDB:
            error = get_arg_str_opt(pCmd, CMD_ARG_SUBDB, NULL, &subdb);
            bail_on_error(error);
            if(verbose)
            {
                error = mdb_subdb_get_v(
                            pCtxt->pDB,
                            subdb,
                            mdb_print_subdb_v);
                bail_on_error(error);
            }
            else
            {
                error = mdb_subdb_get(
                            pCtxt->pDB,
                            subdb,
                            mdb_print_subdb);
                bail_on_error(error);
            }
        break;
    }

cleanup:
    return error;

error:
    goto cleanup;
}

int
mdb_exec_pg_info(
    PMDB_INSPECT_CMD pCmd,
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;
    int pgno = 0;
    MDB_page *pg = NULL;

    if(!pCmd || !pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = get_arg_int(pCmd, CMD_ARG_PGNO, &pgno);
    bail_on_error(error);

    error = mdb_page_info(pCtxt->pDB, pgno, &pg);
    bail_on_error(error);

    mdb_print_page(pg);
cleanup:
    return error;

error:
    goto cleanup;
}
