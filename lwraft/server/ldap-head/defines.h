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
 * Module Name: Directory ldap-head
 *
 * Filename: defines.h
 *
 * Abstract:
 *
 * Definitions
 *
 */

#define MAX_NUM_OF_SOCK_READ_RETRIES 20

#ifdef _WIN32
#define MAX_NUM_OF_BIND_PORT_RETRIES 30

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#endif

// Special char used in ldap search request
// such as all user attrs, all operational attrs, userpassword(vmdir specific) ...etc
#define LDAP_SEARCH_REQUEST_CHAR_NONE   0x00000000
#define LDAP_SEARCH_REQUEST_CHAR_USER   0x00000001
#define LDAP_SEARCH_REQUEST_CHAR_OP     0x00000002
#define LDAP_SEARCH_REQUEST_CHAR_PASSWD   0x00000004

/*
 * A bit map that enables referral to Raft leader for LDAP search or Updates
 * ERROR_CODE is set to LDAP_REFERRAL is that bit is set otherwise is set to 0
 *  -- Some LDAP client libraries may expect error code 0 with referral URL.
 */
#define VMDIR_RAFT_ENABLE_SEARCH_REFERRAL 0x1
#define VMDIR_RAFT_SEARCH_REFERRAL_ERROR_CODE 0x02
#define VMDIR_RAFT_ENABLE_UPDATE_REFERRAL 0x4
#define VMDIR_RAFT_ENABLE_UPDATE_ERROR_CODE 0x8

typedef int (*NEW_CONNECTION_FUNC)(
                ber_socket_t      sfd,
                VDIR_CONNECTION   **conn,
                Sockbuf_IO        *pSockbuf_IO);

typedef enum
{
    METRICS_LDAP_OP_BIND,
    METRICS_LDAP_OP_SEARCH,
    METRICS_LDAP_OP_ADD,
    METRICS_LDAP_OP_MODIFY,
    METRICS_LDAP_OP_DELETE,
    METRICS_LDAP_OP_UNBIND,
    METRICS_LDAP_OP_COUNT

} METRICS_LDAP_OPS;

typedef enum
{
    METRICS_LDAP_SUCCESS,
    METRICS_LDAP_UNAVAILABLE,
    METRICS_LDAP_SERVER_DOWN,
    METRICS_LDAP_UNWILLING_TO_PERFORM,
    METRICS_LDAP_INVALID_DN_SYNTAX,
    METRICS_LDAP_NO_SUCH_ATTRIBUTE,
    METRICS_LDAP_INVALID_SYNTAX,
    METRICS_LDAP_UNDEFINED_TYPE,
    METRICS_LDAP_TYPE_OR_VALUE_EXISTS,
    METRICS_LDAP_OBJECT_CLASS_VIOLATION,
    METRICS_LDAP_ALREADY_EXISTS,
    METRICS_LDAP_CONSTRAINT_VIOLATION,
    METRICS_LDAP_NOT_ALLOWED_ON_NONLEAF,
    METRICS_LDAP_PROTOCOL_ERROR,
    METRICS_LDAP_INVALID_CREDENTIALS,
    METRICS_LDAP_INSUFFICIENT_ACCESS,
    METRICS_LDAP_AUTH_METHOD_NOT_SUPPORTED,
    METRICS_LDAP_SASL_BIND_IN_PROGRESS,
    METRICS_LDAP_TIMELIMIT_EXCEEDED,
    METRICS_LDAP_SIZELIMIT_EXCEEDED,
    METRICS_LDAP_NO_SUCH_OBJECT,
    METRICS_LDAP_UNKNOWN,
    METRICS_LDAP_ERROR_COUNT

} METRICS_LDAP_ERRORS;
