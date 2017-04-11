/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef INCLUDE_PUBLIC_SSOVMDIRCLIENT_H_
#define INCLUDE_PUBLIC_SSOVMDIRCLIENT_H_

// data attributes

typedef enum
{
    VMDIR_MEMBER_TYPE_USER, /* Users */
    VMDIR_MEMBER_TYPE_GROUP, /* Groups */
    VMDIR_MEMBER_TYPE_SOLUTIONUSER, /* Solution Users */
    VMDIR_MEMBER_TYPE_ALL /* All available member types */
} VMDIR_MEMBER_TYPE;

typedef struct
{
    PSTRING* ppEntry;
    size_t length;
} VMDIR_STRING_ARRAY_DATA;

typedef struct
{
    PSTRING description;
} VMDIR_GROUP_DETAILS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
} VMDIR_PRINCIPAL_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    VMDIR_GROUP_DETAILS_DATA* details;
    VMDIR_PRINCIPAL_DATA* alias;
    PSTRING objectId;
} VMDIR_GROUP_DATA;

typedef struct
{
    VMDIR_GROUP_DATA** ppEntry;
    size_t length;
} VMDIR_GROUP_ARRAY_DATA;

typedef struct
{
    PSTRING password;
    SSO_LONG* lastSet;
    SSO_LONG* lifetime;
} VMDIR_PASSWORD_DETAILS_DATA;

typedef struct
{
    PSTRING currentPassword;
    PSTRING newPassword;
} VMDIR_PASSWORD_RESET_REQUEST_DATA;

typedef struct
{
    PSTRING email;
    PSTRING upn;
    PSTRING firstName;
    PSTRING lastName;
    PSTRING description;
} VMDIR_USER_DETAILS_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    VMDIR_PRINCIPAL_DATA* alias;
    VMDIR_USER_DETAILS_DATA* details;
    bool* disabled;
    bool* locked;
    PSTRING objectId;
    VMDIR_PASSWORD_DETAILS_DATA* passwordDetails;
} VMDIR_USER_DATA;

typedef struct
{
    VMDIR_USER_DATA** ppEntry;
    size_t length;
} VMDIR_USER_ARRAY_DATA;

typedef struct
{
    PSTRING name;
    PSTRING domain;
    PSTRING description;
    VMDIR_PRINCIPAL_DATA* alias;
    REST_CERTIFICATE_DATA* certificate;
    bool* disabled;
    PSTRING objectId;
} VMDIR_SOLUTION_USER_DATA;

typedef struct
{
    VMDIR_SOLUTION_USER_DATA** ppEntry;
    size_t length;
} VMDIR_SOLUTION_USER_ARRAY_DATA;

typedef struct
{
    VMDIR_USER_ARRAY_DATA* users;
    VMDIR_GROUP_ARRAY_DATA* groups;
    VMDIR_SOLUTION_USER_ARRAY_DATA* solutionUsers;
} VMDIR_SEARCH_RESULT_DATA;

// Data APIs

SSOERROR
VmdirStringArrayDataNew(
    VMDIR_STRING_ARRAY_DATA** ppStrings,
    PSTRING* ppEntry,
    size_t length);

void
VmdirStringArrayDataDelete(
    VMDIR_STRING_ARRAY_DATA* pStrings);

SSOERROR
VmdirPrincipalDataNew(
    VMDIR_PRINCIPAL_DATA** ppPrincipal,
    PCSTRING name,
    PCSTRING domain);

void
VmdirPrincipalDataDelete(
    VMDIR_PRINCIPAL_DATA* pPrincipal);

SSOERROR
VmdirUserDetailsDataNew(
    VMDIR_USER_DETAILS_DATA** ppUserDetails,
    PCSTRING email,
    PCSTRING upn,
    PCSTRING firstName,
    PCSTRING lastName,
    PCSTRING description);

void
VmdirUserDetailsDataDelete(
    VMDIR_USER_DETAILS_DATA* pUserDetails);

SSOERROR
VmdirPasswordDetailsDataNew(
    VMDIR_PASSWORD_DETAILS_DATA** ppPasswordDetails,
    PCSTRING password,
    const SSO_LONG* lastSet,
    const SSO_LONG* lifetime);

void
VmdirPasswordDetailsDataDelete(
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails);

SSOERROR
VmdirUserDataNew(
    VMDIR_USER_DATA** ppUser,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_PRINCIPAL_DATA* alias,
    const VMDIR_USER_DETAILS_DATA* details,
    const bool* disabled,
    const bool* locked,
    PCSTRING objectId,
    const VMDIR_PASSWORD_DETAILS_DATA* passwordDetails);

