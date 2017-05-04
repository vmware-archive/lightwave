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

static
DWORD
_TestVmDirCreateThread(
    VmDirStartRoutine* pStartRoutine,
    DWORD startVal,
    PVMDIR_THREAD *ppTID
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

    VmDirAllocateStringAVsnprintf(&pUserCount, "%d", value);

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

    VmDirAllocateStringAVsnprintf(&pUserCount, "%d", value);

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

VOID
TestVmDirCreateUserWithControls(
    DWORD  usrCount,
    DWORD  startVal,
    BOOLEAN displayTime
    )
{
   int     msgid = 0;
   char    pszServerHost[SIZE_256] = {0};
   char    newdn[SIZE_256] = {0};
   char    adminUPN[SIZE_256] = {0};
   char    pwd[SIZE_256] = {0};
   char    cn_value[SIZE_256] = {0};
   char    sn_value[SIZE_256] = {0};
   DWORD   dwError = 0;
   DWORD   value = 0;
   DWORD   count = 0;
   int     messageid[SIZE_256] = {0};
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
   uint64_t  startTime[SIZE_256] = {0};

   if (usrCount == 0)
   {
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
   }
   else
   {
       dwError = VmDirStringCpyA(pszServerHost, VmDirStringLenA("localhost")+1, "localhost");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(adminUPN, VmDirStringLenA("Administrator@vsphere.local")+1, "Administrator@vsphere.local");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(pwd, VmDirStringLenA("Admin!23")+1, "Admin!23");
       BAIL_ON_VMDIR_ERROR(dwError);

       value = startVal;
       dwError = TestVmDirGenerateNewUserAttributes(newdn, sn_value, cn_value, value);
       BAIL_ON_VMDIR_ERROR(dwError);
   }

   dwError = VmDirSafeLDAPBind(&pLd, pszServerHost, adminUPN, pwd);
   BAIL_ON_VMDIR_ERROR(dwError);

   dwError = TestVmDirCreateConsistentWriteControl(&pCtrl);
   if (dwError != LDAP_SUCCESS  || pCtrl == NULL)
   {
       printf("\n not able to create control !!");
       BAIL_ON_VMDIR_ERROR(dwError);
   }
   pSrvctrl[0] = pCtrl;

   do
   {
       pSn_values[0] = sn_value;
       pCn_values[0] = cn_value;

       attribute.mod_op = LDAP_MOD_ADD;
       attribute.mod_type = ATTR_CN;
       attribute.mod_values = pCn_values;

       attribute1.mod_op = LDAP_MOD_ADD;
       attribute1.mod_type = ATTR_OBJECT_CLASS;
       attribute1.mod_values = pObjectclass_values;

       attribute2.mod_op = LDAP_MOD_ADD;
       attribute2.mod_type = ATTR_SN;
       attribute2.mod_values = pSn_values;

       startTime[count] = VmDirGetTimeInMilliSec();
       dwError = ldap_add_ext(pLd, newdn, pAttributes, pSrvctrl, NULL, &msgid);
       BAIL_ON_VMDIR_ERROR(dwError);
       printf("\n\n ldap_add_ext to add the new entry: %s corresponding messageid: %d", newdn, msgid);

       value++;
       dwError = TestVmDirGenerateNewUserAttributes(newdn, sn_value, cn_value, value);
       BAIL_ON_VMDIR_ERROR(dwError);

       messageid[count] = msgid;
       count++;
   }while (count < usrCount);

   TestVmDirGetResults(pLd, messageid, count, startTime, displayTime);

cleanup:

   if (pLd)
   {
      dwError = ldap_unbind_ext_s(pLd, NULL, NULL);
      BAIL_ON_VMDIR_ERROR(dwError);
   }
   return;

error:
   printf(" \nTestVmDirCreateUserWithControls failed. (%d)\n", dwError);
   goto cleanup;
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
TestVmDirModifyUserWithControls(
    DWORD  usrCount,
    DWORD  startVal,
    BOOLEAN displayTime
    )
{
    int     msgid = 0;
    char    serverName[SIZE_256] = {0};
    char    modifydn[SIZE_256] = {0};
    char    adminUPN[SIZE_256] = {0};
    char    pwd[SIZE_256] = {0};
    char    attrName[SIZE_256] = {0};
    char    newValue[SIZE_256] = {0};
    DWORD   messageid[SIZE_256] = {0};
    DWORD   dwError = 0;
    DWORD   count = 0;
    DWORD   val = 0;
    LDAP    *pLd = NULL;
    char    *pvalues[2] = { newValue,
                            NULL };
    LDAPMod attribute = {0};
    LDAPMod *pAttributes[2] = { &attribute,
                                NULL };
    LDAPControl *pCtrl = NULL;
    LDAPControl *pSrvctrl[2] = { NULL,
                                 NULL };
   uint64_t  startTime[SIZE_256] = {0};


    if (usrCount == 0)
    {
        printf("\n hostname: (example: hostname or Ip addr): ");
        scanf("%s", serverName);
        printf("\n admin UPN (example: Administrator@vsphere.local): ");
        scanf("%s", adminUPN);
        printf("\n password: ");
        scanf("%s", pwd);
        printf("\n modify dn (example: cn=newuser,cn=users,dc=vsphere,dc=local): ");
        scanf("%s", modifydn);
        printf("\n attribute name (example: cn) ");
        scanf("%s", attrName);
        printf("\n value (example: newuser) ");
        scanf("%s", newValue);

        if (IsNullOrEmptyString(serverName) ||
            IsNullOrEmptyString(adminUPN) ||
            IsNullOrEmptyString(pwd) ||
            IsNullOrEmptyString(modifydn) ||
            IsNullOrEmptyString(attrName) ||
            IsNullOrEmptyString(newValue))
        {
            printf("\n Invalid input parameter, empty or null string found ");
            return;
        }
    }
    else
    {
       dwError = VmDirStringCpyA(serverName, VmDirStringLenA("localhost")+1, "localhost");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(adminUPN, VmDirStringLenA("Administrator@vsphere.local")+1, "Administrator@vsphere.local");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(pwd, VmDirStringLenA("Admin!23")+1, "Admin!23");
       BAIL_ON_VMDIR_ERROR(dwError);

       val = startVal;
       dwError = TestVmDirGenerateNewUserAttributes(modifydn, NULL, NULL, val);
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(attrName, VmDirStringLenA("cn")+1, "cn");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = TestVmDirGenerateModifyCN(newValue, val);
       BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSafeLDAPBind(&pLd, serverName, adminUPN, pwd);
    BAIL_ON_VMDIR_ERROR(dwError);

    attribute.mod_op = LDAP_MOD_REPLACE;
    attribute.mod_type = attrName;
    attribute.mod_values = pvalues;

    dwError = TestVmDirCreateConsistentWriteControl(&pCtrl);
    if (dwError != LDAP_SUCCESS  || pCtrl == NULL)
    {
       printf("\n not able to create control !!");
       BAIL_ON_VMDIR_ERROR(dwError);
    }
    pSrvctrl[0] = pCtrl;

    do
    {
        startTime[count] = VmDirGetTimeInMilliSec();
        dwError = ldap_modify_ext(pLd, modifydn, pAttributes, pSrvctrl, NULL, &msgid);
        BAIL_ON_VMDIR_ERROR(dwError);
        printf("\n\n ldap_modify_ext to modify entry: %s corresponding message id: %d", modifydn, msgid);

        messageid[count] = msgid;
        count++;

        val++;
        dwError = TestVmDirGenerateNewUserAttributes(modifydn, NULL, NULL, val);
        BAIL_ON_VMDIR_ERROR(dwError);

        dwError = TestVmDirGenerateModifyCN(newValue, val);
        BAIL_ON_VMDIR_ERROR(dwError);

    }while (count < usrCount);

    TestVmDirGetResults(pLd, messageid, count, startTime, displayTime);

cleanup:
    if (pLd)
    {
        dwError = ldap_unbind_ext_s(pLd, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    return;

error:
    printf(" \nTestVmDirModifyUserWithControls failed. (%d)\n", dwError);
    goto cleanup;
}

VOID
TestVmDirDeleteUserWithControls(
    DWORD  usrCount,
    DWORD  startVal,
    BOOLEAN displayTime
    )
{
    int     msgid = 0;
    char    serverName[SIZE_256] = {0};
    char    deleteDN[SIZE_256] = {0};
    char    adminUPN[SIZE_256] = {0};
    char    pwd[SIZE_256] = {0};
    int     messageid[SIZE_256] = {0};
    DWORD   dwError = 0;
    DWORD   count = 0;
    DWORD   value = 0;
    LDAP    *pLd = NULL;
    LDAPControl *pCtrl = NULL;
    LDAPControl *pSrvctrl[2] = { NULL,
                                 NULL };
    uint64_t  startTime[SIZE_256] = {0};

    if (usrCount == 0)
    {
        printf("\n hostname: (example: hostname or Ip addr): ");
        scanf("%s", serverName);
        printf("\n admin UPN (example: Administrator@vsphere.local): ");
        scanf("%s", adminUPN);
        printf("\n password: ");
        scanf("%s", pwd);
        printf("\n delete dn (example: cn=newuser,cn=users,dc=vsphere,dc=local): ");
        scanf("%s", deleteDN);

        if (IsNullOrEmptyString(serverName) ||
            IsNullOrEmptyString(adminUPN) ||
            IsNullOrEmptyString(pwd) ||
            IsNullOrEmptyString(deleteDN))
        {
            printf("\n Invalid input parameter, empty or null string found ");
            return;
        }
    }
    else
    {
       dwError = VmDirStringCpyA(serverName, VmDirStringLenA("localhost")+1, "localhost");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(adminUPN, VmDirStringLenA("Administrator@vsphere.local")+1, "Administrator@vsphere.local");
       BAIL_ON_VMDIR_ERROR(dwError);

       dwError = VmDirStringCpyA(pwd, VmDirStringLenA("Admin!23")+1, "Admin!23");
       BAIL_ON_VMDIR_ERROR(dwError);

       value = startVal;
       dwError = TestVmDirGenerateNewUserAttributes(deleteDN, NULL, NULL, value);
       BAIL_ON_VMDIR_ERROR(dwError);
    }

    dwError = VmDirSafeLDAPBind(&pLd, serverName, adminUPN, pwd);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = TestVmDirCreateConsistentWriteControl(&pCtrl);
    if (dwError != LDAP_SUCCESS  || pCtrl == NULL)
    {
       printf("\n not able to create control !!");
       BAIL_ON_VMDIR_ERROR(dwError);
    }
    pSrvctrl[0] = pCtrl;

    do
    {
        startTime[count] = VmDirGetTimeInMilliSec();
        dwError = ldap_delete_ext(pLd, deleteDN, pSrvctrl, NULL, &msgid);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("\n\n ldap_delete_ext to delete entry: %s corresponding messageid: %d", deleteDN, msgid);
        messageid[count] = msgid;
        count++;

        value++;
        dwError = TestVmDirGenerateNewUserAttributes(deleteDN, NULL, NULL, value);
        BAIL_ON_VMDIR_ERROR(dwError);
    }while (count < usrCount);

    TestVmDirGetResults(pLd, messageid, count, startTime, displayTime);

cleanup:
    if (pLd)
    {
        dwError = ldap_unbind_ext_s(pLd, NULL, NULL);
        BAIL_ON_VMDIR_ERROR(dwError);
    }
    return;

error:
    printf(" \nTestVmDirDeleteUserWithControls failed. (%d)\n", dwError);
    goto cleanup;
}

VOID
TestVmDirStrongConsistencyOperations(
    VOID
    )
{
    char   operation[SIZE_256] = {0};
    DWORD  userCount = 0;
    DWORD  startVal = 0;

    printf("\n Ldap Operation: (ADD|MODIFY|DELETE): ");
    scanf("%s", operation);
    printf("\n Number of users: ");
    scanf("%d", &userCount);
    printf("\n startVal: (startVal is 100 auto generated user will start from newuser100): ");
    scanf("%d", &startVal);

    if (userCount < 0 || startVal < 0)
    {
        printf("\n Invalid Input parameters");
        return;
    }
    else if (userCount > 256)
    {
        printf("\n Maximum of only 256 entries can be concurrently manipulated by this tool, resetting userCount to 256");
        userCount = 256;
    }

    if (VmDirStringCompareA(operation, "ADD", TRUE) == 0)
    {
        TestVmDirCreateUserWithControls(userCount, startVal, FALSE);
    }
    else if (VmDirStringCompareA(operation, "MODIFY", TRUE) == 0)
    {
        TestVmDirModifyUserWithControls(userCount, startVal, FALSE);
    }
    else if (VmDirStringCompareA(operation, "DELETE", TRUE) == 0)
    {
        TestVmDirDeleteUserWithControls(userCount, startVal, FALSE);
    }

    return;
}

DWORD
TestVmDirCreateUserWithControlsThreadFun(
    PVOID  pStartVal
    )
{
    DWORD  dwStartValue = 0;
    DWORD  dwError = 0;

    if (pStartVal != NULL)
    {
        dwStartValue = *(PDWORD)pStartVal;
    }

    TestVmDirCreateUserWithControls(
        1,//userCount
        dwStartValue,
        TRUE
        );

    VMDIR_SAFE_FREE_MEMORY(pStartVal);

    return dwError;
}

DWORD
TestVmDirModifyUserWithControlsThreadFun(
    PVOID  pStartVal
    )
{
    DWORD  dwStartValue = 0;
    DWORD  dwError = 0;

    if (pStartVal != NULL)
    {
        dwStartValue = *(PDWORD)pStartVal;
    }

    TestVmDirModifyUserWithControls(
        1,//userCount
        dwStartValue,
        TRUE
        );

    VMDIR_SAFE_FREE_MEMORY(pStartVal);

    return dwError;
}

DWORD
TestVmDirDeleteUserWithControlsThreadFun(
    PVOID  pStartVal
    )
{
    DWORD  dwStartValue = 0;
    DWORD  dwError = 0;

    if (pStartVal != NULL)
    {
        dwStartValue = *(PDWORD)pStartVal;
    }

    TestVmDirDeleteUserWithControls(
        1,//userCount
        dwStartValue,
        TRUE
        );

    VMDIR_SAFE_FREE_MEMORY(pStartVal);

    return dwError;
}

VOID
TestVmDirConcurrentStrongConsistencyOperations(
    VOID
    )
{
    char   operation[SIZE_256] = {0};
    DWORD  userCount = 0;
    DWORD  count = 0;
    DWORD  startVal = 0;
    DWORD  dwError = 0;
    PVMDIR_THREAD  pTID[10] = {0};

    printf("\n Ldap Operation: (ADD|MODIFY|DELETE): ");
    scanf("%s", operation);
    printf("\n Number of users: ");
    scanf("%d", &userCount);
    printf("\n startVal: (startVal is 100 auto generated user will start from newuser100): ");
    scanf("%d", &startVal);

    if (userCount < 0 || startVal < 0)
    {
        printf("\n Invalid Input parameters");
        return;
    }
    else if (userCount > 10)
    {
        printf("\n Maximum of only 10 entries can be concurrently manipulated by this tool, resetting userCount to 10");
        userCount = 10;
    }

    for (count = 0; count < userCount; count++,startVal++)
    {
        if (VmDirStringCompareA(operation, "ADD", TRUE) == 0)
        {
            dwError = _TestVmDirCreateThread(
                          TestVmDirCreateUserWithControlsThreadFun,
                          startVal,
                          &pTID[count]
                          );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(operation, "MODIFY", TRUE) == 0)
        {
            dwError = _TestVmDirCreateThread(
                          TestVmDirModifyUserWithControlsThreadFun,
                          startVal,
                          &pTID[count]
                          );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
        else if (VmDirStringCompareA(operation, "DELETE", TRUE) == 0)
        {
            dwError = _TestVmDirCreateThread(
                          TestVmDirDeleteUserWithControlsThreadFun,
                          startVal,
                          &pTID[count]
                          );
            BAIL_ON_VMDIR_ERROR(dwError);
        }
    }

    for (count = 0; count < userCount; count++)
    {
        VmDirThreadJoin(pTID[count], NULL);
    }

cleanup:
    for (count = 0; count < userCount; count++)
    {
        VMDIR_SAFE_FREE_MEMORY(pTID[count]);
    }
    return;

error:
    printf("\n TestVmDirConcurrentStrongConsistencyOperation: failed with error: %d", dwError);
    goto cleanup;
}

static
DWORD
_TestVmDirCreateThread(
    VmDirStartRoutine* pStartRoutine,
    DWORD dwStartVal,
    PVMDIR_THREAD *ppTID
    )
{
    DWORD   dwError = ERROR_SUCCESS;
    PDWORD  pdwStartVal = NULL;
    PVMDIR_THREAD pTid = NULL;

    // pTid will be freed by the caller
    dwError = VmDirAllocateMemory(sizeof(VMDIR_THREAD), (PVOID)&pTid);
    BAIL_ON_VMDIR_ERROR(dwError);

    // pdwStartVal will be freed by the newly created thread
    dwError = VmDirAllocateMemory(sizeof(DWORD), (PVOID)&pdwStartVal);
    BAIL_ON_VMDIR_ERROR(dwError);
    *pdwStartVal = dwStartVal;

   //create and start the thread
   dwError = VmDirCreateThread(pTid, TRUE, pStartRoutine, pdwStartVal);
   BAIL_ON_VMDIR_ERROR(dwError);

   *ppTID = pTid;

cleanup:
    return dwError;

error:
    VMDIR_SAFE_FREE_MEMORY(pdwStartVal);
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
        printf( "11. TestVmDirModifyUserWithControls\n");
        printf( "12. TestVmDirDeleteUserWithControls\n");
        printf( "13. TestVmDirStrongConsistencyOperations\n");
        printf( "14. TestVmDirConcurrentStrongConsistencyOperation\n");
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
              TestVmDirCreateUserWithControls(
                  0,//userCount
                  0,//startValue
                  FALSE//displayTime
                  );
              break;

          case 11:
              TestVmDirModifyUserWithControls(
                  0,//userCount
                  0,//startValue
                  FALSE//displayTime
                  );
              break;

          case 12:
              TestVmDirDeleteUserWithControls(
                  0,//userCount
                  0,//startValue
                  FALSE//displayTime
                  );
              break;

          case 13:
               TestVmDirStrongConsistencyOperations();
               break;

          case 14:
               TestVmDirConcurrentStrongConsistencyOperations();
               break;

          default:
              goto cleanup;
        }
    }

cleanup:

    return 0;

}
