/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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

#include "defines.h"

// -s sub 3646
// -s one 1207
#define TEST_BIG_CONTAINER_DN       VMDIR_TEST_SEARCH_BASE_RDN

// -s sub 30
// -s one 29
#define TEST_SMALL_CONTAINER_DN     VMDIR_TEST_SEARCH_CONTAINER_3_SMALL_RDN","VMDIR_TEST_SEARCH_BASE_RDN


#define TEST_SEARCH_CASE_1                          \
{                                                   \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "small sub tree present filter: DN candidate set", \
            /*.pszBaseDN    = */ TEST_SMALL_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 30, 0, 0, 30,                    \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "small one level tree present filter: parentid candidate set", \
            /*.pszBaseDN    = */ TEST_SMALL_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 29, 0, 0, 29,                    \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree present filter: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3646, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level tree present filter: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 1207, 0 , 1207, 0,       \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged small sub tree present filter: DN candidate set", \
            /*.pszBaseDN    = */ TEST_SMALL_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 30, 0, 0, 30,                    \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged small one level tree present filter: parentid candidate set", \
            /*.pszBaseDN    = */ TEST_SMALL_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 29, 0, 0, 29,                    \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big sub tree present filter: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3646, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big one level tree present filter: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 1207, 0, 1207, 0,        \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree good attr EQ filter: candidate set", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringUnique=stringignore1024",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 1, 0, 0, 1,                      \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big sub tree good attr EQ filter: candidate set", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringUnique=stringignore1024",    \
            /*.dwPageSize   = */ 10,                \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            0, "", 1, 0, 0, 1,                      \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree bad attr EQ filter: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big sub tree bad attr EQ filter: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {   /* vmwTestSearchCaseIgnoreStringNonunique+EQ has higher score than parentid */  \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 597, 0, 1821, 0,        \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big one level bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 597, 0, 1821, 0,        \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree bad attr SUB filter duplicate EID: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big sub tree bad attr SUB filter duplicate EID: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level bad attr SUB filter duplicate EID: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 597, 0, 1207, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big one level bad attr SUB filter duplicate EID: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 597, 0, 1207, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree OR filter: DN iterator, disable iterator inside OR", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(|(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*)(description=*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3642, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree OR filter: DN iterator, non-indexed attr inside OR", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(|(description=*)(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3642, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree AND(OR()()) sub filter: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(&(|(description=*)(cn=*))(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
}

#define TEST_SEARCH_CASE_2                          \
{                                                   \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree present filter size limit 5: DN iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 5,                 \
        },                                          \
        {                                           \
            1, "entryDN", 5, 0, 5, 0,               \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level tree present filter size limit 5: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 5,                 \
        },                                          \
        {                                           \
            1, "parentid", 5, 0, 5, 0,              \
        },                                          \
    },                                              \
}

#define TEST_SEARCH_CASE_NORMAL_USER_ITERATION_LIMIT \
{                                                   \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level tree exceed iteration limit: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 0, 1, 0, 0,              \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big one level tree exceed iteration limit: parentid iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,        \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "parentid", 0, 1, 0, 0,              \
        },                                          \
    },                                              \
}

/*
 * Test cases with customized priority map - vmwTestSearchCaseIgnoreStringNonunique:20
 * This will make vmwTestSearchCaseIgnoreStringNonunique has higher score than entryDN/parentid
 *   in iterator evaluation
 */
#define TEST_SEARCH_CASE_CUSTOMIZE_MAP              \
{                                                   \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big sub tree bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big one level bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 597, 0, 1821, 0,        \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "paged big one level bad attr EQ filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 1,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0",    \
            /*.dwPageSize   = */ 100,               \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 597, 0, 1821, 0,        \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree bad attr SUB filter duplicate EID: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree OR filter: DN iterator, disable vmwTestSearchCaseIgnoreStringNonunique inside OR", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(|(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*)(description=*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3642, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree OR filter: DN iterator, non-indexed attr inside OR", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(|(description=*)(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3642, 0, 3646, 0,         \
        },                                          \
    },                                              \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "big sub tree AND(OR()()) sub filter: vmwTestSearchCaseIgnoreStringNonunique iterator", \
            /*.pszBaseDN    = */ TEST_BIG_CONTAINER_DN,  \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "(&(|(description=*)(cn=*))(vmwTestSearchCaseIgnoreStringNonunique=StringIgnore0*))",    \
            /*.dwPageSize   = */ 0,                 \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "vmwTestSearchCaseIgnoreStringNonunique", 1821, 0, 3646, 0,         \
        },                                          \
    },                                              \
}

#define TEST_SEARCH_CASE_ABNORMAL                   \
{                                                   \
    {                                               \
        {                                           \
            /*.pszDesc      = */ "Long running page (pause between page) and invalid cookie: DN iterator", \
            /*.pszBaseDN    = */ VMDIR_TEST_SEARCH_BASE_RDN,        \
            /*.iScope       = */ 2,                 \
            /*.pszFilter    = */ "cn=*",            \
            /*.dwPageSize   = */ 1500,              \
            /*.iSizeLimit   = */ 0,                 \
        },                                          \
        {                                           \
            1, "entryDN", 3646, 0, 3646, 0,         \
        },                                          \
    },                                              \
}
