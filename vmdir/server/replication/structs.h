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
    LDAPMessage *entry;
    int         entryState;
    USN         ulPartnerUSN;
    PSTR        pszDn;
    DWORD       dwDnLength;
    int         errVal;
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

typedef struct _VMDIR_REPLICATION_PASSWORD
{
    PCSTR pszPassword;
    time_t *pPasswordFailTime;
} VMDIR_REPLICATION_PASSWORD, *PVMDIR_REPLICATION_PASSWORD;

typedef struct _VMDIR_REPLICATION_CREDENTIALS
{
    PSTR pszUPN;
    PSTR pszDN;
    PSTR pszPassword;
    PSTR pszOldPassword;
    BOOLEAN bChanged;
} VMDIR_REPLICATION_CREDENTIALS, *PVMDIR_REPLICATION_CREDENTIALS;

typedef struct _VMDIR_REPLICATON_CONNECTION
{
    LDAP *pLd;
    PSTR pszConnectionDescription;
} VMDIR_REPLICATION_CONNECTION, *PVMDIR_REPLICATION_CONNECTION;

typedef struct _VMDIR_REPLICATION_CONTEXT
{
    PVDIR_SCHEMA_CTX pSchemaCtx;
    BOOLEAN bFirstReplicationCycle;
    time_t stLastTimeTriedToFillHoleInDirectory;
    PSTR pszKrb5ErrorMsg;
} VMDIR_REPLICATION_CONTEXT, *PVMDIR_REPLICATION_CONTEXT;
