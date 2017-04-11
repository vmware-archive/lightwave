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
 * Module Name: Directory ACL
 *
 * Filename: structs.h
 *
 * Abstract:
 *
 * Directory ACL module
 *
 * Private Structures
 *
 */

typedef struct _VDIR_ACL_CTX
{
    PSECURITY_DESCRIPTOR_RELATIVE pSecurityDescriptor;
    ULONG ulSecDescLength;
} VDIR_ACL_CTX;

typedef struct _VMDIR_SID_GEN_STACK_NODE
{
    DWORD dwDomainRidSequence;
    PSTR pszDomainDn;
} VMDIR_SID_GEN_STACK_NODE, *PVMDIR_SID_GEN_STACK_NODE;

typedef struct _VDIR_DOMAIN_SID_GEN_STATE
{
    PSTR                    pszDomainDn; // in order to search with a given DN
    PSTR                    pszDomainSid; // comparably to domainSid
    DWORD                   dwDomainRidSeqence;
    DWORD                   dwCount;

    LW_HASHTABLE_NODE       Node;

} VDIR_DOMAIN_SID_GEN_STATE, *PVDIR_DOMAIN_SID_GEN_STATE;

typedef struct _VDIR_SID_GEN_STATE
{
    // NOTE: order of fields MUST stay in sync with struct initializer...
    PVMDIR_MUTEX        mutex;
    PLW_HASHTABLE       pHashtable;
    PVDIR_THREAD_INFO pRIDSyncThr;
    PVMDIR_TSSTACK      pStack;
} VDIR_SID_GEN_STATE, *PVDIR_SID_GEN_STATE;
