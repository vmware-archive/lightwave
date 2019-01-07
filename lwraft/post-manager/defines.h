/*
 * Copyright ©2019 VMware, Inc.  All Rights Reserved.
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

#define VMDIR_OPTION_LOGGING_LEVEL          'l'
#define VMDIR_OPTION_LOG_FILE_NAME          'L'
#define VMDIR_OPTION_ENABLE_SYSLOG          's'
#define VMDIR_OPTION_CONSOLE_MODE           'c'
#define VMDIR_OPTIONS_VALID                 "l:L:sc:"

/*
 * Table to define and initialize VMDIR configuration data.
 *
 * To add a new configuration key,
 * 1. define its name in vmdircommon.h
 * 2. define its entry in the table below and init default/cfg Value
 *
 * VMDIR_CONFIG_VALUE_TYPE_STRING <-> REG_SZ
 * VMDIR_CONFIG_VALUE_TYPE_MULTISTRING <-> REG_MULI_SZ
 * VMDIR_CONFIG_VALUE_TYPE_DWORD  <-> REG_DWORD
 * VMDIR_CONFIG_VALUE_TYPE_BOOLEAN <-> REG_DWORD
 *
 */
