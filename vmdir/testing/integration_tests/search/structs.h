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

typedef struct _VMDIR_SEARCH_OP_CONTEXT
{
    BOOLEAN             bDone;
    BOOLEAN             bPaged;
    DWORD               dwPageSize;
    struct berval*      pbvCookie;
    DWORD               dwResultCode;
    DWORD               dwResultCount;
    DWORD               dwTotalResultCount;
    PSTR                pszRestPagedCookie;
} VMDIR_SEARCH_OP_CONTEXT, *PVMDIR_SEARCH_OP_CONTEXT;

typedef struct _VMDIR_SEARCH_TEST_CONTEXT
{
    PVMDIR_TEST_STATE   pTestState;
    TEST_LDAP_CONNECTION_OWNER  testLdapOwner;

    PSTR                pszSearchDN;
    PSTR                pszSearchC1DN;
    PSTR                pszSearchC2DN;
    PSTR                pszSearchC3DN;
} VMDIR_SEARCH_TEST_CONTEXT, *PVMDIR_SEARCH_TEST_CONTEXT;

typedef struct _VMDIR_SEARCH_TEST_RECORD
{
    DWORD   dwIndex;
    PSTR    pszDN;
    PSTR    pszCN;
    PSTR    pszStrIgnoreNonunique;
    PSTR    pszStrIgnoreUnique;
    PSTR    pszStrExactNonunique;
    PSTR    pszStrExactUnique;
    PSTR    pszIntegerNonunique;
    PSTR    pszIntegerUnique;
} VMDIR_SEARCH_TEST_RECORD, *PVMDIR_SEARCH_TEST_RECORD;

typedef struct _VMDIR_SEARCH_TEST_RESULT
{
    VDIR_SRV_SEARCH_ALGO algo;
    PSTR                 pszIndex;
    ber_int_t            iNumEntryReceived;
    ber_int_t            bExceedMaxIteation;
    ber_int_t            iNumIteration;         // not validated
    ber_int_t            iNumCandiateSize;      // not validated
} VMDIR_SEARCH_TEST_RESULT, *PVMDIR_SEARCH_TEST_RESULT;

typedef struct _VMDIR_SEARCH_TEST_DEFINITION
{
    PSTR        pszDesc;
    PSTR        pszBaseDN;
    int         iScope;
    PSTR        pszFilter;
    DWORD       dwPageSize;    // > 0 for paged search
    int         iSizeLimit;
} VMDIR_SEARCH_TEST_DEFINITION, *PVMDIR_SEARCH_TEST_DEFINITION;

typedef struct _VMDIR_SEARCH_TEST_CASE
{
    VMDIR_SEARCH_TEST_DEFINITION    definition;
    VMDIR_SEARCH_TEST_RESULT        result;
} VMDIR_SEARCH_TEST_CASE, *PVMDIR_SEARCH_TEST_CASE;
