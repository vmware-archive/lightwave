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

#define VMDIR_TEST_SEARCH_BASE              "testSearch"
#define VMDIR_TEST_SEARCH_CONTAINER_1_BIG   "testSearchC1Big"
#define VMDIR_TEST_SEARCH_CONTAINER_2_BIG   "testSearchC2Big"
#define VMDIR_TEST_SEARCH_CONTAINER_3_SMALL "testSearchC3Small"

#define VMDIR_TEST_SEARCH_BASE_RDN                  "cn="VMDIR_TEST_SEARCH_BASE
#define VMDIR_TEST_SEARCH_CONTAINER_1_BIG_RDN       "cn="VMDIR_TEST_SEARCH_CONTAINER_1_BIG
#define VMDIR_TEST_SEARCH_CONTAINER_2_BIG_RDN       "cn="VMDIR_TEST_SEARCH_CONTAINER_2_BIG
#define VMDIR_TEST_SEARCH_CONTAINER_3_SMALL_RDN     "cn="VMDIR_TEST_SEARCH_CONTAINER_3_SMALL


#define ATTR_STR_IGNORE_NONUNIQUE        "vmwTestSearchCaseIgnoreStringNonunique"
#define ATTR_STR_IGNORE_UNIQUE           "vmwTestSearchCaseIgnoreStringUnique"
#define ATTR_STR_EXACT_NONUNIQUE         "vmwTestSearchCaseExactStringNonunique"
#define ATTR_STR_EXACT_UNIQUE            "vmwTestSearchCaseExactStringUnique"
#define ATTR_INTEGER_NONUNIQUE           "vmwTestSearchIntegerNonunique"
#define ATTR_INTEGER_UNIQUE              "vmwTestSearchIntegerUnique"

#define OC_TEST_SEARCH                   "vmwSearchTest"
#define VMDIR_TEST_SEARCH_OBJECT_CN      "TestSearchCN-"

#define VMDIR_STR_IGNORE_LOWER           "stringignore"
#define VMDIR_STR_IGNORE                 "StringIgnore"
#define VMDIR_STR_IGNORE_LEN             sizeof(VMDIR_STR_IGNORE)-1
#define VMDIR_STR_EXACT                  "StringExact"
#define VMDIR_STR_EXACT_LEN              sizeof(VMDIR_STR_EXACT)-1

#define MAX_SEARCH_OBJECT               (1204+10) /* per container */ * 3 /* 3 containers */

#define INTEGER_NONUNIQUE_MOD           2

#define SCOPE_NUM_TO_STR(iScope) (iScope==0 ? VMDIR_STR_SCOPE_BASE : (iScope==1 ? VMDIR_STR_SCOPE_ONE : VMDIR_STR_SCOPE_SUB))

#define VMDIR_TEST_SEARCH_ERROR_BASE            10000

#define VMDIR_TEST_SEARCH_ERROR_RESULT_CODE     (VMDIR_TEST_SEARCH_ERROR_BASE + 1)
#define VMDIR_TEST_SEARCH_ERROR_ALGORITHM       (VMDIR_TEST_SEARCH_ERROR_BASE + 2)
#define VMDIR_TEST_SEARCH_ERROR_TOTAL_ENTRY     (VMDIR_TEST_SEARCH_ERROR_BASE + 3)
#define VMDIR_TEST_SEARCH_ERROR_INDEX_TABLE     (VMDIR_TEST_SEARCH_ERROR_BASE + 4)
#define VMDIR_TEST_SEARCH_ERROR_SCAN_LIMIT      (VMDIR_TEST_SEARCH_ERROR_BASE + 5)

