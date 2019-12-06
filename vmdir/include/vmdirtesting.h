/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

struct _VMDIR_TEST_STATE;
typedef DWORD (*PTEST_SETUP_CALLBACK)(struct _VMDIR_TEST_STATE *pState);
typedef DWORD (*PTEST_RUNNER_CALLBACK)(struct _VMDIR_TEST_STATE *pState);
typedef DWORD (*PTEST_CLEANUP_CALLBACK)(struct _VMDIR_TEST_STATE *pState);

typedef enum
{
    LDAP_OWNER_ADMIN = 0,
    LDAP_OWNER_NORMAL_USER,
    LDAP_OWNER_NORMAL_COMPUTER,
    LDAP_OWNER_ANONYMOUS,
    LDAP_OWNER_CUSTOM
} TEST_LDAP_CONNECTION_OWNER;

typedef struct _VMDIR_TEST_STATE
{
    //
    // Admin connection to server.
    //
    LDAP *pLd;
    LDAP *pSecondLd;

    PCSTR   pszAdminAccessToken;

    //
    // Connection to server using a non-admin account.
    //
    LDAP *pLdLimited;

    //
    // Anonymous connection to the server.
    //
    LDAP *pLdAnonymous;

    //
    // Customizable connection to the server.
    //
    LDAP *pLdCustom;

    //
    // Connection to server using a computer account.
    //
    LDAP *pLdComputer;

    //
    // The test runner's cleanup callback. We'll call this when an assertion
    // fails and we're going to exit() the process.
    //
    PTEST_CLEANUP_CALLBACK  pfnCleanupCallback;

    //
    // Per test context
    //
    PVOID  pContext;

    PCSTR pszSTSServerName;     // The sts server name

    PCSTR pszServerName;        // The server name
    PCSTR pszSecondServerName;  // The second LW server name

    PCSTR pszUserUPN;           // UserUPN to connect with.
    PCSTR pszUserDN;            // User DN to connect with.
    PCSTR pszUserName;          // User name to connect with.

    PCSTR pszPassword;          // Password to connect with.
    PCSTR pszDomain;            // The domain to use (e.g., vsphere.local)
    PCSTR pszBaseDN;            // The domain's DN.

    PCSTR pszTest;              // The name of a particular test to run or a directory to load tests from.

    PCSTR pszTestContainerName; // The name of the test container; all objects should be created beneath this.
    PCSTR pszInternalUserName;  // The name of the internal user we create for operations that shouldn't be run as admin.
    PCSTR pszComputerName;      // The name of the computer we create.

    BOOLEAN bKeepGoing;         // Keep going if an individual test fails.
    BOOLEAN bBreakIntoDebugger; // Break into the debugger when a test fails.
    BOOLEAN bRemoteOnly;        // skip IPC test cases
    BOOLEAN bSkipCleanup;       // skip test cleanup, so we can verify test are replicated correctly
} VMDIR_TEST_STATE, *PVMDIR_TEST_STATE;

DWORD
VmDirTestOidcTokenAcquire(
    PCSTR       pszSSOServer,
    DWORD       dwSSOPort,
    PVMDIR_OIDC_ACQUIRE_TOKEN_INFO pTokenInfo,
    PSTR*       ppszToken
    );

VOID
VmDirTestLdapUnbind(
    LDAP *pLd
    );

DWORD
VmDirTestCreateAnonymousConnection(
    PCSTR pszServerName,
    LDAP **ppLd
    );

PCSTR
VmDirTestGetTestContainerCn(
    PVMDIR_TEST_STATE pState
    );

PCSTR
VmDirTestGetInternalUserCn(
    PVMDIR_TEST_STATE pState
    );

PCSTR
VmDirTestGetComputerCn(
    PVMDIR_TEST_STATE pState
    );

DWORD
VmDirTestDeleteUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName
    );

DWORD
VmDirTestReplaceBinaryAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    BYTE *pbAttributeValue,
    DWORD dwDataLength
    );

DWORD
VmDirTestReplaceAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
VmDirTestAddAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
VmDirTestDeleteAttributeValues(
    LDAP *pLd,
    PCSTR pszDN,
    PCSTR pszAttribute,
    PCSTR *ppszAttributeValues
    );

DWORD
VmDirTestGetEntryAttributeValuesInStr(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    PVMDIR_STRING_LIST* ppList
    );

DWORD
VmDirTestGetAttributeValueString(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    PSTR *ppszAttributeValue
    );

DWORD
VmDirTestGetAttributeValue(
    LDAP *pLd,
    PCSTR pBase,
    int ldapScope,
    PCSTR pszFilter,
    PCSTR pszAttribute,
    BYTE **ppbAttributeValue,
    PDWORD pdwAttributeLength
    );

