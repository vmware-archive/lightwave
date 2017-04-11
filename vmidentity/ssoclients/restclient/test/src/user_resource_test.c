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

static PCSTRING name = "test_user_rest_c_client";

PCSTRING
IdmExternalUserCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    bool* pCreated = NULL;

    PCSTRING domain = testTenant;

    IDM_PRINCIPAL_DATA* pPrincipal = NULL;
    e = IdmPrincipalDataNew(&pPrincipal, name, domain);
    BAIL_ON_ERROR(e);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalUserCreate(pRestClient, testTenant, pPrincipal, &pCreated, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_USER_DATA* pUserReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmUserGet(pRestClient, testTenant, name, domain, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmUserDataDelete(pUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserGetGroupsTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    IDM_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmUserGetGroups(pRestClient, testTenant, name, domain, true, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    IdmGroupArrayDataDelete(pGroupArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
IdmUserDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = IdmExternalUserDelete(pRestClient, testTenant, name, domain, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserCreateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;

    VMDIR_USER_DETAILS_DATA* pDetails = NULL;

    bool disabled = false;
    bool locked = true;
    PSTRING objectId = "objectId";

    SSO_LONG lastSet = 1;
    SSO_LONG lifetime = 200000;
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails = NULL;

    VMDIR_USER_DATA* pUser = NULL;

    PCSTRING domain = testTenant;

    e = VmdirPrincipalDataNew(&pAlias, "name", "domain");
    BAIL_ON_ERROR(e);

    e = VmdirUserDetailsDataNew(
        &pDetails,
        "name@domain.com",
        "name@test_tenant_name",
        "firstName",
        "lastName",
        "description");
    BAIL_ON_ERROR(e);

    e = VmdirPasswordDetailsDataNew(&pPasswordDetails, "Admin!23", &lastSet, &lifetime);
    BAIL_ON_ERROR(e);

    e = VmdirUserDataNew(&pUser, name, domain, pAlias, pDetails, &disabled, &locked, objectId, pPasswordDetails);
    BAIL_ON_ERROR(e);

    VmdirPasswordDetailsDataDelete(pPasswordDetails);
    VmdirPrincipalDataDelete(pAlias);
    VmdirUserDetailsDataDelete(pDetails);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserCreate(pRestClient, testTenant, pUser, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirUserDataDelete(pUserReturn);
    VmdirUserDataDelete(pUser);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserDeleteTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserDelete(pRestClient, testTenant, name, domain, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserGetTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserGet(pRestClient, testTenant, name, domain, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirUserDataDelete(pUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserGetGroupsTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_GROUP_ARRAY_DATA* pGroupArrayReturn = NULL;

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserGetGroups(pRestClient, testTenant, name, domain, true, &pGroupArrayReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirGroupArrayDataDelete(pGroupArrayReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserUpdateTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;

    VMDIR_USER_DETAILS_DATA* pDetails = NULL;

    bool disabled = true;
    bool locked = true;
    PSTRING objectId = "objectId2";

    SSO_LONG lastSet = 1;
    SSO_LONG lifetime = 200000;
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails = NULL;

    VMDIR_USER_DATA* pUser = NULL;

    PCSTRING domain = testTenant;

    e = VmdirPrincipalDataNew(&pAlias, "name2", "domain2");
    BAIL_ON_ERROR(e);

    e = VmdirUserDetailsDataNew(
        &pDetails,
        "name2@domain.com",
        "name2@test_tenant_name",
        "firstName2",
        "lastName2",
        "description2");
    BAIL_ON_ERROR(e);

    e = VmdirPasswordDetailsDataNew(&pPasswordDetails, "Admin!23", &lastSet, &lifetime);
    BAIL_ON_ERROR(e);

    e = VmdirUserDataNew(&pUser, name, domain, pAlias, pDetails, &disabled, &locked, objectId, pPasswordDetails);
    BAIL_ON_ERROR(e);

    VmdirPasswordDetailsDataDelete(pPasswordDetails);
    VmdirPrincipalDataDelete(pAlias);
    VmdirUserDetailsDataDelete(pDetails);

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserUpdate(pRestClient, testTenant, name, domain, pUser, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirUserDataDelete(pUserReturn);
    VmdirUserDataDelete(pUser);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserUpdatePasswordTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    PCSTRING currentPassword = "Admin!23";
    PCSTRING newPassword = "Admin!56";

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserUpdatePassword(pRestClient, testTenant, name, domain, currentPassword, newPassword, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirUserDataDelete(pUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserResetPasswordTest()
{
    SSOERROR e = SSOERROR_NONE;
    PREST_CLIENT pRestClient = NULL;

    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    PCSTRING newPassword = "Admin!12";

    PCSTRING domain = testTenant;

    e = RestTestBearerTokenClientNew(pscHost, pscPort, testTenant, testTenantUsername, testTenantPassword, false, &pRestClient);
    BAIL_ON_ERROR(e);

    e = VmdirUserResetPassword(pRestClient, testTenant, name, domain, newPassword, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    // cleanup
    RestClientDelete(pRestClient);
    VmdirUserDataDelete(pUserReturn);

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
