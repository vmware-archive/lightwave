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
    PSTR *pStringList;
    DWORD dwCount; // Current count.
    DWORD dwSize; // Max number of strings we can store currently.
} STRING_LIST, *PSTRING_LIST;

typedef struct
{
    PCSTR pszServerName;        // the server name
    PCSTR pszUserName;          // administrator user - default to "Administrator"
    PCSTR pszPassword;          // administrator password
    PCSTR pszObjectName;        // Name (DN) of the object to operate on.
    PCSTR pszBaseDN;            // The base DN that we'll query against for users/groups.
    PCSTR pszGrantParameter;    // What user/group we're granting privileges to (if any).
    PCSTR pszRemoveParameter;   // What user/group we're removing a privilege from.
    BOOLEAN bVerbose;           // Break down the object's ACL information.
    PCSTR pszPasswordFile;      // password file
} COMMAND_LINE_PARAMETER_STATE, *PCOMMAND_LINE_PARAMETER_STATE;
