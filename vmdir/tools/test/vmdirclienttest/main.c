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
TestVmDirLdapAddGetResults(
    LDAP    *pLd,
    int      msgid
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

/*  StrongConsistentWrite - create a test client capable of sending writes with control*/
int
TestVmDirCreateConsistentWriteControl(
    LDAPControl **ppCtrl
    )
{
   /* criticality of the control is false */
   return ldap_control_create(LDAP_CONTROL_CONSISTENT_WRITE, 0, NULL, 0, ppCtrl);
}

VOID
TestVmDirCreateUserWithControls(
    VOID
    )
{
   int     msgid = 0;
   char    pszServerHost[SIZE_256] = {0};
   char    newdn[SIZE_256] = {0};
   char    adminUPN[SIZE_256] = {0};
   char    pwd[SIZE_256] = {0};
   char    cn_value[SIZE_256] = {0};
   char    sn_value[SIZE_256] = {0};
   PSTR    pszLDAPHostName = NULL;
   PSTR    pszDN = NULL;
   PSTR    pszPwd = NULL;
   PSTR    pszNewDN = NULL;
   DWORD   dwError = 0;
   LDAP    *pLd = NULL;
   LDAPMod attribute = {0};
   LDAPMod attribute1 = {0};
   LDAPMod attribute2 = {0};
   char    *pCn_values[2] = { NULL,
                              NULL };
   char    *pObjectclass_values[] = { "top",
                                      "person",
                                      "organizationalPerson",
                                      "user",
                                      NULL };
   char    *pSn_values[2] = { NULL,
                              NULL };
   LDAPMod *pAttributes[4] = { &attribute,
                               &attribute1,
                               &attribute2,
                               NULL };
   LDAPControl *pCtrl = NULL;
   LDAPControl *pSrvctrl[2] = { NULL,
                                NULL };

   printf("\n hostname: (example: hostname or Ip addr): ");
   scanf("%s",pszServerHost);
   printf("\n admin UPN (example: Administrator@vsphere.local): ");
   scanf("%s",adminUPN);
   printf("\n password: ");
   scanf("%s",pwd);
   printf("\n new  dn (example: cn=newuser,cn=users,dc=vsphere,dc=local): ");
   scanf("%s",newdn);
   printf("\n sn value (example: newuser) ");
   scanf("%s",sn_value);
   printf("\n cn value (example: newuser) ");
   scanf("%s",cn_value);

   if (IsNullOrEmptyString(pszServerHost) ||
       IsNullOrEmptyString(adminUPN) ||
       IsNullOrEmptyString(pwd) ||
       IsNullOrEmptyString(newdn) ||
       IsNullOrEmptyString(sn_value) ||
       IsNullOrEmptyString(cn_value))
   {
      printf("\n Invalid input parameter, empty or null string found ");
      return;
   }

   pSn_values[0] = sn_value;
   pCn_values[0] = cn_value;

   dwError = VmDirAllocateStringAVsnprintf(&pszLDAPHostName,
                         pszServerHost[0] != '\0' ? pszServerHost : "localhost");
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = VmDirAllocateStringAVsnprintf(&pszDN, adminUPN);
   BAIL_ON_VMDIR_ERROR(dwError);
   dwError = VmDirAllocateStringAVsnprintf(&pszPwd, pwd);
   BAIL_ON_VMDIR_ERROR(dwError);

   printf("\n VmDirCreateUserWithControls ldap initiating bind!!!! ");
   dwError = VmDirSafeLDAPBind(&pLd, pszLDAPHostName, pszDN, pszPwd);
   BAIL_ON_VMDIR_ERROR(dwError);
   printf("\n VmDirCreateUserWithControls ldap bind succeeded!!!! ");

   attribute.mod_op = LDAP_MOD_ADD;
   attribute.mod_type = ATTR_CN;
   attribute.mod_values = pCn_values;

   attribute1.mod_op = LDAP_MOD_ADD;
   attribute1.mod_type = ATTR_OBJECT_CLASS;
   attribute1.mod_values = pObjectclass_values;

   attribute2.mod_op = LDAP_MOD_ADD;
   attribute2.mod_type = ATTR_SN;
   attribute2.mod_values = pSn_values;

   dwError = VmDirAllocateStringAVsnprintf(&pszNewDN, newdn);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = TestVmDirCreateConsistentWriteControl(&pCtrl);
   if (dwError != LDAP_SUCCESS  || pCtrl == NULL)
   {
      printf("\n not able to create control !!");
      BAIL_ON_VMDIR_ERROR(dwError);
   }
   pSrvctrl[0] = pCtrl;

   printf("\n ldap_add_ext to add the new entry to the database with controls");
   dwError = ldap_add_ext(pLd, pszNewDN, pAttributes, pSrvctrl, NULL, &msgid);
   BAIL_ON_VMDIR_ERROR(dwError);
   printf("\n Added new user to the database successfully ");

   dwError = TestVmDirLdapAddGetResults(pLd, msgid);
   BAIL_ON_VMDIR_ERROR(dwError);

cleanup:
   VMDIR_SAFE_FREE_MEMORY(pszLDAPHostName);
   VMDIR_SAFE_FREE_MEMORY(pszDN);
   VMDIR_SAFE_FREE_MEMORY(pszPwd);
   VMDIR_SAFE_FREE_MEMORY(pszNewDN);

   if (pLd)
   {
      dwError = ldap_unbind_ext_s(pLd, NULL, NULL);
      BAIL_ON_VMDIR_ERROR(dwError);
      printf(" \n VmDirCreateUserWithControls ldap unbind succeeded ");
   }
   return;

error:
   printf(" \nTestVmDirCreateUserWithControls failed. (%d)\n", dwError);
   goto cleanup;
}

DWORD
TestVmDirLdapAddGetResults(
    LDAP    *pLd,
    int      msgid
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
	    if (time(NULL) - startTime > SECONDS_IN_MINUTE)/* time out */
	    {
               completed = TRUE;
               printf("\n TestVmDirLdapAddGetResults: Not able to obtain result for 60 seconds - Timed out ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
	    }
            break;

         default:
            completed = TRUE;

            if (pResult == NULL)
            {
               printf("\n TestVmDirLdapAddGetResults: pResult is NULL ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
            }

            parse_rc = ldap_parse_result(pLd, pResult, &dwError, NULL, NULL, NULL, &ppServerctrls, 1/*freeit*/);

            if (parse_rc != 0)
            {
               printf("\n TestVmDirLdapAddGetResults: ldap_parse_result failed with status: %d ", parse_rc);
               dwError = parse_rc;
	       BAIL_ON_VMDIR_ERROR(parse_rc);
            }

            BAIL_ON_VMDIR_ERROR(dwError);
	    printf("\n LDAP Add succeeded ");

            if (ppServerctrls[0] == NULL)
            {
               printf("\n TestVmDirLdapAddGetResults: Serverctrls is NULL failed to obtain controls ");
               dwError = -1;
	       BAIL_ON_VMDIR_ERROR(dwError);
            }

            if (VmDirStringCompareA(ppServerctrls[0]->ldctl_oid, LDAP_CONTROL_CONSISTENT_WRITE, TRUE) == 0)
            {
               ber = ber_init(&ppServerctrls[0]->ldctl_value);

               if (ber == NULL)
               {
                  printf("\n TestVmDirLdapAddGetResults: ber_init failed (returned NULL) ");
                  dwError = -1;
	          BAIL_ON_VMDIR_ERROR(dwError);
               }

               if (ber_scanf(ber, "{i}", &status ) == LBER_ERROR)
               {
                  printf("\n TestVmDirLdapAddGetResults: Not able to read status from berElement ");
                  dwError = -1;
	          BAIL_ON_VMDIR_ERROR(dwError);
               }

               printf("\n\n Result: ");
               printf("\n     Received Control OID matches Strong Consistency Write Control as expected ");
               printf("\n     control OID: %s status: %d ", ppServerctrls[0]->ldctl_oid, status);
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
   if (ppServerctrls[0] != NULL)
   {
      ldap_controls_free(ppServerctrls);
   }
   return dwError;

error:
   printf("\n TestVmDirLdapAddGetResults: failed ");
   goto cleanup;
}

/*  StrongConsistentWrite end */

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
        printf( "10. TestVmDirCreateUserWithControls\n");
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

          case 10:
              TestVmDirCreateUserWithControls();
              break;

          default:
              goto cleanup;
        }
    }

cleanup:

    return 0;

}
