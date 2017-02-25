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

#include "includes.h"

static PSTRING name = "test_group_rest_c_client";

PCSTRING
IdmGroupGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_DATA* pGroupReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmGroupGet(pRestClient, testTenant, name, domain, &pGroupReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmGroupDataDelete(pGroupReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmGroupGetMembersTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_MEMBER_TYPE memberType = IDM_MEMBER_TYPE_USER;
    size_t limit = 10;
    IDM_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmGroupGetMembers(pRestClient, testTenant, name, domain, memberType, limit, &pSearchResultReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmSearchResultDataDelete(pSearchResultReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmGroupGetParentsTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmGroupGetParents(pRestClient, testTenant, name, domain, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmGroupArrayDataDelete(pGroupArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_GROUP_DATA* pGroupReturn = NULL;

    VMDIR_GROUP_DETAILS_DATA* pDetails = NULL;

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;
    // alias is not supported on server yet and should be NULL.

    PSTRING objectId = "test_objectId";
    VMDIR_GROUP_DATA* pGroup = NULL;

    PCSTRING domain = testTenant;

    e = VmdirGroupDetailsDataNew(&pDetails, "test_group_details");
    BAIL_ON_ERROR(e);

    e = VmdirGroupDataNew(&pGroup, name, domain, pDetails, pAlias, objectId);
    BAIL_ON_ERROR(e);

    VmdirGroupDetailsDataDelete(pDetails);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupCreate(pRestClient, testTenant, pGroup, &pGroupReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirGroupDataDelete(pGroupReturn);
    VmdirGroupDataDelete(pGroup);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupDelete(pRestClient, testTenant, name, domain, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_GROUP_DATA* pGroupReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupGet(pRestClient, testTenant, name, domain, &pGroupReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirGroupDataDelete(pGroupReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_GROUP_DATA* pGroupReturn = NULL;

    VMDIR_GROUP_DETAILS_DATA* pDetails = NULL;

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;
    // alias is not supported on server yet and should be NULL.

    PSTRING objectId = "test_objectId";
    VMDIR_GROUP_DATA* pGroup = NULL;

    PCSTRING domain = testTenant;

    e = VmdirGroupDetailsDataNew(&pDetails, "test_group_details");
    BAIL_ON_ERROR(e);

    e = VmdirGroupDataNew(&pGroup, NULL, NULL, pDetails, pAlias, objectId);
    BAIL_ON_ERROR(e);

    VmdirGroupDetailsDataDelete(pDetails);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupUpdate(pRestClient, testTenant, name, domain, pGroup, &pGroupReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirGroupDataDelete(pGroupReturn);
    VmdirGroupDataDelete(pGroup);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupAddMembersTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PSTRING pMemberArray[] =
    {
        "Administrator@test_tenant_name"
    };
    VMDIR_STRING_ARRAY_DATA* pMembers = NULL;

    VMDIR_MEMBER_TYPE memberType = VMDIR_MEMBER_TYPE_USER;

    PCSTRING domain = testTenant;

    e = VmdirStringArrayDataNew(&pMembers, pMemberArray, (size_t) (sizeof(pMemberArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupAddMembers(pRestClient, testTenant, name, domain, pMembers, memberType, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirStringArrayDataDelete(pMembers);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupGetMembersTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_MEMBER_TYPE memberType = VMDIR_MEMBER_TYPE_USER;
    size_t limit = 10;
    VMDIR_SEARCH_RESULT_DATA* pSearchResultReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupGetMembers(pRestClient, testTenant, name, domain, memberType, limit, &pSearchResultReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirSearchResultDataDelete(pSearchResultReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupRemoveMembersTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PSTRING pMemberArray[] =
    {
        "Administrator@test_tenant_name"
    };
    VMDIR_STRING_ARRAY_DATA* pMembers = NULL;

    VMDIR_MEMBER_TYPE memberType = VMDIR_MEMBER_TYPE_USER;

    PCSTRING domain = testTenant;

    e = VmdirStringArrayDataNew(&pMembers, pMemberArray, (size_t) (sizeof(pMemberArray) / sizeof(PSTRING)));
    BAIL_ON_ERROR(e);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupRemoveMembers(pRestClient, testTenant, name, domain, pMembers, memberType, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirStringArrayDataDelete(pMembers);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirGroupGetParentsTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirGroupGetParents(pRestClient, testTenant, name, domain, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirGroupArrayDataDelete(pGroupArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
