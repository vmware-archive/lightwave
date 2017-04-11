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



#pragma once

#ifndef _WIN32
#define VmAfd_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VmAfd_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#ifndef VMDDNS_COMPRESSED_NAME
#define VMDDNS_COMPRESSED_NAME(Data)     \
    (((Data) & (1 << 0x07))) &&          \
    (((Data) & (1 << 0x06)))
#endif /* VMAFD_DNS_COMPRESSED_NAME */

#define VMAFD_IPC_PACKET_SIZE 64*1024
#define VMAFD_FQDN_SEPARATOR '.'
#define VMAFD_IPC_SIZE_T UINT64

#define VMDNS_DEFAULT_REFRESH_INTERVAL  3600
#define VMDNS_DEFAULT_RETRY_INTERVAL    600
#define VMDNS_DEFAULT_EXPIRE            86400
#define VMDNS_DEFAULT_TTL               3600
#define VMDNS_DEFAULT_LDAP_PORT         389

#ifndef _WIN32
#define SOCKET_FILE_PATH "/var/run/vmafd_socket"
#define EVERYONE_UID -1
#define MAX_GWTPWR_BUF_LENGTH 16384
#endif
#if defined _WIN32
#define NAME_OF_PIPE "\\\\.\\pipe\\vmafd_pipe"
#define PIPE_TIMEOUT_INTERVAL 5000
#define PIPE_CLIENT_RETRY_COUNT 3
#endif

#define VMDDNS_LABEL_LENGTH_MAX 63
#define VMDDNS_NAME_LENGTH_MAX  255

//
// VMDIR Errors
//

#define VMDIR_ERROR_BASE            9000

#define VMDIR_RANGE(n,x,y)                  (((x) <= (n)) && ((n) <= (y)))
#define IS_VMDIR_ERROR_SPACE(n) \
    VMDIR_RANGE((n),(VMDIR_ERROR_BASE ) , (VMDIR_ERROR_BASE + 999) )

#define VMDIR_SUCCESS                            0
#define VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION 9102
#define VMDIR_ERROR_ENTRY_NOT_FOUND           9106
#define VMDIR_ERROR_ENTRY_ALREADY_EXIST       9107
#define VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED 9108
#define VMDIR_ERROR_TIMELIMIT_EXCEEDED        9109
#define VMDIR_ERROR_SIZELIMIT_EXCEEDED        9110
#define VMDIR_ERROR_SASL_BIND_IN_PROGRESS     9111
#define VMDIR_ERROR_BUSY                      9112
#define VMDIR_ERROR_UNAVAILABLE               9113
#define VMDIR_ERROR_UNWILLING_TO_PERFORM      9114
#define VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF    9117
#define VMDIR_ERROR_SERVER_DOWN               9127
#define VMDIR_ERROR_RID_LIMIT_EXCEEDED        9200
#define VMDIR_ERROR_INSUFFICIENT_ACCESS       9207
#define VMDIR_ERROR_PASSWORD_TOO_LONG         9230
#define VMDIR_ERROR_PASSWORD_POLICY_VIOLATION 9232
#define VMDIR_ERROR_USER_LOCKOUT              9233
#define VMDIR_ERROR_USER_INVALID_CREDENTIAL   9234
#define VMDIR_ERROR_PASSWORD_EXPIRED          9239
#define VMDIR_ERROR_ACCOUNT_DISABLED          9240
#define VMDIR_ERROR_ACCOUNT_LOCKED            9241
#define VMDIR_ERROR_INVALID_POLICY_DEFINITION 9242
#define VMDIR_ERROR_USER_NO_CREDENTIAL        9243
#define VMDIR_ERROR_INVALID_DN                9602
#define VMDIR_ERROR_INVALID_SYNTAX            9603
#define VMDIR_ERROR_NO_SUCH_ATTRIBUTE         9611
#define VMDIR_ERROR_OBJECTCLASS_VIOLATION     9616
#define VMDIR_ERROR_STRUCTURE_VIOLATION       9617
#define VMDIR_ERROR_NAMING_VIOLATION          9618
#define VMDIR_ERROR_TYPE_OR_VALUE_EXISTS      9619
#define VMDIR_ERROR_UNDEFINED_TYPE            9620
#define VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND    9703
#define VMDIR_ERROR_BACKEND_PARENT_NOTFOUND   9704
#define VMDIR_ERROR_BACKEND_CONSTRAINT        9705
#define VMDIR_ERROR_BACKEND_ENTRY_EXISTS      9706
