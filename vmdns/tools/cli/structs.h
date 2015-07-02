/*
 * Copyright (c) VMware Inc.  All rights Reserved.
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

    VMDNS_RECORD    record;
    PSTR            pszService;
    PSTR            pszProtocol;

    PSTR            pszForwarder;
    DWORD           dwNumForwarders;

} VM_DNS_CLI_CONTEXT, *PVM_DNS_CLI_CONTEXT;
