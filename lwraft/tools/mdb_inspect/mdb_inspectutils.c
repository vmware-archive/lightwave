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
mdb_list_open_dbs(
    PMDB_INSPECT_CONTEXT pCtxt
    )
{
    int error = 0;

    if(!pCtxt)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    if(pCtxt->pDB)
    {
        fprintf(stdout, "File: %s\n", pCtxt->pDB->pszDBPath);
        fprintf(stdout, "Length: %ld\n", pCtxt->pDB->nMapLength);
    }

error:
    return error;
}

const char *
page_type_str(
    int pg_flag
    )
{
    switch(pg_flag)
    {
        case P_BRANCH: return "branch";
        case P_LEAF:   return "leaf";
        case P_META:   return "meta";
    }
    return "other";
}

int
mdb_print_page_v(
    MDB_page *pg,
    MDB_node *node,
    int index
    )
{
    if(pg->mp_flags == P_BRANCH)
    {
        fprintf(stdout, "Page: %ld\n", node->mn_lo);
    }
    else if(pg->mp_flags == P_LEAF && node->mn_flags == 0)
    {
        fprintf(stdout, "Key: %s\n", node->mn_data);
    }
    return 0;
}

int
mdb_print_page(
    MDB_page *pg
    )
{
    int error = 0;

    if(!pg)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    float freebytes = pg->mp_upper - pg->mp_lower;
    fprintf(stdout, "Page info: %d\n", pg->mp_pgno);
    fprintf(stdout, "  Page type: %s\n", page_type_str(pg->mp_flags));
    fprintf(stdout, "  No. Keys: %d\n", (pg->mp_lower - 16)/2);
    fprintf(stdout, "  Free space: %d bytes, %.2f%%\n", (int)freebytes, freebytes/(float)DEFAULT_PAGE_SIZE);

error:
    return error;
}

void
mdb_print_db(
    MDB_db *db
    )
{
    fprintf(stdout, "  Root: %ld\n", db->md_root);
    fprintf(stdout, "  Tree depth: %d\n", db->md_depth);
    fprintf(stdout, "  Branch pages: %ld\n", db->md_branch_pages);
    fprintf(stdout, "  Leaf pages: %ld\n", db->md_leaf_pages);
    fprintf(stdout, "  Overflow pages: %ld\n", db->md_overflow_pages);
    fprintf(stdout, "  Entries: %ld\n", db->md_entries);
}

int
mdb_print_stat(
    PMDB_INSPECT_CONTEXT pCtxt,
    int dbi
    )
{
    int error = 0;
    int dbiselect = MAIN_DBI;

    if(!pCtxt || !pCtxt->pDB)
    {
        error = EINVAL;
        bail_on_error(error);
    }
    if(dbi == FREE_DBI)
    {
        dbiselect = FREE_DBI;
        fprintf(stdout, "Status of Free DB:\n");
    }
    else
    {
        fprintf(stdout, "Status of Main DB:\n");
    }
    mdb_print_db(pCtxt->pDB->env.meta[dbiselect]);

error:
    return error;
}

void
free_db(
    PMDB_DB pDB
    )
{
    if(!pDB)
        return;

    if(pDB->pMapAddr && pDB->pMapAddr != MAP_FAILED)
    {
        munmap(pDB->pMapAddr, pDB->nMapLength);
    }
    free(pDB->pszDBPath);
    free(pDB);
}

void
free_context(
    PMDB_INSPECT_CONTEXT pContext
    )
{
    if(!pContext)
        return;
    free_db(pContext->pDB);
    free(pContext);
}

void
free_args(
    int argc,
    char **argv
    )
{
    if(!argc || !argv)
    {
        return;
    }

    for(int i = 0; i < argc; ++i)
    {
        free(argv[i]);
    }
    free(argv);
}

int
make_args(
    const char item,
    void *userdata
    )
{
    int error = 0;
    CMDARGS *st = (CMDARGS *)userdata;

    if(!isspace(item))
    {
        st->buff[st->buffindex] = item;
        (st->buffindex)++;
    }
    else if(st->buffindex)
    {
        st->buff[st->buffindex] = '\0';
        if(st->argv)
        {
            st->argv[st->argc] = strdup(st->buff);
        }
        st->buffindex = 0;
        st->argc++;
    }
    st->lastchar = item;

    return error;
}

int
visit_chars(
    const char *cmd,
    PFNVISITCB fnvisitcb,
    void *userdata
    )
{
    while(cmd && *cmd)
    {
        fnvisitcb(*cmd++, userdata);
    }
    fnvisitcb(' ', userdata);
}

int
split_cmd_to_argc_argv(
    const char *cmd,
    int *argc,
    char ***argv
    )
{
    int error = 0;
    int numspaces = 0;
    CMDARGS args = {0};

    error = visit_chars(cmd, make_args, &args);
    bail_on_error(error);

    if(args.argc > 0)
    {
        args.argv = (char **)malloc(sizeof(char *) * args.argc);
    }

    args.argc = 0;
    error = visit_chars(cmd, make_args, &args);
    bail_on_error(error);

    *argc = args.argc;
    *argv = args.argv;
cleanup:
    return error;

error:
    free_args(args.argc, args.argv);
    goto cleanup;
}

void
free_value(
    PVALUE value
    )
{
    if(!value)
        return;
    if(value->type == VAL_TYPE_STR)
    {
        free(value->strval);
    }
    free(value);
}

void
free_keyvalue(
    PKEYVALUE keyvalue
    )
{
    if(!keyvalue)
        return;
    free(keyvalue->value);
    free(keyvalue);
}

void
free_keyvalues(
    PKEYVALUE keyvalue
    )
{
    if(!keyvalue)
        return;
    while(keyvalue->next)
    {
        PKEYVALUE temp = keyvalue->next;
        free_keyvalue(keyvalue);
        keyvalue = temp;
    }
}

void
free_inspect_cmd(
    PMDB_INSPECT_CMD pCmd
    )
{
    if(!pCmd)
        return;

    free_args(pCmd->extra_argc, pCmd->extra_argv);
    free_keyvalues(pCmd->options);
    free(pCmd);
}

int
mdb_print_subdb(
    MDB_node *node
    )
{
    int error = 0;

    if(!node)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    fprintf(stdout, "%s\n", node->mn_data);

error:
    return error;
}

int
mdb_print_subdb_v(
    MDB_node *node,
    MDB_db *db
    )
{
    int error = 0;

    if(!node || !db)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    fprintf(stdout, "Subdb: %s\n", node->mn_data);
    mdb_print_db(db);

error:
    return error;
}
