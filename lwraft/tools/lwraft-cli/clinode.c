/*
 * Copyright © 2107 VMware, Inc.  All Rights Reserved.
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

#include "includes.h"

static
DWORD
RaftCliParsePrincipal(
    PCSTR pszHostname,
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    );

static
VOID
RaftCliPrintClusterNode(
    PVMDIR_RAFT_CLUSTER pCluster
    );

static
VOID
RaftCliPrintClusterState(
    PVMDIR_RAFT_CLUSTER pCluster
    );

DWORD
RaftCliListNodesA(
    PCSTR     pszHostName
    )
{
    DWORD     dwError = 0;
    PVMDIR_RAFT_CLUSTER pCluster = NULL;

    if (IsNullOrEmptyString(pszHostName))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirRaftListCluster(pszHostName, &pCluster);
    BAIL_ON_VMDIR_ERROR(dwError);

    RaftCliPrintClusterNode(pCluster);

cleanup:
    VmDirFreeRaftCluster(pCluster);

    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliShowNodesA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword
    )
{
    DWORD     dwError = 0;
    PSTR      pszDomain = NULL;
    PSTR      pszPassword1 = NULL;
    PSTR      pszUser = NULL;
    PCSTR     pszPasswordLocal = pszPassword;
    PCSTR     pszLoginLocal = pszLogin ? pszLogin : RAFT_LOGIN_DEFAULT;
    PVMDIR_RAFT_CLUSTER pCluster = NULL;

    if (IsNullOrEmptyString(pszHostName))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = RaftCliParsePrincipal(pszHostName, pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = RaftCliReadPassword(
                    pszUser,
                    pszDomain,
                    NULL,
                    &pszPassword1);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirRaftShowClusterState(
                pszHostName,
                pszDomain,
                pszUser,
                pszPasswordLocal,
                &pCluster);
    BAIL_ON_VMDIR_ERROR(dwError);

    RaftCliPrintClusterState(pCluster);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUser);
    VMDIR_SAFE_FREE_MEMORY(pszPassword1);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);
    VmDirFreeRaftCluster(pCluster);

    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliPromoteA(
    PCSTR     pszPreferredHostName,
    PCSTR     pszDomain,
    PCSTR     pszUser,
    PCSTR     pszPassword
    )
{
    DWORD   dwError = 0;
    PCSTR   pszLocalUser = pszUser ? pszUser : RAFT_LOGIN_DEFAULT;
    PSTR    pszPassword1 = NULL;
    PCSTR   pszLocalPassword = pszPassword;

    if (IsNullOrEmptyString(pszDomain))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!pszLocalPassword)
    {
        dwError = RaftCliReadPassword(
                    pszUser,
                    pszDomain,
                    NULL,
                    &pszPassword1);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszLocalPassword = pszPassword1;
    }

    dwError = VmDirSetupHostInstance(
                pszDomain,
                pszPreferredHostName ? pszPreferredHostName : "localhost",
                pszLocalUser,
                pszLocalPassword,
                NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszPassword1);

    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliPromotePartnerA(
    PCSTR     pszPreferredHostName,
    PCSTR     pszPartnerName,
    PCSTR     pszUser,
    PCSTR     pszPassword
    )
{
    DWORD   dwError = 0;
    PCSTR   pszLocalUser = pszUser ? pszUser : RAFT_LOGIN_DEFAULT;
    PCSTR   pszLocalPassword = pszPassword;
    PSTR    pszPassword1 = NULL;
    PSTR    pszLeaderNameCanon = NULL;
    PSTR    pszLeader = NULL;
    PSTR    pszLocalDomain = NULL;

    if (IsNullOrEmptyString(pszPartnerName))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if (!pszLocalPassword)
    {
        dwError = VmDirGetDomainName(pszPartnerName, &pszLocalDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = RaftCliReadPassword(
                    pszLocalUser,
                    pszLocalDomain,
                    NULL,
                    &pszPassword1);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszLocalPassword = pszPassword1;
    }

    dwError = VmDirRaftLeader(pszPartnerName, &pszLeader);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetCanonicalHostName(pszLeader, &pszLeaderNameCanon);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirJoin(
                pszPreferredHostName ? pszPreferredHostName : "localhost",
                pszLocalUser,
                pszLocalPassword,
                NULL,
                pszLeaderNameCanon,
                FIRST_REPL_CYCLE_MODE_COPY_DB);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLeaderNameCanon);
    VMDIR_SAFE_FREE_MEMORY(pszPassword1);
    VMDIR_SAFE_FREE_MEMORY(pszLocalDomain);
    VMDIR_SAFE_FREE_MEMORY(pszLeader);

    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliDemoteA(
    PCSTR     pszHostName,
    PCSTR     pszLogin,
    PCSTR     pszPassword,
    PCSTR     pszLeaveNode
    )
{
    DWORD     dwError = 0;
    PSTR      pszDomain = NULL;
    PSTR      pszPassword1 = NULL;
    PSTR      pszUser = NULL;
    PCSTR     pszPasswordLocal = pszPassword;
    PCSTR     pszLoginLocal = pszLogin ? pszLogin : RAFT_LOGIN_DEFAULT;

    if (IsNullOrEmptyString(pszHostName) ||
        IsNullOrEmptyString(pszLeaveNode))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = RaftCliParsePrincipal(pszHostName, pszLoginLocal, &pszUser, &pszDomain);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (!pszPassword)
    {
        dwError = RaftCliReadPassword(
                    pszUser,
                    pszDomain,
                    NULL,
                    &pszPassword1);
        BAIL_ON_VMDIR_ERROR(dwError);
        pszPasswordLocal = pszPassword1;
    }

    dwError = VmDirRaftLeaveCluster(
                pszHostName,
                pszDomain,
                pszUser,
                pszPasswordLocal,
                pszLeaveNode);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszUser);
    VMDIR_SAFE_FREE_MEMORY(pszPassword1);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);

    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliStartVoteA(
    PCSTR   pszLogin,
    PCSTR   pszPassword,
    PCSTR   pszServerName
    )
{
    DWORD   dwError = 0;
    PSTR    pszLeader = NULL;

    if (IsNullOrEmptyString(pszLogin) ||
            IsNullOrEmptyString(pszPassword))
    {
        BAIL_WITH_VMDIR_ERROR(dwError, VMDIR_ERROR_INVALID_PARAMETER);
    }

    dwError = VmDirRaftLeader(
            pszServerName ? pszServerName : "localhost",
            &pszLeader);
    BAIL_ON_VMDIR_ERROR(dwError);

    // always target the leader node
    dwError = VmDirRaftStartVoteClient(pszLogin, pszPassword, pszLeader);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLeader);
    return dwError;

error:
    goto cleanup;
}

DWORD
RaftCliReadPassword(
    PCSTR pszUser,
    PCSTR pszDomain,
    PCSTR pszPrompt,
    PSTR* ppszPassword
    )
{
    DWORD dwError = 0;
    struct termios orig, nonecho;
    CHAR  szPassword[33] = "";
    PSTR  pszPasswordTemp = szPassword;
    PSTR  pszPassword = NULL;
    DWORD iChar = 0;

    memset(szPassword, 0, sizeof(szPassword));

    if (IsNullOrEmptyString(pszPrompt))
    {
        fprintf(stdout, "Enter password for %s@%s: ", pszUser, pszDomain);
    }
    else
    {
        fprintf(stdout, "%s:", pszPrompt);
    }
    fflush(stdout);

    tcgetattr(0, &orig); // get current settings
    memcpy(&nonecho, &orig, sizeof(struct termios)); // copy settings
    nonecho.c_lflag &= ~(ECHO); // don't echo password characters
    tcsetattr(0, TCSANOW, &nonecho); // set current settings to not echo

    // Read up to 32 characters of password

    for (; iChar < sizeof(szPassword); iChar++)
    {
        ssize_t nRead = 0;
        CHAR ch;

        if ((nRead = read(STDIN_FILENO, &ch, 1)) < 0)
        {
            dwError = LwErrnoToWin32Error(errno);
            BAIL_ON_VMDIR_ERROR(dwError);
        }

        if (nRead == 0 || ch == '\n')
        {
            fprintf(stdout, "\n");
            fflush(stdout);
            break;
        }
        else if (ch == '\b') /* backspace */
        {
            if (iChar > 0)
            {
                iChar--;
                szPassword[iChar] = '\0';
            }
        }
        else
        {
            szPassword[iChar] = ch;
        }
    }

    if (IsNullOrEmptyString(pszPasswordTemp))
    {
        dwError = ERROR_PASSWORD_RESTRICTION;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirAllocateStringA(szPassword, &pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    *ppszPassword = pszPassword;

cleanup:

    tcsetattr(0, TCSANOW, &orig);

    return dwError;

error:

    *ppszPassword = NULL;

    goto cleanup;
}

static
DWORD
RaftCliParsePrincipal(
    PCSTR pszHostname,
    PCSTR pszLogin,
    PSTR* ppszUser,
    PSTR* ppszDomain
    )
{
    DWORD dwError = 0;
    PCSTR pszCursor = NULL;
    PSTR  pszUser = NULL;
    PSTR  pszDomain = NULL;

    if (IsNullOrEmptyString(pszLogin) || *pszLogin == '@'
            || !ppszUser || !ppszDomain)
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (!(pszCursor = strchr(pszLogin, '@')) || !pszCursor++)
    {
        dwError = VmDirGetDomainName(pszHostname, &pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);


        dwError = VmDirAllocateStringA(pszLogin, &pszUser);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    else
    {
        size_t len = pszCursor-pszLogin-1;
        int i = 0;

        dwError = VmDirAllocateStringA(pszCursor, &pszDomain);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = VmDirAllocateMemory(len+1, (PVOID*)&pszUser);
        BAIL_ON_VMDIR_ERROR(dwError);

        for (; i < len; i++)
        {
            pszUser[i] = pszLogin[i];
        }
    }

    *ppszUser = pszUser;
    *ppszDomain = pszDomain;

cleanup:

    return dwError;

error:

    *ppszUser = NULL;
    *ppszDomain = NULL;

    VMDIR_SAFE_FREE_MEMORY(pszUser);
    VMDIR_SAFE_FREE_MEMORY(pszDomain);

    goto cleanup;
}

static
VOID
RaftCliPrintClusterNode(
    PVMDIR_RAFT_CLUSTER pCluster
    )
{
    PVMDIR_RAFT_NODE    pNode = NULL;

    fprintf(stdout,
            "\nRaft leader:\n    %s\n\n",
            pCluster->pszLeader ? pCluster->pszLeader:"N/A");

    fprintf(stdout, "Raft follower:\n");
    for (pNode = pCluster->pNode; pNode; pNode=pNode->pNext)
    {
        if (pNode->role == VMDIRD_RAFT_ROLE_FOLLOWER)
        {
            fprintf(stdout, "    %s\n", pNode->pszName);
        }
    }
    fprintf(stdout, "\n");

    fprintf(stdout, "Raft offline or candidate member:\n");
    for (pNode = pCluster->pNode; pNode; pNode=pNode->pNext)
    {
        if (pNode->role == VMDIRD_RAFT_ROLE_CANDIDATE)
        {
            fprintf(stdout, "    %s\n", pNode->pszName);
        }
    }

    fprintf(stdout,
            "\n\n%-5d active followers\n%-5d nodes in Raft Cluster\n\n",
            pCluster->dwNumActiveFollower,
            pCluster->dwNumMmember);
}

static
VOID
RaftCliPrintClusterState(
    PVMDIR_RAFT_CLUSTER pCluster
    )
{
    PVMDIR_RAFT_NODE    pNode = NULL;

    fprintf(stdout,
            "\n%-30s %-10s %-6s %-15s %-15s\n",
            "Node Name", "Role", "Term", "LastIndex", "LastAppliedIndex");
    fprintf(stdout,
            "%-30s %-10s %-6s %-15s %-15s\n",
            "------------------------------", "----------", "------", "---------------", "----------------");

    for (pNode = pCluster->pNode; pNode; pNode=pNode->pNext)
    {
        fprintf(stdout,
                "%-30s %-10s %-6lu %-15lu %-15lu\n",
                pNode->pszName,
                pNode->role == VMDIRD_RAFT_ROLE_LEADER ? "Leader": (pNode->role == VMDIRD_RAFT_ROLE_FOLLOWER ? "Follower":"Member"),
                pNode->iRaftTerm,
                pNode->iLastLogIndex,
                pNode->iLastAppliedIndex);
    }
    fprintf(stdout, "\n");
}
