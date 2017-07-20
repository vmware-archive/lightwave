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

typedef struct
{
    PSTR pszServerName;         // the server name
    PSTR pszUserName;           // administrator user - default to "Administrator"
    PSTR pszPassword;           // administrator password
    PSTR pszObjectName;         // Name (DN) of the object to operate on.
    PSTR pszBaseDN;             // The base DN that we'll query against for users/groups.
    PSTR pszGrantParameter;     // What user/group we're granting privileges to (if any).
    PSTR pszRemoveParameter;    // What user/group we're removing a privilege from.
    PSTR pszPasswordFile;       // password file
    BOOLEAN bVerbose;           // Break down the object's ACL information.
    BOOLEAN bRecursive;         // Apply the operation to the specified object
                                // and all objects below it.
} COMMAND_LINE_PARAMETER_STATE, *PCOMMAND_LINE_PARAMETER_STATE;
