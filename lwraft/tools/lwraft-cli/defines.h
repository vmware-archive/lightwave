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

typedef enum
{
    LWRAFT_DIR_COMMAND_UNKNOWN = 0,
    LWRAFT_DIR_COMMAND_NODE_STATE,
    LWRAFT_DIR_COMMAND_NODE_PROMOTE,
    LWRAFT_DIR_COMMAND_NODE_DEMOTE,
    LWRAFT_DIR_COMMAND_NODE_LIST,
} LWRAFT_NODE_COMMAND;

#define RAFT_LOGIN_DEFAULT           "administrator"


#define ERROR_LOCAL_BASE                            (100000)
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN        (ERROR_LOCAL_BASE + 1)
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_READ        (ERROR_LOCAL_BASE + 2)
#define ERROR_LOCAL_PASSWORD_EMPTY                  (ERROR_LOCAL_BASE + 3)

