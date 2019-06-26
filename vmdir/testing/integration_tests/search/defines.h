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
#define VMDIR_TEST_SEARCH_CONTAINER_1       "testSearchC1"
#define VMDIR_TEST_SEARCH_CONTAINER_2       "testSearchC2"
#define VMDIR_TEST_SEARCH_BASE_RDN          "cn="VMDIR_TEST_SEARCH_BASE
#define VMDIR_TEST_SEARCH_CONTAINER_1_RDN   "cn="VMDIR_TEST_SEARCH_CONTAINER_1
#define VMDIR_TEST_SEARCH_CONTAINER_2_RDN   "cn="VMDIR_TEST_SEARCH_CONTAINER_2

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

#define INTERGER_NONUNIQUE_MOD          VMDIR_SIZE_128
