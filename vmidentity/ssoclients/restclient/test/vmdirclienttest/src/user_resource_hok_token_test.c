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

static PCSTRING tenant = "vsphere.local";
static PCSTRING name = "test_user_name";
static PSTRING domain = "vsphere.local";

PCSTRING
VmdirUserCreateTestByHOKToken()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    VMDIR_PRINCIPAL_DATA* pAlias = NULL;

    VMDIR_USER_DETAILS_DATA* pDetails = NULL;

    bool disabled = true;
    bool locked = true;
    PSTRING objectId = "objectId";

    SSO_LONG lastSet = 1;
    SSO_LONG lifetime = 200000;
    VMDIR_PASSWORD_DETAILS_DATA* pPasswordDetails = NULL;

    VMDIR_USER_DATA* pUser = NULL;


    e = VmdirPrincipalDataNew(&pAlias, "name", "domain");
    BAIL_ON_ERROR(e);

    e = VmdirUserDetailsDataNew(
        &pDetails,
        "name@domain.com",
        "name@vsphere.local",
        "firstName",
        "lastName",
        "description");
    BAIL_ON_ERROR(e);

    e = VmdirPasswordDetailsDataNew(&pPasswordDetails, "Admin!23", &lastSet, &lifetime);
    BAIL_ON_ERROR(e);

    e = VmdirUserDataNew(&pUser, name, domain, pAlias, pDetails, &disabled, &locked, objectId, pPasswordDetails);
    BAIL_ON_ERROR(e);

    e = VmdirUserCreate(pHOKTokenClient, tenant, pUser, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserGetTestByHOKToken()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    VMDIR_USER_DATA* pUserReturn = NULL;

    e = VmdirUserGet(pHOKTokenClient, tenant, name, domain, &pUserReturn, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}

PCSTRING
VmdirUserDeleteTestByHOKToken()
{
    SSOERROR e = SSOERROR_NONE;
    REST_SERVER_ERROR* pServerError = NULL;

    e = VmdirUserDelete(pHOKTokenClient, tenant, name, domain, &pServerError);
    BAIL_ON_ERROR(e);

    error:

    return RestTestGenerateErrorMessage(__FUNCTION__, e, pServerError);
}
