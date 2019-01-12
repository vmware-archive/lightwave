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

#define SIZE_512    512

#define VMDIR_STATE_STARTING    1
#define VMDIR_STATE_RUNNING     2
#define VMDIR_STATE_STOPPING    3
#define VMDIR_STATE_STOPPED     4
#define VMDIR_STATE_DEAD        5

#define VMDIR_POSTD_PATH    "PostdPath"
#define VMDIR_POSTD_ARGS    "PostdArgs"

typedef struct _VMDIR_PROCESS
{
    pid_t   pid;
    DWORD   dwState;
} VMDIR_PROCESS, *PVMDIR_PROCESS;

typedef struct _VMDIR_PROCESS_TABLE
{
    PLW_HASHMAP     pProcessMap;
    PVMDIR_RWLOCK   pProcessTableLock;
} VMDIR_PROCESS_TABLE, *PVMDIR_PROCESS_TABLE;

PVMDIR_PROCESS_TABLE gpProcessTable;
PSTR                 gpszPostdPath;
PSTR                 *gppszPostdArgs;