void
VmdirUserDataDelete(
    VMDIR_USER_DATA* pUser);

SSOERROR
VmdirGroupDetailsDataNew(
    VMDIR_GROUP_DETAILS_DATA** ppGroupDetails,
    PCSTRING description);

void
VmdirGroupDetailsDataDelete(
    VMDIR_GROUP_DETAILS_DATA* pGroupDetails);

SSOERROR
VmdirGroupDataNew(
    VMDIR_GROUP_DATA** ppGroup,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_GROUP_DETAILS_DATA* details,
    const VMDIR_PRINCIPAL_DATA* alias,
    PCSTRING objectId);

void
VmdirGroupDataDelete(
    VMDIR_GROUP_DATA* pGroup);

SSOERROR
VmdirGroupArrayDataNew(
    VMDIR_GROUP_ARRAY_DATA** ppGroupArray,
    VMDIR_GROUP_DATA** ppEntry,
    size_t length);

void
VmdirGroupArrayDataDelete(
    VMDIR_GROUP_ARRAY_DATA* pGroupArray);

SSOERROR
VmdirPasswordResetRequestDataNew(
    VMDIR_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest,
    PCSTRING currentPassword,
    PCSTRING newPassword);

void
VmdirPasswordResetRequestDataDelete(
    VMDIR_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest);

SSOERROR
VmdirUserArrayDataNew(
    VMDIR_USER_ARRAY_DATA** ppUserArray,
    VMDIR_USER_DATA** ppEntry,
    size_t length);

void
VmdirUserArrayDataDelete(
    VMDIR_USER_ARRAY_DATA* pUserArray);

SSOERROR
VmdirSolutionUserDataNew(
    VMDIR_SOLUTION_USER_DATA** ppSolutionUser,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING description,
    const VMDIR_PRINCIPAL_DATA* alias,
    const REST_CERTIFICATE_DATA* certificate,
    const bool* disabled,
    PCSTRING objectId);

void
VmdirSolutionUserDataDelete(
    VMDIR_SOLUTION_USER_DATA* pSolutionUser);

SSOERROR
VmdirSolutionUserArrayDataNew(
    VMDIR_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray,
    VMDIR_SOLUTION_USER_DATA** ppEntry,
    size_t length);

void
VmdirSolutionUserArrayDataDelete(
    VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray);

SSOERROR
VmdirSearchResultDataNew(
    VMDIR_SEARCH_RESULT_DATA** ppSearchResult,
    const VMDIR_USER_ARRAY_DATA* users,
    const VMDIR_GROUP_ARRAY_DATA* groups,
    const VMDIR_SOLUTION_USER_ARRAY_DATA* solutionUsers);

void
VmdirSearchResultDataDelete(
    VMDIR_SEARCH_RESULT_DATA* pSearchResult);

// Resource APIs

SSOERROR
VmdirGroupCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_GROUP_DATA* pGroup,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_GROUP_DATA* pGroup,
    VMDIR_GROUP_DATA** ppGroupReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupAddMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_STRING_ARRAY_DATA* pMembers,
    VMDIR_MEMBER_TYPE memberType,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupGetMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_MEMBER_TYPE memberType,
    size_t limit,
    VMDIR_SEARCH_RESULT_DATA** ppSearchResultReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupRemoveMembers(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_STRING_ARRAY_DATA* pMembers,
    VMDIR_MEMBER_TYPE memberType,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirGroupGetParents(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirSolutionUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_SOLUTION_USER_DATA* pSolutionUser,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirSolutionUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirSolutionUserUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    const VMDIR_SOLUTION_USER_DATA* pSolutionUser,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirSolutionUserDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserCreate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    const VMDIR_USER_DATA* pUser,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserGet(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserGetGroups(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    bool nested,
    VMDIR_GROUP_ARRAY_DATA** ppGroupArrayReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserUpdate(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    const VMDIR_USER_DATA* pUser,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserUpdatePassword(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING currentPassword,
    PCSTRING newPassword,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserResetPassword(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    PCSTRING newPassword,
    VMDIR_USER_DATA** ppUserReturn,
    REST_SERVER_ERROR** ppError);

SSOERROR
VmdirUserDelete(
    PCREST_CLIENT pClient,
    PCSTRING tenant,
    PCSTRING name,
    PCSTRING domain,
    REST_SERVER_ERROR** ppError);

#endif /* INCLUDE_PUBLIC_SSOVMDIRCLIENT_H_ */