DWORD
VmDirTestGetObjectList(
    LDAP*               pLd,
    PCSTR               pszDn,
    PCSTR               pszFilter,      /* OPTIONAL */
    PCSTR               pszAttr,        /* OPTIONAL */
    PVMDIR_STRING_LIST* ppObjectList    /* OPTIONAL */
    );

VOID
VmDirTestReportAssertionFailure(
    PCSTR pszExpression,
    PCSTR pszCustomMsg,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    );

VOID
VmDirTestReportAssertionFailureDwordBetweenOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    PCSTR pszSideC,
    DWORD dwValueA,
    DWORD dwValueB,
    DWORD dwValueC,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    );

VOID
VmDirTestReportAssertionFailureDwordOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    DWORD dwValueA,
    DWORD dwValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    );

VOID
VmDirTestReportAssertionFailurePtrOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    PVOID pValueA,
    PVOID pValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    );

VOID
VmDirTestReportAssertionFailureStringOperands(
    PCSTR pszSideA,
    PCSTR pszSideB,
    PCSTR pszValueA,
    PCSTR pszValueB,
    BOOLEAN bEquality,
    PCSTR pszFile,
    PCSTR pszFunction,
    DWORD dwLineNumber,
    PVMDIR_TEST_STATE pState
    );

DWORD
VmDirTestGetInternalUserDn(
    PVMDIR_TEST_STATE pState,
    PSTR *ppszDn
    );

DWORD
VmDirTestCreateContainer(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszContainerDN, /* OPTIONAL */
    PCSTR pszAcl /* OPTIONAL */
    );

DWORD
VmDirTestDeleteContainer(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer
    );

DWORD
VmDirTestCreateComputer(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszComputerName
    );

DWORD
VmDirTestCreateUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PCSTR pszAcl /* OPTIONAL */
    );

DWORD
VmDirTestCreateUserEx(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,     /* OPTIONAL */
    PCSTR pszUserName,
    PCSTR pszUserPassword,  /* OPTIONAL */
    PCSTR pszAcl,           /* OPTIONAL */
    PSTR*   ppszUserDN
    );

DWORD
VmDirTestAddUserToGroupByDn(
    LDAP *pLd,
    PCSTR pszUserDn,
    PCSTR pszGroupDn
    );

DWORD
VmDirTestAddUserToGroup(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszUserName,
    PCSTR               pszUserContainer, // optional
    PCSTR               pszGroupName,
    PCSTR               pszGroupContainer // optional
    );

DWORD
VmDirTestRemoveUserFromGroupByDn(
    LDAP *pLd,
    PCSTR pszUserDn,
    PCSTR pszGroupDn
    );

DWORD
VmDirTestRemoveUserFromGroup(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszUserName,
    PCSTR               pszUserContainer, // optional
    PCSTR               pszGroupName,
    PCSTR               pszGroupContainer // optional
    );

DWORD
VmDirTestDeleteUserEx(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    BOOLEAN bUseLimitedAccount
    );

DWORD
VmDirTestGetUserSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserName,
    PCSTR pszUserContainer, // optional
    PSTR *ppszUserSid
    );

LDAP*
VmDirTestGetLdapOwner(
    PVMDIR_TEST_STATE           pState,
    TEST_LDAP_CONNECTION_OWNER  owner
    );

DWORD
VmDirTestGetTestContainerDn(
    PVMDIR_TEST_STATE pState,
    PSTR *ppszDN
    );

DWORD
VmDirTestGetDomainSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszDomainDn,
    PSTR *ppszDomainSid
    );

DWORD
VmDirTestCreateUserWithSecurityDescriptor(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PBYTE pbSecurityDescriptor,
    DWORD dwLength
    );

DWORD
VmDirTestCreateUserWithLimitedAccount(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName,
    PCSTR pszAcl /* OPTIONAL */
    );

DWORD
VmDirTestConnectionFromUser(
    PVMDIR_TEST_STATE pState,
    PCSTR pszUserName,
    LDAP **ppLd
    );

DWORD
VmDirTestConnectionUser(
    PCSTR   pszHost,
    PCSTR   pszDomain,
    PCSTR   pszUserName,
    PCSTR   pszUserPassword,
    LDAP**  ppLd
    );

DWORD
VmDirTestCreateGroup(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszGroupName,
    PCSTR pszAcl /* OPTIONAL */
    );

DWORD
VmDirTestDeleteGroupEx(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszGroup,
    BOOLEAN bUseLimitedAccount
    );

DWORD
VmDirTestDeleteGroup(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszUserName
    );

DWORD
VmDirTestGetGroupSid(
    PVMDIR_TEST_STATE pState,
    PCSTR pszGroupName,
    PCSTR pszContainer, // optional
    PSTR *ppszGroupSid
    );

