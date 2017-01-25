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

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

// data object to/from Json

SSOERROR
VmdirStringArrayDataToJson(
    const VMDIR_STRING_ARRAY_DATA* pStrings,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToStringArrayData(
    PCSSO_JSON pJson,
    VMDIR_STRING_ARRAY_DATA** ppStrings);

SSOERROR
VmdirPrincipalDataToJson(
    const VMDIR_PRINCIPAL_DATA* pPrincipal,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToPrincipalData(
    PCSSO_JSON pJson,
    VMDIR_PRINCIPAL_DATA** ppPrincipal);

SSOERROR
VmdirUserDetailsDataToJson(
    const VMDIR_USER_DETAILS_DATA* pUserDetails,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToUserDetailsData(
    PCSSO_JSON pJson,
    VMDIR_USER_DETAILS_DATA** ppUserDetails);

SSOERROR
VmdirPasswordDetailsDataToJson(
    const VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToPasswordDetailsData(
    PCSSO_JSON pJson,
    VMDIR_PASSWORD_DETAILS_DATA** ppPasswordDetails);

SSOERROR
VmdirUserDataToJson(
    const VMDIR_USER_DATA* pUser,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToUserData(
    PCSSO_JSON pJson,
    VMDIR_USER_DATA** ppUser);

SSOERROR
VmdirGroupDetailsDataToJson(
    const VMDIR_GROUP_DETAILS_DATA* pGroupDetails,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToGroupDetailsData(
    PCSSO_JSON pJson,
    VMDIR_GROUP_DETAILS_DATA** ppGroupDetails);

SSOERROR
VmdirGroupDataToJson(
    const VMDIR_GROUP_DATA* pGroup,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToGroupData(
    PCSSO_JSON pJson,
    VMDIR_GROUP_DATA** ppGroup);

SSOERROR
VmdirGroupArrayDataToJson(
    const VMDIR_GROUP_ARRAY_DATA* pGroupArray,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToGroupArrayData(
    PCSSO_JSON pJson,
    VMDIR_GROUP_ARRAY_DATA** ppGroupArray);

SSOERROR
VmdirPasswordResetRequestDataToJson(
    const VMDIR_PASSWORD_RESET_REQUEST_DATA* pPasswordResetRequest,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToPasswordResetRequestData(
    PCSSO_JSON pJson,
    VMDIR_PASSWORD_RESET_REQUEST_DATA** ppPasswordResetRequest);

SSOERROR
VmdirUserArrayDataToJson(
    const VMDIR_USER_ARRAY_DATA* pUserArray,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToUserArrayData(
    PCSSO_JSON pJson,
    VMDIR_USER_ARRAY_DATA** ppUserArray);

SSOERROR
VmdirSolutionUserDataToJson(
    const VMDIR_SOLUTION_USER_DATA* pSolutionUser,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToSolutionUserData(
    PCSSO_JSON pJson,
    VMDIR_SOLUTION_USER_DATA** ppSolutionUser);

SSOERROR
VmdirSolutionUserArrayDataToJson(
    const VMDIR_SOLUTION_USER_ARRAY_DATA* pSolutionUserArray,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToSolutionUserArrayData(
    PCSSO_JSON pJson,
    VMDIR_SOLUTION_USER_ARRAY_DATA** ppSolutionUserArray);

SSOERROR
VmdirSearchResultDataToJson(
    const VMDIR_SEARCH_RESULT_DATA* pSearchResult,
    PSSO_JSON pJson);

SSOERROR
VmdirJsonToSearchResultData(
    PCSSO_JSON pJson,
    VMDIR_SEARCH_RESULT_DATA** ppSearchResult);

#endif /* PROTOTYPES_H_ */
