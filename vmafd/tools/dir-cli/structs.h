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



typedef struct _DIR_CLI_CERT
{
    PSTR pszSubjectName;
    PSTR pszFingerPrint;

    X509* pX509Cert;

} DIR_CLI_CERT, *PDIR_CLI_CERT;

typedef struct _DIR_CLI_ENUM_SVC_CONTEXT
{
    LDAP* pLd;
    DWORD dwMaxCount;
    LDAPMessage* pSearchRes;
    LDAPMessage* pEntry;
    DWORD dwNumEntries;
    DWORD dwNumRead;
} DIR_CLI_ENUM_SVC_CONTEXT, *PDIR_CLI_ENUM_SVC_CONTEXT;

typedef struct _DIR_CLI_ENUM_GROUP_CONTEXT
{
    LDAP* pLd;
    DWORD dwMaxCount;
    LDAPMessage* pSearchRes;
    LDAPMessage* pEntry;
    DWORD dwNumEntries;
    struct berval** ppValues;
    DWORD dwNumValues;
    DWORD dwNumRead;
} DIR_CLI_ENUM_GROUP_CONTEXT, *PDIR_CLI_ENUM_GROUP_CONTEXT;

typedef struct _DIR_CLI_ENUM_ORGUNIT_CONTEXT
{
    LDAP* pLd;
    DWORD dwMaxCount;
    LDAPMessage* pSearchRes;
    LDAPMessage* pEntry;
    DWORD dwNumEntries;
    DWORD dwNumRead;
} DIR_CLI_ENUM_ORGUNIT_CONTEXT, *PDIR_CLI_ENUM_ORGUNIT_CONTEXT;

typedef struct _DIR_CLI_USER_INFO_0
{
    PSTR pszAccount;
    PSTR pszUPN;

} DIR_CLI_USER_INFO_0, *PDIR_CLI_USER_INFO_0;

typedef struct _DIR_CLI_USER_INFO_1
{
    PSTR pszAccount;
    PSTR pszUPN;
    PSTR pszFirstName;
    PSTR pszLastName;

} DIR_CLI_USER_INFO_1, *PDIR_CLI_USER_INFO_1;

typedef struct _DIR_CLI_USER_INFO_2
{
    PSTR pszAccount;
    PSTR pszUPN;
    PSTR pszPwdLastSet;
    PSTR pszPwdExpTime;
    DWORD dwUserAccCtrl;
    BOOLEAN bIsPwdNeverExpired;

} DIR_CLI_USER_INFO_2, *PDIR_CLI_USER_INFO_2;

typedef struct _DIR_CLI_USER_INFO
{
    union{
        PDIR_CLI_USER_INFO_0 pUserInfo_0;
        PDIR_CLI_USER_INFO_1 pUserInfo_1;
        PDIR_CLI_USER_INFO_2 pUserInfo_2;
    }info;
    DWORD dwInfoLevel;
} DIR_CLI_USER_INFO, *PDIR_CLI_USER_INFO;

