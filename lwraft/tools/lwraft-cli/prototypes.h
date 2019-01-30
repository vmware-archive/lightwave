/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

// clinode.c

DWORD
RaftCliReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    );

DWORD
RaftCliListNodesA(
    PCSTR     pszHostName
    );

DWORD
RaftCliShowNodesA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    );

DWORD
RaftCliPromoteA(
    PCSTR     pszHostName,
    PCSTR     pszDomain,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    );

DWORD
RaftCliPromotePartnerA(
    PCSTR     pszHostName,
    PCSTR     pszPartnerName,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    );

DWORD
RaftCliDemoteA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword,
    PCSTR     pszLeaveNode
    );

DWORD
RaftCliStartVoteA(
    PCSTR   pszLogin,
    PCSTR   pszPassword,
    PCSTR   pszServerName
    );

DWORD
RaftCliStopProcessA(
    DWORD   dwGroupId
    );

DWORD
RaftCliStartProcessA(
    DWORD   dwGroupId
    );

DWORD
RaftCliListProcessesA(
    VOID
    );

// clibackup.c
DWORD
RaftCliDBBackup(
    PCSTR pszServerName,
    PCSTR pszLogin,
    PCSTR pszPassword,
    PCSTR pszBackupPath
    );

// clidr.c
DWORD
RaftCliDRNodeRestoreFromDB(
    PCSTR pszLogin,
    PCSTR pszPassword
    );
