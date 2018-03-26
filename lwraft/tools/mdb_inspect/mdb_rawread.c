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

/*
    checks if a page is valid by checking
    1. if page flags set as meta
    2. magic bytes read 0xBEEFC0DE
    3. version is set to 1
*/
int
is_valid_meta_page(
    MDB_metabuf *metabuf
    )
{
    int error = 0;
    if(!metabuf)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    if(metabuf->mb_page.mp_flags != P_META ||
       metabuf->mb_metabuf.mm_meta.mm_magic != MDB_MAGIC ||
       metabuf->mb_metabuf.mm_meta.mm_version != MDB_DATA_VERSION)
    {
        error = EINVAL;
        bail_on_error(error);
    }

error:
    return error;
}

/*
    check if this is a valid mdb
    read first 2 pages, check if valid
*/
int
mdb_is_valid(
    PMDB_DB pDB
    )
{
    int error = 0;
    if(!pDB || !pDB->pMapAddr)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    error = is_valid_meta_page((MDB_metabuf *)pDB->pMapAddr);
    bail_on_error(error);

    error = is_valid_meta_page((MDB_metabuf *)(pDB->pMapAddr + DEFAULT_PAGE_SIZE));
    bail_on_error(error);

cleanup:
    return error;

error:
    goto cleanup;
}

/*
    cheap mdb environment
    populate references to free and main db
*/
int
mdb_read_env(
    MDB_DB *pDB
    )
{
    int error = 0;
    MDB_metabuf *metabuf = NULL;
    if(!pDB || !pDB->pMapAddr)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    metabuf = (MDB_metabuf *)pDB->pMapAddr;
    pDB->env.meta[FREE_DBI] = &metabuf->mb_metabuf.mm_meta.mm_dbs[MAIN_DBI];

    metabuf = (MDB_metabuf *)(pDB->pMapAddr + DEFAULT_PAGE_SIZE);
    pDB->env.meta[MAIN_DBI] = &metabuf->mb_metabuf.mm_meta.mm_dbs[MAIN_DBI];

    pDB->env.root = pDB->env.meta[FREE_DBI]->md_root;
    if(pDB->env.root < pDB->env.meta[MAIN_DBI]->md_root)
    {
        pDB->env.root = pDB->env.meta[MAIN_DBI]->md_root;
    }

    pDB->env.pagecount = pDB->nMapLength / DEFAULT_PAGE_SIZE;
error:
    return error;
}

int
mdb_page_get(
    PMDB_DB pDB,
    pgno_t pgno_start,
    pgno_t pgno_end,
    PFNPAGECB pgcb
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;

    if(!pDB || pgno_start < 0 || pgno_end >= pDB->env.pagecount || !pgcb)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    while(pgno_start <= pgno_end)
    {
        pg = (MDB_page *)(pDB->pMapAddr + pgno_start * DEFAULT_PAGE_SIZE);
        pgcb(pg);
        ++pgno_start;
    }

error:
    return error;
}

int
_iterate_branch(
    MDB_page *pg,
    PFNPAGEVCB pgvcb
    )
{
    int i = 0;
    int numkeys = (pg->mp_lower - 16)/2;
    char *pagestart = (char *)pg;
    for(i = 0; i < numkeys; ++i)
    {
        MDB_node *node = (MDB_node *)(pagestart + pg->mp_ptrs[i]);
        pgvcb(pg, node, i);
    }
    return 0;
}

int
mdb_page_get_v(
    PMDB_DB pDB,
    pgno_t pgno_start,
    pgno_t pgno_end,
    PFNPAGECB pgcb,
    PFNPAGEVCB pgvcb
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;

    if(!pDB || pgno_start < 0 || pgno_end >= pDB->env.pagecount || !pgcb || !pgvcb)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    while(pgno_start <= pgno_end)
    {
        pg = (MDB_page *)(pDB->pMapAddr + pgno_start * DEFAULT_PAGE_SIZE);
        pgcb(pg);
        _iterate_branch(pg, pgvcb);
        ++pgno_start;
    }

error:
    return error;
}

