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
mdb_open(
    const char *pCmd,
    PMDB_INSPECT_CONTEXT pContext
    );

int
mdb_read_env(
    PMDB_DB pDB
    );

int
mdb_page_info(
    PMDB_DB pDB,
    pgno_t pgno,
    MDB_page **ppg
    );

typedef int (*PFNPAGECB)(MDB_page *);

int
mdb_page_get(
    PMDB_DB pDB,
    pgno_t pgno_start,
    pgno_t pgno_end,
    PFNPAGECB pgcb
    );

typedef int (*PFNPAGEVCB)(MDB_page *, MDB_node *, int);
int
mdb_page_get_v(
    PMDB_DB pDB,
    pgno_t pgno_start,
    pgno_t pgno_end,
    PFNPAGECB pgcb,
    PFNPAGEVCB pgvcb
    );

typedef int (*PFNSUBDBCB)(MDB_node *);
int
mdb_subdb_get(
    PMDB_DB pDB,
    const char *subdb,
    PFNSUBDBCB subdb_cb
    );

typedef int (*PFNSUBDBVCB)(MDB_node *, MDB_db *);
int
mdb_subdb_get_v(
    PMDB_DB pDB,
    const char *subdb,
    PFNSUBDBVCB subdb_cb
    );

int
mdb_list_subs(
    PMDB_DB pDB
    );
