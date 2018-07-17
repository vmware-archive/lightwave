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
 * Module Name: VMAFD
 *
 * Filename: vmafderrorcode.h
 *
 * Abstract:
 *
 * Common error code map
 *
 */

#ifndef __VMAFDERRORCODE_H__
#define __VMAFDERRORCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * <asm-generic/errno-base.h>
 * <asm-generic/errno.h>
 */
#define ERROR_OPERATION_NOT_PERMITTED    EPERM              //  1
#define ERROR_NO_SUCH_FILE_OR_DIRECTORY  ENOENT             //  2
#define ERROR_NO_SUCH_PROCESS            ESRCH              //  3
#define ERROR_INT_SYSTEM_CALL            EINTR              //  4
#define ERROR_IO                         EIO                //  5
#define ERROR_NO_SUCH_DEVICE_OR_ADDR     ENXIO              //  6
#define ERROR_ARG_LIST_TOO_LONG          E2BIG              //  7
#define ERROR_EXEC_FORMT                 ENOEXEC            //  8
#define ERROR_BAD_FILE_NUMUMBER          EBADF              //  9
#define ERROR_NO_CHILD_PROCESS           ECHILD             //  10
#define ERROR_TRY_AGAIN                  EAGIN              //  11
#define ERROR_NO_MEMORY                  ENOMEM             //  12
// #define ERROR_ACCESS_DENIED              EACCES             //  13
#define ERROR_RESOURCE_BUSY              EBUSY              //  16
// // #define ERROR_FILE_EXISTS                EEXIST             //  17
#define ERROR_NO_SUCH_DEVICE             ENODEV             //  19
#define ERROR_NOT_A_DIRECTORY            ENOTDIR            //  20
#define ERROR_IS_A_DIRECTORY             EISDIR             //  21
// #define ERROR_INVALID_PARAMETER          EINVAL             //  22
// #define ERROR_FILE_TOO_LARGE             EFBIG              //  27
#define ERROR_NO_SPACE                   ENOSPC             //  28
#define ERROR_ILLEGAL_SEEK               ESPIPE             //  29
#define ERROR_READ_ONLY_FILESYSTEM       EROFS              //  30
// #define ERROR_BROKEN_PIPE                EPIPE              //  32
#define ERROR_OUT_OF_RANGE               ERANGE             //  34

#define ERROR_DEADL_LOCK                 EDEADLK            //  35
#define ERROR_NAME_TOO_LONG              ENAMETOOLONG       //  36

#define ERROR_NO_DATA_AVAILABLE          ENODATA            //  61
#define ERROR_TIME_EXPIRED               ETIME              //  62
// #define ERROR_NO_NETWORK                 ENONET             //  64
#define ERROR_COMMUNICATION              ECOMM              //  70
#define ERROR_PROTOCOL                   EPROTO             //  71
#define ERROR_VALUE_OVERFLOW             EOVERFLOW          //  75
#define ERROR_NAME_NOT_UNIQUE            ENOTUNIQ           //  76
#define ERROR_BAD_FILE_DESCRIPTOR        EBADFD             //  77

/*
 * vmafd defined error code
 */
#define VMAFD_RANGE(n,x,y)                  (((x) <= (n)) && ((n) <= (y)))

// generic error (range 1000 - 1999)
#define ERROR_INVALID_CONFIGURATION     1000
#define ERROR_DATA_CONSTRAIN_VIOLATION  1001
#define ERROR_OPERATION_INTERRUPT       1002

// object sid generation
#define ERROR_ORG_LIMIT_EXCEEDED        1003
#define ERROR_RID_LIMIT_EXCEEDED        1004
#define ERROR_ORG_ID_GEN_FAILED         1005
#define ERROR_NO_OBJECT_SID_GEN         1006

// ACL/SD
#define ERROR_NO_SECURITY_DESCRIPTOR    1007
#define ERROR_NO_OBJECTSID_ATTR         1008
#define ERROR_TOKEN_IN_USE              1009
#define ERROR_NO_MYSELF                 1010

#define ERROR_BAD_ATTRIBUTE_DATA        1011
#define ERROR_PASSWORD_TOO_LONG         1012

#define ERROR_CANNOT_PROMOTE_VMDIR      1013
#define ERROR_CANNOT_DEMOTE_VMDIR       1014
#define ERROR_CANNOT_JOIN_VMDIR         1015
#define ERROR_CANNOT_LEAVE_VMDIR        1016
#define ERROR_CANNOT_SPLIT_VMDIR        1017
#define ERROR_CANNOT_MERGE_VMDIR        1018

#define ERROR_CANNOT_PROMOTE_VMDIR_ALREADY_PROMOTED    1019
#define ERROR_CANNOT_CREATE_KERBEROS_CONFIGURATION    1020
#define ERROR_CANNOT_CONNECT_VMAFD      1021

// oidc sso errors
#define VMAFD_SSOERROR_NONE                                     0
#define VMAFD_SSOERROR_INVALID_ARGUMENT                         1101
#define VMAFD_SSOERROR_OUT_OF_MEMORY                            1102
#define VMAFD_SSOERROR_CURL_FAILURE                             1103
#define VMAFD_SSOERROR_CURL_INIT_FAILURE                        1104
#define VMAFD_SSOERROR_HTTP_SEND_FAILURE                        1104
#define VMAFD_SSOERROR_JSON_FAILURE                             1105
#define VMAFD_SSOERROR_JSON_PARSE_FAILURE                       1106
#define VMAFD_SSOERROR_OPENSSL_FAILURE                          1107
#define VMAFD_SSOERROR_VMAFD_LOAD_FAILURE                       1108
#define VMAFD_SSOERROR_VMAFD_CALL_FAILURE                       1109
#define VMAFD_SSOERROR_TOKEN_INVALID_SIGNATURE                  1110
#define VMAFD_SSOERROR_TOKEN_INVALID_AUDIENCE                   1111
#define VMAFD_SSOERROR_TOKEN_EXPIRED                            1112

#define VMAFD_SSOERROR_OIDC_SERVER                              1113
#define VMAFD_SSOERROR_OIDC_SERVER_INVALID_REQUEST              1114
#define VMAFD_SSOERROR_OIDC_SERVER_INVALID_SCOPE                1115
#define VMAFD_SSOERROR_OIDC_SERVER_INVALID_GRANT                1116
#define VMAFD_SSOERROR_OIDC_SERVER_INVALID_CLIENT               1117
#define VMAFD_SSOERROR_OIDC_SERVER_UNSUPPORTED_RESPONSE_TYPE    1118
#define VMAFD_SSOERROR_OIDC_SERVER_UNSUPPORTED_GRANT_TYPE       1119
#define VMAFD_SSOERROR_OIDC_SERVER_ACCESS_DENIED                1120
#define VMAFD_SSOERROR_OIDC_SERVER_SERVER_ERROR                 1121

#define VMAFD_SSOERROR_UNKNOWN                                  1122
#define VMAFD_OIDC_ERROR_RESPONSE                               1022
#define VMAFD_OIDC_EMPTY_RESPONSE                               1023

#ifdef __cplusplus
}
#endif

#endif /* __VMAFDERRORCODE_H__ */
