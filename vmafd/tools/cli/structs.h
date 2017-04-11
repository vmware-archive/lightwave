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
    VM_AFD_ACTION_UNKNOWN = 0,
    VM_AFD_ACTION_GET_STATUS,
    VM_AFD_ACTION_GET_DOMAIN_NAME,
    VM_AFD_ACTION_SET_DOMAIN_NAME,
    VM_AFD_ACTION_GET_DOMAIN_STATE,
    VM_AFD_ACTION_GET_LDU,
    VM_AFD_ACTION_SET_LDU,
    VM_AFD_ACTION_GET_RHTTPPROXY_PORT,
    VM_AFD_ACTION_SET_RHTTPPROXY_PORT,
    VM_AFD_ACTION_SET_DC_PORT,
    VM_AFD_ACTION_GET_CM_LOCATION,
    VM_AFD_ACTION_GET_LS_LOCATION,
    VM_AFD_ACTION_GET_PNID_URL,
    VM_AFD_ACTION_GET_PNID,
    VM_AFD_ACTION_SET_PNID,
    VM_AFD_ACTION_GET_SITE_GUID,
    VM_AFD_ACTION_GET_SITE_NAME,
    VM_AFD_ACTION_PROMOTE_VM_DIR,
    VM_AFD_ACTION_DEMOTE_VM_DIR,
    VM_AFD_ACTION_JOIN_VM_DIR,
    VM_AFD_ACTION_LEAVE_VM_DIR,
    VM_AFD_ACTION_JOIN_AD,
    VM_AFD_ACTION_LEAVE_AD,
    VM_AFD_ACTION_QUERY_AD,
    VM_AFD_ACTION_GET_DC_NAME,
    VM_AFD_ACTION_SET_DC_NAME,
    VM_AFD_ACTION_ADD_PASSWORD_ENTRY,
    VM_AFD_ACTION_GET_MACHINE_ACCOUNT_INFO,
    VM_AFD_ACTION_SET_MACHINE_ACCOUNT_INFO,
    VM_AFD_ACTION_GET_MACHINE_SSL_CERTIFICATES,
    VM_AFD_ACTION_SET_MACHINE_SSL_CERTIFICATES,
    VM_AFD_ACTION_GET_MACHINE_ID,
    VM_AFD_ACTION_SET_MACHINE_ID,
    VM_AFD_ACTION_GET_HEARTBEAT_STATUS,
    VM_AFD_ACTION_REFRESH_SITE_NAME,
    VM_AFD_ACTION_GET_DC_LIST,
    VM_AFD_ACTION_CHANGE_PNID
} VM_AFD_ACTION, *PVM_AFD_ACTION;

typedef struct _VM_AFD_CLI_CONTEXT
{
    VM_AFD_ACTION      action;

    PSTR               pszServerName;
    PSTR               pszDomainName;
    PSTR               pszDCName;
    PSTR               pszOrgUnit;
    PSTR               pszUserName;
    PSTR               pszPassword;
    PSTR               pszSiteName;
    PSTR               pszPartnerName;
    PSTR               pszMachineName;
    PSTR               pszMachineId;
    PSTR               pszLDU;
    PSTR               pszCMLocation;
    PSTR               pszLSLocation;
    PSTR               pszPNID;
    PSTR               pszSiteGUID;
    DWORD              dwPort;
    VMAFD_DOMAIN_STATE domainState;
    VMAFD_STATUS       status;
    DWORD              dwLeaveFlags;
} VM_AFD_CLI_CONTEXT, *PVM_AFD_CLI_CONTEXT;
