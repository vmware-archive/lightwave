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

#define VM_COMMON_ERROR_NO_MEMORY            (VM_COMMON_ERROR_BASE + 8)    /* 11008 */
#define VM_COMMON_ERROR_INVALID_PARAMETER    (VM_COMMON_ERROR_BASE + 87)   /* 11087 */
#define VM_COMMON_ERROR_NO_DATA              (VM_COMMON_ERROR_BASE + 88)   /* 11088 */

#define VM_COMMON_ERROR_CURL_FAILURE         (VM_COMMON_ERROR_BASE + 129)  /* 11129 */
#define VM_COMMON_ERROR_CURL_INIT_FAILURE    (VM_COMMON_ERROR_BASE + 130)  /* 11130 */

#define VM_COMMON_ERROR_JSON_LOAD_FAILURE    (VM_COMMON_ERROR_BASE + 131)  /* 11131 */

#endif /* __VM_COMMON_ERRORCODE_H__ */
