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


typedef enum
{
    VM_DNS_ACTION_UNKNOWN = 0,
    VM_DNS_ACTION_ADD_ZONE,
    VM_DNS_ACTION_DEL_ZONE,
    VM_DNS_ACTION_LIST_ZONES,
    VM_DNS_ACTION_ADD_RECORD,
    VM_DNS_ACTION_DEL_RECORD,
    VM_DNS_ACTION_LIST_RECORDS,
    VM_DNS_ACTION_QUERY_RECORD,
    VM_DNS_ACTION_ADD_FORWARDER,
    VM_DNS_ACTION_DEL_FORWARDER,
    VM_DNS_ACTION_LIST_FORWARDERS

} VM_DNS_ACTION;

typedef struct _VM_DNS_CLI_CONTEXT
{
    PVMDNS_SERVER_CONTEXT pServerContext;

    VM_DNS_ACTION   action;

    PSTR            pszZone;
    PSTR            pszNSHost;
    PSTR            pszNSIp;
    PSTR            pszMboxDomain;
    DWORD           dwZoneType;

    VMDNS_RECORD    record;
    PSTR            pszService;
    PSTR            pszProtocol;

    PSTR            pszForwarder;
    DWORD           dwNumForwarders;

    PSTR            pszServer;
} VM_DNS_CLI_CONTEXT, *PVM_DNS_CLI_CONTEXT;