int
mdb_page_info(
    PMDB_DB pDB,
    pgno_t pgno,
    MDB_page **ppg
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;

    if(!pDB || !ppg || pgno < 0 || pgno >= pDB->env.pagecount)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pg = (MDB_page *)(pDB->pMapAddr + pgno * DEFAULT_PAGE_SIZE);

    *ppg = pg;

error:
    return error;
}

/*
    if we start at any root, we should be able to see a list of sub databases
    sub databases are keys with MDB_db structs as their data.
*/
int
mdb_list_subs(
    PMDB_DB pDB
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;
    if(!pDB)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pg = (MDB_page *)(pDB->pMapAddr + pDB->env.root * DEFAULT_PAGE_SIZE);

    /* it is easy to see how many keys are there in a page */


    /* there might be a case where this is a branch
       if you have enough subdata nodes
    */
    if(pg->mp_flags == P_LEAF)
    {
    }

error:
    return error;
}

/*
    memory map database file and add to context.
*/
int
mdb_open(
    const char *pszDBPath,
    PMDB_INSPECT_CONTEXT pContext
    )
{
    int error = 0;
    PMDB_DB pDB = NULL;
    struct stat sb;

    if(!pszDBPath || !pContext)
    {
        error = EINVAL;
        bail_on_error(error);
    }
    pDB = (PMDB_DB)calloc(1, sizeof(MDB_DB));
    if(!pDB)
    {
        error = ENOMEM;
        bail_on_error(error);
    }

    pDB->fd = open(pszDBPath, O_RDONLY);
    if(pDB->fd == -1)
    {
        error = errno;
        bail_on_error(error);
    }

    if (fstat(pDB->fd, &sb) == -1)
    {
        error = errno;
        bail_on_error(error);
    }

    pDB->pMapAddr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, pDB->fd, 0);
    if(pDB->pMapAddr == MAP_FAILED)
    {
        error = errno;
        bail_on_error(error);
    }
    pDB->pszDBPath = strdup(pszDBPath);
    pDB->nMapLength = sb.st_size;

    error = mdb_is_valid(pDB);
    bail_on_error(error);

    error = mdb_read_env(pDB);
    bail_on_error(error);

    pContext->pDB = pDB;

cleanup:
    return error;
error:
    if(pDB)
    {
        free_db(pDB);
    }
    goto cleanup;
}

int
mdb_subdb_get(
    PMDB_DB pDB,
    const char *subdb,
    PFNSUBDBCB subdb_cb
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;
    int numkeys = 0;
    int i = 0;
    char *nodestart = NULL;
    char *pagestart = NULL;

    if(!pDB || !subdb_cb)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pagestart = pDB->pMapAddr + pDB->env.root * DEFAULT_PAGE_SIZE;
    pg = (MDB_page *)pagestart;
    numkeys = (pg->mp_lower - 16)/2;

    for(i = 0; i < numkeys; ++i)
    {
        MDB_node *node = (MDB_node *)(pagestart + pg->mp_ptrs[i]);
        subdb_cb(node);
    }

error:
    return error;
}

int
mdb_subdb_get_v(
    PMDB_DB pDB,
    const char *subdb,
    PFNSUBDBVCB subdb_cb
    )
{
    int error = 0;
    int page_header_size = offsetof(MDB_page, mp_ptrs);/* 16 for version 1*/
    MDB_page *pg = NULL;
    int numkeys = 0;
    int i = 0;
    char *nodestart = NULL;
    char *pagestart = NULL;

    if(!pDB || !subdb_cb)
    {
        error = EINVAL;
        bail_on_error(error);
    }

    pagestart = pDB->pMapAddr + pDB->env.root * DEFAULT_PAGE_SIZE;
    pg = (MDB_page *)pagestart;
    numkeys = (pg->mp_lower - 16)/2;

    for(i = 0; i < numkeys; ++i)
    {
        MDB_node *node = (MDB_node *)(pagestart + pg->mp_ptrs[i]);
        MDB_db *db = (MDB_db *)((char *)node->mn_data + node->mn_ksize);
        subdb_cb(node, db);
    }

error:
    return error;
}
