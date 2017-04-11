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



typedef enum
{
    DIR_COMMAND_UNKNOWN = 0,
    DIR_COMMAND_SERVICE_CREATE,
    DIR_COMMAND_SERVICE_UPDATE,
    DIR_COMMAND_SERVICE_DELETE,
    DIR_COMMAND_SERVICE_LIST,
    DIR_COMMAND_USER_CREATE,
    DIR_COMMAND_USER_MODIFY,
    DIR_COMMAND_USER_DELETE,
    DIR_COMMAND_USER_FIND_BY_NAME,
    DIR_COMMAND_GROUP_CREATE,
    DIR_COMMAND_GROUP_DELETE,
    DIR_COMMAND_GROUP_MODIFY,
    DIR_COMMAND_GROUP_LIST,
    DIR_COMMAND_CERTIFICATE_PUB_CERT,
    DIR_COMMAND_CERTIFICATE_PUB_CRL,
    DIR_COMMAND_CERTIFICATE_UNPUB_CERT,
    DIR_COMMAND_CERTIFICATE_GET,
    DIR_COMMAND_CERTIFICATE_LIST,
    DIR_COMMAND_PASSWORD_CREATE,
    DIR_COMMAND_PASSWORD_CHANGE,
    DIR_COMMAND_PASSWORD_RESET,
    DIR_COMMAND_FUNCLVL_GET,
    DIR_COMMAND_FUNCLVL_SET,
    DIR_COMMAND_NODES_LIST,
    DIR_COMMAND_COMPUTER_PASSWORD_RESET,
    DIR_COMMAND_STATE_GET,
    DIR_COMMAND_STATE_SET,
    DIR_COMMAND_TENANT_CREATE,
    DIR_COMMAND_TENANT_DELETE,
    DIR_COMMAND_TENANT_LIST,
    DIR_COMMAND_ORGUNIT_CREATE,
    DIR_COMMAND_ORGUNIT_LIST,
} DIR_COMMAND;

typedef enum
{
    SSO_ROLE_UNKNOWN = 0,
    SSO_ROLE_ADMINISTRATOR,
    SSO_ROLE_USER
} SSO_ADMIN_ROLE;

typedef enum
{
    ATTR_NOT_FOUND = 0,
    ATTR_MATCH,
    ATTR_DIFFER,
} ATTR_SEARCH_RESULT;

typedef enum
{
    USER_INFO_LEVEL_DEFAULT = 0,
    USER_INFO_LEVEL_ONE,
    USER_INFO_LEVEL_TWO
} USER_INFO_LEVEL;

typedef enum
{
    USER_MODIFY_NO_OPT        = 1 << 0,
    USER_MODIFY_PWD_NEVER_EXP = 1 << 1,
    USER_MODIFY_PWD_EXPIRE    = 1 << 2,
} USER_MODIFY_OPT;

#define DIR_LOGIN_DEFAULT "administrator"

#define OBJECT_CLASS_SVC_PRINCIPAL "vmwServicePrincipal"
#define OBJECT_CLASS_USER          "user"
#define OBJECT_CLASS_COMPUTER      "computer"
#define OBJECT_CLASS_GROUP         "group"
#define OBJECT_CLASS_ORGANIZATIONAL_UNIT "organizationalUnit"
#define OBJECT_CLASS_TOP           "top"

#define ATTR_NAME_OBJECTCLASS "objectclass"
#define ATTR_NAME_SUBJECT_DN  "vmwSTSSubjectDN"
#define ATTR_NAME_CN          "cn"
#define ATTR_NAME_ACCOUNT     "sAMAccountName"
#define ATTR_NAME_CERT        "userCertificate"
#define ATTR_NAME_MEMBER      "member"
#define ATTR_NAME_DESCRIPTION "description"
#define ATTR_NAME_CA_CERTIFICATE "cACertificate"
#define ATTR_NAME_CA_CRL         "certificateRevocationList"
#define ATTR_NAME_CA_CERTIFICATE_DN "cACertificateDN"
#define ATTR_NAME_ENTRY_DN "entryDN"
#define ATTR_NAME_GROUP       "groupName"
#define ATTR_NAME_UPN         "userPrincipalName"
#define ATTR_NAME_GIVEN_NAME  "givenName"
#define ATTR_NAME_SN          "sn"
#define ATTR_NAME_USER_ACC_CTRL    "userAccountControl"
#define ATTR_NAME_PWD_NEVER_EXPIRE "vmwPasswordNeverExpires"
#define ATTR_NAME_PASS_EXP_IN_DAY  "vmwPasswordLifetimeDays"
#define ATTR_NAME_PWD_LAST_SET     "pwdLastSet"
#define ATTR_NAME_PWD_LOCKOUT_POLICY_CN "password and lockout policy"
#define ATTR_NAME_OU          "ou"

#define USER_ACC_CTRL_DISABLE_FLAG              0x00000002
#define USER_ACC_CTRL_LOCKOUT_FLAG              0x00000010
#define USER_ACC_CTRL_PASSWORD_EXPIRE_FLAG      0x00800000

#define LDAP_BOOLEAN_SYNTAX_TRUE_STR        "TRUE"
#define LDAP_BOOLEAN_SYNTAX_FALSE_STR       "FALSE"
#define SOLUTION_USERS_GROUP_NAME           "SolutionUsers"
#define TRUSTED_USERS_GROUP_NAME            "ActAsUsers"
#define BUILTIN_ADMINISTRATORS_GROUP_NAME   "Administrators"
#define BUILTIN_USERS_GROUP_NAME            "Users"
#define ATTR_USER_PASSWORD                  "userPassword"

#define CA_CN_NAME          "Certificate-Authority"
#define CA_CONTAINER_NAME   "Certificate-Authorities"

#define MAX_CN_LENGTH 64

#define SECS_IN_MINUTE (60)
#define SECS_IN_HOUR (SECS_IN_MINUTE * 60)
#define SECS_IN_DAY (24 * SECS_IN_HOUR)

#define ERROR_LOCAL_BASE                            (100000)
#define ERROR_LOCAL_OPTION_UNKNOWN                  (ERROR_LOCAL_BASE + 1) // This option does not exist
#define ERROR_LOCAL_OPTION_INVALID                  (ERROR_LOCAL_BASE + 2) // The options are not semantically valid
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN        (ERROR_LOCAL_BASE + 3)
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_READ        (ERROR_LOCAL_BASE + 4)
#define ERROR_LOCAL_PASSWORD_EMPTY                  (ERROR_LOCAL_BASE + 5)

#define DirCliIsFlagSet(value, flag) ((value & flag) == (flag))
