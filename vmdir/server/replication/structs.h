/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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


/*
 * Module Name: Directory replication
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 *
 * Private Structures
 *
 */

typedef struct _VMDIR_REPLICATION_PAGE_ENTRY
{
    LDAPMessage*    entry;// not valid during retry
    int             entryState;
    PSTR            pszPartner;
    USN             ulPartnerUSN;
    PSTR            pszDn;
    DWORD           dwDnLength;
    int             errVal;
    PVDIR_BERVALUE  pBervEncodedEntry; // valid only during retry
    VDIR_BERVALUE   reqDn;
} VMDIR_REPLICATION_PAGE_ENTRY, *PVMDIR_REPLICATION_PAGE_ENTRY;

typedef struct _VMDIR_REPLICATION_PAGE
{
    PSTR pszFilter;
    LDAPControl syncReqCtrl;
    LDAPMessage *searchRes;
    LDAPControl **searchResCtrls;
    VMDIR_REPLICATION_PAGE_ENTRY *pEntries;
    int iEntriesRequested;
    int iEntriesReceived;
    USN lastSupplierUsnProcessed;
    int iEntriesProcessed;
    int iEntriesOutOfSequence;
} VMDIR_REPLICATION_PAGE, *PVMDIR_REPLICATION_PAGE;

typedef struct _VMDIR_REPLICATON_CONNECTION
{
    LDAP*   pLd;
    PSTR    pszPartnerHostName;
} VMDIR_REPLICATION_CONNECTION, *PVMDIR_REPLICATION_CONNECTION;

typedef struct _VMDIR_REPLICATION_CONTEXT
{
    PVDIR_SCHEMA_CTX    pSchemaCtx;
    time_t              stLastTimeTriedToFillHoleInDirectory;
    PSTR                pszKrb5ErrorMsg;
    PDEQUE              pFailedEntriesQueue;
} VMDIR_REPLICATION_CONTEXT, *PVMDIR_REPLICATION_CONTEXT;

typedef struct _VMDIR_REPLICATION_METRICS_CACHE
{
    PLW_HASHMAP     pHashMap;
    PVMDIR_RWLOCK   pLock;

} VMDIR_REPLICATION_METRICS_CACHE, PVMDIR_REPLICATION_METRICS_CACHE;

typedef struct _VMDIR_DR_DC_INFO
{
    PSTR pszUPN;
    PSTR pszSite;
} VMDIR_DR_DC_INFO, *PVMDIR_DR_DC_INFO;

typedef struct _VMDIR_SWAP_DB_INFO
{
    PSTR    pszOrgDBServerName;     // remote db was taken from this server object name
    PSTR    pszOrgDBUTDVector;
    PSTR    pszOrgDBMaxUSN;
    PSTR    pszPartnerServerName;   // optional partner server object name
    PSTR    pszMyUTDVcetor;
    PSTR    pszMyHighWaterMark;
} VMDIR_SWAP_DB_INFO, *PVMDIR_SWAP_DB_INFO;

typedef struct _VMDIR_JOIN_FLOW_INFO
{
    BOOLEAN bJoinWithPreCopiedDB;
    DWORD   dwPreSetMaxServerId;
} VMDIR_JOIN_FLOW_INFO, *PVMDIR_JOIN_FLOW_INFO;
