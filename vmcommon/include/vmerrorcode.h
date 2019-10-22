/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef _VM_COMMON_ERRORCODE_H__
#define _VM_COMMON_ERRORCODE_H__

#define VM_COMMON_ERROR_BASE                 11000

#define VM_COMMON_RANGE(n,x,y) (((x) <= (n)) && ((n) <= (y)))

// common error space 11000~11999
#define IS_VM_COMMON_ERROR_SPACE(n) \
    VM_COMMON_RANGE( \
        (n), \
        (VM_COMMON_ERROR_BASE), \
        (VM_COMMON_ERROR_BASE + 999))

#define VM_COMMON_ERROR_FILE_NOT_FOUND       (VM_COMMON_ERROR_BASE + 2)    /* 11002 */
#define VM_COMMON_ERROR_NO_MEMORY            (VM_COMMON_ERROR_BASE + 8)    /* 11008 */
#define VM_COMMON_ERROR_INVALID_PARAMETER    (VM_COMMON_ERROR_BASE + 87)   /* 11087 */
#define VM_COMMON_ERROR_NO_DATA              (VM_COMMON_ERROR_BASE + 88)   /* 11088 */

#define VM_COMMON_ERROR_CURL_FAILURE         (VM_COMMON_ERROR_BASE + 129)  /* 11129 */
#define VM_COMMON_ERROR_CURL_INIT_FAILURE    (VM_COMMON_ERROR_BASE + 130)  /* 11130 */

#define VM_COMMON_ERROR_JSON_LOAD_FAILURE    (VM_COMMON_ERROR_BASE + 131)  /* 11131 */
#define VM_COMMON_ERROR_OPENSSL_FAILURE      (VM_COMMON_ERROR_BASE + 132)  /* 11132 */
#define VM_COMMON_ERROR_INVALID_ARGUMENT     (VM_COMMON_ERROR_BASE + 133)  /* 11133 */
#define VM_COMMON_INVALID_TIME_SPECIFIED     (VM_COMMON_ERROR_BASE + 134)  /* 11134 */
#define VM_COMMON_UNSUPPORTED_HTTP_METHOD    (VM_COMMON_ERROR_BASE + 135)  /* 11135 */
#define VM_COMMON_INVALID_EVP_METHOD         (VM_COMMON_ERROR_BASE + 136)  /* 11136 */
#define VM_COMMON_ERROR_JSON_MAP_BAD_TYPE    (VM_COMMON_ERROR_BASE + 137)  /* 11137 */
#define VM_COMMON_ERROR_INVALID_TIME         (VM_COMMON_ERROR_BASE + 138)  /* 11138 */
#define VM_COMMON_ERROR_JSON_WRITE_FILE      (VM_COMMON_ERROR_BASE + 139)  /* 11139 */

#define VM_COMMON_ERROR_BAD_AUTH_DATA        (VM_COMMON_ERROR_BASE + 150)  /* 11150 */

// 11200 ~ 11299 VM COMMON  error
#define VM_COMMON_ERROR_FILE_IO              (VM_COMMON_ERROR_BASE + 200)  /* 11200 */
#define VM_COMMON_ERROR_NOT_FOUND            (VM_COMMON_ERROR_BASE + 201)  /* 11201 */
#define VM_COMMON_ERROR_SIZELIMIT_EXCEEDED   (VM_COMMON_ERROR_BASE + 202)  /* 11202 */
#define VM_COMMON_ERROR_OPERATION_NOT_PERMITTED (VM_COMMON_ERROR_BASE + 203)  /* 11203 */
#define VM_COMMON_ERROR_NO_SUCH_USER         (VM_COMMON_ERROR_BASE + 204)  /* 11204 */
#define VM_COMMON_ERROR_FILE_CHANGE_OWNER    (VM_COMMON_ERROR_BASE + 205)  /* 11205 */

// 11300 ~ 11350 regconfig error
#define VM_COMMON_ERROR_REGCONFIG_KEY_NOT_FOUND (VM_COMMON_ERROR_BASE + 300)  /* 11300 */
#define VM_COMMON_ERROR_REGCONFIG_BAD_KEY       (VM_COMMON_ERROR_BASE + 301)  /* 11301 */
#define VM_COMMON_ERROR_REGCONFIG_BAD_VALUE     (VM_COMMON_ERROR_BASE + 302)  /* 11302 */
#define VM_COMMON_ERROR_REGCONFIG_VALUE_TOO_BIG (VM_COMMON_ERROR_BASE + 303)  /* 11303 */
#define VM_COMMON_ERROR_REGCONFIG_INVALID_STATE (VM_COMMON_ERROR_BASE + 304)  /* 11304 */
#define VM_COMMON_ERROR_REGCONFIG_INVALID_MERGE (VM_COMMON_ERROR_BASE + 305)  /* 11305 */

// 11500 ~ 11550 YAML error
#define VM_COMMON_ERROR_YAML_PARSE_INIT      (VM_COMMON_ERROR_BASE + 500)  /* 11500 */
#define VM_COMMON_ERROR_YAML_PARSE           (VM_COMMON_ERROR_BASE + 501)  /* 11501 */

#endif /* __VM_COMMON_ERRORCODE_H__ */