DWORD
VmDirTestListUsersGroups(
    LDAP *pLd,
    PCSTR pszUserDn,
    PVMDIR_STRING_LIST *ppvsGroups /* OUT */
    );

DWORD
VmDirTestListGroupMembers(
    LDAP *pLd,
    PCSTR pszUserDn,
    PVMDIR_STRING_LIST *ppvsMembers/* OUT */
    );

DWORD
VmDirTestCreateClass(
    PVMDIR_TEST_STATE pState,
    PCSTR pszClassName
    );

DWORD
VmDirTestCreateObject(
    PVMDIR_TEST_STATE pState,
    PCSTR pszContainer,
    PCSTR pszClassName,
    PCSTR pszObjectName
    );

DWORD
VmDirTestCreateObjectByDNPrefix(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszDNPrefix,
    PCSTR               pszClassName
    );

DWORD
VmDirTestDeleteObjectByDNPrefix(
    PVMDIR_TEST_STATE   pState,
    PCSTR               pszDNPrefix
    );

DWORD
VmDirTestDeleteContainerByDn(
    LDAP *pLd,
    PCSTR pszContainerDn
    );

DWORD
VmDirTestGetGuid(
    PSTR *ppszGuid
    );

DWORD
VmDirTestCreateSimpleUser(
    LDAP *pLd,
    PCSTR pszCN,
    PCSTR pszUserDN
    );

DWORD
VmDirTestCreateSimpgleContainer(
    LDAP *pLd,
    PCSTR pszCN,
    PCSTR pszContainerDN
    );

BOOLEAN
VmDirTestCanReadSingleEntry(
    LDAP* pLd,
    PCSTR pszBaseDn
    );

DWORD
VmDirTestCreateSimpleContainer(
    LDAP *pLd,
    PCSTR pszCN,
    PCSTR pszContainerDN
    );

BOOLEAN
VmDirStringListContainsEx(
    PVMDIR_STRING_LIST pvsList,
    PCSTR pszString,
    BOOLEAN bCaseSensitive
    );

BOOLEAN
VmDirStringListEqualsNoOrder(
    PVMDIR_STRING_LIST pStringListLHS,
    PVMDIR_STRING_LIST pStringListRHS,
    BOOLEAN bCaseSensitive
    );

#define TestAssertBetween(a, b, c) if ((a < c && (a >= b || c <= b)) || \
                                       (a > c && (c >= b || a <= b)) || \
                                       (a == c)) \
        { VmDirTestReportAssertionFailureDwordBetweenOperands(#a, #b, #c, a, b, c, __FILE__, __FUNCTION__, __LINE__, pState); }

#define TestAssertEquals(a, b) if (a != b) { VmDirTestReportAssertionFailureDwordOperands(#a, #b, a, b, TRUE, __FILE__, __FUNCTION__, __LINE__, pState); }
#define TestAssertNotEquals(a, b) if (a == b) { VmDirTestReportAssertionFailureDwordOperands(#a, #b, a, b, FALSE, __FILE__, __FUNCTION__, __LINE__, pState); }

#define TestAssertPtrEquals(a, b) if (a != b) { VmDirTestReportAssertionFailurePtrOperands(#a, #b, a, b, TRUE, __FILE__, __FUNCTION__, __LINE__, pState); }
#define TestAssertPtrNotEquals(a, b) if (a == b) { VmDirTestReportAssertionFailurePtrOperands(#a, #b, a, b, FALSE, __FILE__, __FUNCTION__, __LINE__, pState); }

#define TestAssertStrEquals(a, b) if (strcmp(a, b) != 0) { VmDirTestReportAssertionFailureStringOperands(#a, #b, a, b, TRUE, __FILE__, __FUNCTION__, __LINE__, pState); }
#define TestAssertStrNotEquals(a, b) if (strcmp(a, b) == 0) { VmDirTestReportAssertionFailureStringOperands(#a, #b, a, b, FALSE, __FILE__, __FUNCTION__, __LINE__, pState); }

#define TestAssertStrIEquals(a, b) if (VmDirStringCompare(a, b, TRUE) != 0) { VmDirTestReportAssertionFailureStringOperands(#a, #b, a, b, TRUE, __FILE__, __FUNCTION__, __LINE__, pState); }
#define TestAssertStrINotEquals(a, b) if (VmDirStringCompare(a, b, TRUE) == 0) { VmDirTestReportAssertionFailureStringOperands(#a, #b, a, b, FALSE, __FILE__, __FUNCTION__, __LINE__, pState); }

#define TestAssert(expr) if (!(expr)) { VmDirTestReportAssertionFailure(#expr, "", __FILE__, __FUNCTION__, __LINE__, pState); }
#define TestAssertMsg(expr, msg) if (!(expr)) { VmDirTestReportAssertionFailure(#expr, msg, __FILE__, __FUNCTION__, __LINE__, pState); }
