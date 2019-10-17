/*
 * Copyright Â   2019 VMware, Inc.  All Rights Reserved.
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

#ifndef __VM_REGCONFIG_DEFINE_H__
#define __VM_REGCONFIG_DEFINE_H__

#define VM_REGCONFIG_KEY_SEPARATOR_STR  "\\"
#define VM_REGCONFIG_KEY_SEPARATOR      '\\'
#define VM_REGCONFIG_TOP_KEY_PATH       "Services"
#define VM_REGCONFIG_TOP_KEY_PATH_LEN   sizeof(VM_REGCONFIG_TOP_KEY_PATH)-1

#define VM_REGCONFIG_INDENTATION        4
#define VM_REGCONFIG_INDENTATION_STR    "    "

#define VM_REGCONFIG_KEY_ESCAPE         '\\'
#define VM_REGCONFIG_KEY_QUOTE_STR      "\""
#define VM_REGCONFIG_KEY_DQUOTE          '"'

#define VM_REGCONFIG_MULTISZ_STR        "|"
#define VM_REGCONFIG_NEWLINE            '\n'
#define VM_REGCONFIG_NEWLINE_STR        "\n"

#endif /* __VM_REGCONFIG_DEFINE_H__ */
