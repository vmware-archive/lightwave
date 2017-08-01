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

VOID
TestVmDirGetResults(
    LDAP       *pLd,
    int        *pMessageid,
    DWORD       count,
    uint64_t   *pStartTime,
    BOOLEAN     displayTime
    )
{
    DWORD  iter = 0;
    DWORD  dwError = 0;

    for (iter = 0; iter < count; iter++)
    {
        dwError = TestVmDirLdapGetResults(pLd, pMessageid[iter], pStartTime[iter], displayTime);
        if (dwError != 0)
        {
            printf("\n messageid: %d failed", pMessageid[iter]);
        }
    }
}

DWORD
TestVmDirLdapGetResults(
    LDAP    *pLd,
    int      msgid,
    uint64_t ldapOpStartTime,
    BOOLEAN  displayTimeTaken
    )
{
   DWORD            dwError = 0;
   struct timeval   zerotime = {0};
   BOOLEAN          completed = FALSE;
   LDAPMessage      *pResult = NULL;
   LDAPControl      **ppServerctrls = NULL;
   int              parse_rc = 0;
   int              status = 0;
   BerElement       *ber = NULL;
   time_t           startTime = time(NULL);

   /* To indicate that client will be taking polling approach */
   zerotime.tv_sec = zerotime.tv_usec = 0L;

   while (completed == FALSE)
   {
      dwError = ldap_result(pLd, msgid, 0 /*all*/, &zerotime, &pResult);

      switch (dwError)
      {
         case -1:
            completed = TRUE;
            BAIL_ON_VMDIR_ERROR(dwError);
            break;

         case 0:
	    if (time(NULL) - startTime > SECONDS_IN_MINUTE * 3)/* time out */
	    {
               completed = TRUE;
               printf("\n TestVmDirLdapGetResults: Not able to obtain result for 3 mins - Timed out ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
	    }
            break;

         default:
            completed = TRUE;

            if (pResult == NULL)
            {
               printf("\n TestVmDirLdapGetResults: pResult is NULL ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
            }

            parse_rc = ldap_parse_result(pLd, pResult, &dwError, NULL, NULL, NULL, &ppServerctrls, 1/*freeit*/);

            if (parse_rc != 0)
            {
               printf("\n TestVmDirLdapGetResults: ldap_parse_result failed with status: %d ", parse_rc);
               dwError = parse_rc;
	       BAIL_ON_VMDIR_ERROR(parse_rc);
            }

            BAIL_ON_VMDIR_ERROR(dwError);

            if (ppServerctrls == NULL || ppServerctrls[0] == NULL)
            {
               printf("\n TestVmDirLdapGetResults: Serverctrls is NULL failed to obtain controls ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (VmDirStringCompareA(ppServerctrls[0]->ldctl_oid, LDAP_CONTROL_CONSISTENT_WRITE, TRUE) == 0)
            {
               ber = ber_init(&ppServerctrls[0]->ldctl_value);

               if (ber == NULL)
               {
                  printf("\n TestVmDirLdapGetResults: ber_init failed (returned NULL) ");
                  dwError = -1;
	          BAIL_ON_VMDIR_ERROR(dwError);
               }

               if (ber_scanf(ber, "{i}", &status ) == LBER_ERROR)
               {
                  printf("\n TestVmDirLdapGetResults: Not able to read status from berElement ");
                  dwError = -1;
	          BAIL_ON_VMDIR_ERROR(dwError);
               }

               if (displayTimeTaken)
               {
	           printf("\n Success - Time taken: %d milliseconds msg-id: %d with status: %d",
                       (DWORD)(VmDirGetTimeInMilliSec() - ldapOpStartTime), msgid, status);
               }
               else
               {
	           printf("\n Success - msg-id: %d with status: %d", msgid, status);
               }
            }
            else
            {
               printf("\n\n Result: ");
               printf("\n     control OID does not matches the Strong Consistency Write Control - failure ");
               printf("\n     Actual control OID: %s Expected control OID: %s ", ppServerctrls[0]->ldctl_oid, LDAP_CONTROL_CONSISTENT_WRITE);
            }
      }
   }

cleanup:
   if (ppServerctrls && ppServerctrls[0] != NULL)
   {
      ldap_controls_free(ppServerctrls);
   }
   return dwError;

error:
   printf("\n TestVmDirLdapGetResults: failed ");
   goto cleanup;
}

VOID
TestSetupServerInfo(
    PVMDIRCLIENT_TEST_CONTEXT    pCtx
    )
{
    DWORD   dwError = 0;
    CHAR    serverName[SIZE_256] = {0};
    CHAR    domainName[SIZE_256] = {0};
    CHAR    userName[SIZE_256] = {0};
    CHAR    password[SIZE_256] = {0};

    printf("\n host name: (example: hostname or Ip addr): ");
    scanf("%s", serverName);
    printf("\n domain name: ");
    scanf("%s", domainName);
    printf("\n user name: ");
    scanf("%s", userName);
    printf("\n password: ");
    scanf("%s", password);

    dwError = VmDirAllocateStringA(serverName, &pCtx->pszServerName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(domainName, &pCtx->pszDomainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(userName, &pCtx->pszUserName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringA(password, &pCtx->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringPrintf(&pCtx->pszUPN, "%s@%s", userName, domainName);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirSafeLDAPBind(&pCtx->pLd, pCtx->pszServerName, pCtx->pszUPN, pCtx->pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
    return;

error:
    goto cleanup;
}

#ifndef _WIN32
int main(int argc, char* argv[])
#else
int _tmain(int argc, TCHAR *targv[])
#endif
{
    VMDIRCLIENT_TEST_CONTEXT srvCtx = {0};

    while (1)
    {
        int choice = -1;

        printf( "\n\n==================\n");
        printf( "Please select:\n");
        printf( "0. Input setup server information\n");
        printf( "1. TestVmDirSASLClient\n");
        printf( "4. TestVmDirCreateUser\n");
        printf( "5. TestVmDirDBFileTransfer\n");
        printf( "7. TestVmDirForceResetPassword\n");
        printf( "8. TestVmDirSetLogParameters\n");
        printf( "9. TestVmDirCreateUserEx\n");
        printf( "15. TestVmDirCondWriteControl\n");
        printf( "99. exit\n");
        printf( "==================\n\n");
        scanf("%d", &choice);

        switch (choice)
        {
          case 0:
              TestSetupServerInfo(&srvCtx);
              break;

          case 1:
              TestVmDirSASLClient();
              break;

          case 4:
              TestVmDirCreateUser();
              break;

          case 5:
              TestVmDirDBFileTransfer();
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

          case 15:
               TestVmDirCondWriteControl(&srvCtx);
               break;

          default:
              goto cleanup;
        }
    }

cleanup:
    VMDIR_SAFE_FREE_MEMORY(srvCtx.pszServerName);
    VMDIR_SAFE_FREE_MEMORY(srvCtx.pszDomainName);
    VMDIR_SAFE_FREE_MEMORY(srvCtx.pszUPN);
    VMDIR_SAFE_FREE_MEMORY(srvCtx.pszPassword);
    VMDIR_SAFE_FREE_MEMORY(srvCtx.pszUPN);

    if ( srvCtx.pLd )
    {
        ldap_unbind_ext_s(srvCtx.pLd, NULL, NULL);
    }

    return 0;

}
