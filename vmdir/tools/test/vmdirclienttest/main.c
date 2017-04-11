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


#include "includes.h"

/* Example of why you don't call public APIs internally */
ULONG
VmDirCreateBindingHandleA(
    PCSTR      pszNetworkAddress,
    PCSTR      pszNetworkEndpoint,
    handle_t   *ppBinding
    );

DWORD
TestVmDirLdapGetResults(
    LDAP    *pLd,
    int      msgid,
    uint64_t startTime,
    BOOLEAN  displayTimeTaken
    );

#define SIZE_256    256

#if 0
static void _PrintKrbKey(PBYTE pMasterKey, DWORD dwLen)
{
    DWORD i=0;
    printf("\nkey size =  %d\n",dwLen);
    for (i=0; i<dwLen; i++)
    {
        printf("%02x", pMasterKey[i]);
        if ((i+1) % 32 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");
}
#endif

#if 0
void TestVmDirGetKrbMasterKey()
{
    char        pszDomainName[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwSize = 0;

    printf( "  Kerberos realm name:");
    scanf("%s", pszDomainName);

    dwError = VmDirGetKrbMasterKey(
        pszDomainName,
        &pLocalByte,
        &dwSize
        );

    printf("TestRpcVmDirGetKrbMasterKey returns %d\n",dwError);

    if (dwError == 0 )
    {
        _PrintKrbKey(pLocalByte, dwSize);
    }

    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}
#endif

#if 0
void TestVmDirGetKrbUPNKey()
{
    char        pszUpnName[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwSize = 0;

    printf( "  UserPrincipalName:");
    scanf("%s", pszUpnName);

    dwError = VmDirGetKrbUPNKey(
        pszUpnName,
        &pLocalByte,
        &dwSize
        );
    printf("TestRpcVmDirGetKrbUPNKey returns %d\n",dwError);

    if (dwError == 0)
    {
        _PrintKrbKey(pLocalByte, dwSize);
    }

    printf("\n");
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}
#endif

void
TestVmDirForceResetPassword(
    VOID
    )
{
    char        pszDN[SIZE_256] = {0};
    DWORD       dwError = 0;
    PBYTE       pLocalByte = NULL;
    DWORD       dwByteSize = 0;

    printf( "  Account DN: ");
    scanf("%s", pszDN);

    dwError = VmDirForceResetPassword( pszDN, &pLocalByte, &dwByteSize );

    printf("TestRpcVmDirForceResetPassword returns %d\n",dwError);

    if (dwError == 0)
    {
        DWORD dwCnt = 0;
        printf("Pasword reset to:");
        for (dwCnt=0; dwCnt<dwByteSize; dwCnt++)
        {
            printf("%c", pLocalByte[dwCnt]);
        }
        printf("\n");
    }

    printf("\n");
    VMDIR_SAFE_FREE_MEMORY( pLocalByte );
}

void
TestVmDirSetLogParameters(
    VOID
    )
{
    DWORD       dwError = 0;
    char        pszLogLevel[SIZE_256] = {0};
    int         iMask = 0;

    printf( "  Log level (ERROR|WARNING|INFO|VERBOSE|DEBUG):");
    scanf("%s", pszLogLevel);
    printf( "  Log mask :");
    scanf("%d", &iMask);

    dwError = VmDirSetLogLevel(pszLogLevel);
    if (dwError)
    {
        printf( "VmDirSetLogLevel failed (%u)", dwError);
    }

    dwError = VmDirSetLogMask(iMask);
    if (dwError)
    {
        printf( "VmDirSetLogMask failed (%u)", dwError);
    }
}

void TestVmDirCreateUser()
{
    char        pszUserName[SIZE_256] = {0};
    char        pszPassword[SIZE_256] = {0};
    char        pszUPNName[SIZE_256] = {0};
    DWORD       dwError = 0;

    printf( "  User account:");
    scanf("%s", pszUserName);
    printf( "  User password:");
    scanf("%s", pszPassword);
    printf( "  UPN name:");
    scanf("%s", pszUPNName);

    dwError = VmDirCreateUser(
        pszUserName,
        pszPassword,
        pszUPNName,
        FALSE
        );
    printf("TestRpcVmDirCreateUser returns %d\n",dwError);
}

void TestVmDirCreateUserEx()
{
    DWORD  dwError = 0;
    char   szAdmin[] = "Administrator";
    char   szDomain[] = "vsphere.local";
    char   szServer[SIZE_256] = {0};
    char   szAdminPassword[SIZE_256] = {0};
    char   szUserName[SIZE_256] = {0};
    char   szAccount[SIZE_256] = {0};
    char   szFirstname[SIZE_256] = {0};
    char   szLastname[SIZE_256] = {0};
    char   szUPNName[SIZE_256] = {0};
    char   szPassword[SIZE_256] = {0};
    PVMDIR_SERVER_CONTEXT pVmDirCtx = NULL;
    VMDIR_USER_CREATE_PARAMS_A createParams = {0};

    printf("DC:");
    scanf("%s", szServer);
    printf("Password (%s@%s):", szAdmin, szDomain);
    scanf("%s", szAdminPassword);
    printf("First name:");
    scanf("%s", szFirstname);
    printf("Last name:");
    scanf("%s", szLastname);
    printf("Account name:");
    scanf("%s", szAccount);
    printf("Common name:");
    scanf("%s", szUserName);
    printf("Password:");
    scanf("%s", szPassword);

    dwError = VmDirOpenServerA(
                   IsNullOrEmptyString(szServer) ? "localhost" : szServer,
                   szAdmin,
                   szDomain,
                   szAdminPassword,
                   0,
                   NULL,
                   &pVmDirCtx);
    BAIL_ON_VMDIR_ERROR(dwError);  

    createParams.pszName = &szUserName[0];
    createParams.pszAccount = &szAccount[0];
    createParams.pszFirstname = &szFirstname[0];
    createParams.pszLastname = &szLastname[0];
    createParams.pszUPN      = szUPNName;
    createParams.pszPassword = &szPassword[0];

    dwError = VmDirCreateUserA(
                   pVmDirCtx,
                   &createParams);
    BAIL_ON_VMDIR_ERROR(dwError);  

error:

    printf("TestRpcVmDirCreateUserEx returns %d\n",dwError);

    if (pVmDirCtx)
    {
        VmDirCloseServer(pVmDirCtx);
    }
}

void
TestVmDirDBFileTransfer()
{
    printf("TestVmDirDBFileTransfer is not implemented yet.\n");
}

void
TestVmDirReplNow()
{

    DWORD       dwError = 0;
    char        pszServerName[VMDIR_MAX_HOSTNAME_LEN];
    PSTR        pszLocalErrorMsg = NULL;

    printf("Enter partner hostname: ");
    scanf("%s", pszServerName);

    dwError = VmDirReplNow( pszServerName );
    BAIL_ON_VMDIR_ERROR_WITH_MSG( dwError, (pszLocalErrorMsg),
                "TestVmDirReplNow: VmDirReplNow() call failed with error: %d", dwError  );

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return;

error:
    printf( "%s\n", pszLocalErrorMsg ? pszLocalErrorMsg : "Hmmm ... no local error message."  );
    goto cleanup;
}

DWORD
TestVmDirGenerateNewUserAttributes(
    PSTR    newDN,
    PSTR    newSN,
    PSTR    newCN,
    DWORD   value
    )
{
    char  *pPartialDN = ",cn=users,dc=vsphere,dc=local";
    char  *pUser = "cn=newuser";
    PSTR   pUserCount = NULL;
    size_t newsize = 0;
    DWORD  dwError = 0;

    VmDirAllocateStringPrintf(&pUserCount, "%d", value);

    dwError = VmDirStringCpyA(newDN, VmDirStringLenA(pUser)+1, pUser);
    BAIL_ON_VMDIR_ERROR(dwError);

    newsize = SIZE_256 - VmDirStringLenA(newDN);
    dwError = VmDirStringCatA(newDN, newsize, pUserCount);
    BAIL_ON_VMDIR_ERROR(dwError);

    if (newSN != NULL)
    {
        //generateSN
        dwError = VmDirStringCpyA(newSN, VmDirStringLenA(newDN)+1, newDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (newCN != NULL)
    {
        //generateCN
        dwError = VmDirStringCpyA(newCN, VmDirStringLenA(newDN)+1, newDN);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    //generateDN
    newsize = SIZE_256 - VmDirStringLenA(newDN);
    dwError = VmDirStringCatA(newDN, newsize, pPartialDN);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pUserCount);
    return dwError;

error:
   printf(" \n TestVmDirGenerateNewUserDn failed. (%d)\n", dwError);
   goto cleanup;
}

DWORD
TestVmDirGenerateModifyCN(
    PSTR   newCN,
    DWORD  value
    )
{
    PSTR   pUserCount = NULL;
    char   *pUser = "newuser_";
    DWORD  dwError = 0;
    size_t newsize = 0;

    VmDirAllocateStringPrintf(&pUserCount, "%d", value);

    dwError = VmDirStringCpyA(newCN, VmDirStringLenA(pUser)+1, pUser);
    BAIL_ON_VMDIR_ERROR(dwError);

    newsize = SIZE_256 - VmDirStringLenA(newCN);
    dwError = VmDirStringCatA(newCN, newsize, pUserCount);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    VMDIR_SAFE_FREE_MEMORY(pUserCount);
    return dwError;

error:
   printf(" \n TestVmDirGenerateModifyCN failed. (%d)\n", dwError);
   goto cleanup;
}

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, TCHAR *targv[])
#endif
{
    while (1)
    {
        int choice = -1;

        printf( "\n\n==================\n");
        printf( "Please select:\n");
        printf( "0. exit\n");
        printf( "1. TestVmDirSASLClient\n");
#if 0
        printf( "2. TestVmDirGetKrbMasterKey\n");
        printf( "3. TestVmDirGetKrbUPNKey\n");
#endif
        printf( "4. TestVmDirCreateUser\n");
        printf( "5. TestVmDirDBFileTransfer\n");
        printf( "6. TestVmDirReplNow\n");
        printf( "7. TestVmDirForceResetPassword\n");
        printf( "8. TestVmDirSetLogParameters\n");
        printf( "9. TestVmDirCreateUserEx\n");
        printf( "==================\n\n");
        scanf("%d", &choice);

        if (!choice)
        {
            goto cleanup;
        }

        switch (choice)
        {
          case 1:
              TestVmDirSASLClient();
              break;

#if 0
          case 2:
              TestVmDirGetKrbMasterKey();
              break;

          case 3:
              TestVmDirGetKrbUPNKey();
              break;
#endif

          case 4:
              TestVmDirCreateUser();
              break;

          case 5:
              TestVmDirDBFileTransfer();
              break;

          case 6:
              TestVmDirReplNow();
              break;

          case 7:
              TestVmDirForceResetPassword();
              break;

          case 8:
              TestVmDirSetLogParameters();
              break;

          case 9:
              TestVmDirCreateUserEx();
              break;

          default:
              goto cleanup;
        }
    }

cleanup:

    return 0;

}
