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

SSOERROR
IdmLockoutPolicyDataNew(
    IDM_LOCKOUT_POLICY_DATA** ppLockoutPolicy,
    PCSTRING description,
    const SSO_LONG* failedAttemptIntervalSec,
    const INTEGER* maxFailedAttempts,
    const SSO_LONG* autoUnlockIntervalSec)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy = NULL;

    if (ppLockoutPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_LOCKOUT_POLICY_DATA), (void**) &pLockoutPolicy);
    BAIL_ON_ERROR(e);

    if (description != NULL)
    {
        e = RestStringDataNew(&(pLockoutPolicy->description), description);
        BAIL_ON_ERROR(e);
    }

    if (failedAttemptIntervalSec != NULL)
    {
        e = RestLongDataNew(&(pLockoutPolicy->failedAttemptIntervalSec), *failedAttemptIntervalSec);
        BAIL_ON_ERROR(e);
    }

    if (maxFailedAttempts != NULL)
    {
        e = RestIntegerDataNew(&(pLockoutPolicy->maxFailedAttempts), *maxFailedAttempts);
        BAIL_ON_ERROR(e);
    }

    if (autoUnlockIntervalSec != NULL)
    {
        e = RestLongDataNew(&(pLockoutPolicy->autoUnlockIntervalSec), *autoUnlockIntervalSec);
        BAIL_ON_ERROR(e);
    }

    *ppLockoutPolicy = pLockoutPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmLockoutPolicyDataDelete(pLockoutPolicy);
    }

    return e;
}

void
IdmLockoutPolicyDataDelete(
    IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy)
{
    if (pLockoutPolicy != NULL)
    {
        RestStringDataDelete(pLockoutPolicy->description);
        RestLongDataDelete(pLockoutPolicy->failedAttemptIntervalSec);
        RestIntegerDataDelete(pLockoutPolicy->maxFailedAttempts);
        RestLongDataDelete(pLockoutPolicy->autoUnlockIntervalSec);
        SSOMemoryFree(pLockoutPolicy, sizeof(IDM_LOCKOUT_POLICY_DATA));
    }
}

SSOERROR
IdmLockoutPolicyDataToJson(
    const IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pLockoutPolicy != NULL)
    {
        e = RestDataToJson(pLockoutPolicy->description, REST_JSON_OBJECT_TYPE_STRING, NULL, "description", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pLockoutPolicy->failedAttemptIntervalSec,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "failedAttemptIntervalSec",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pLockoutPolicy->maxFailedAttempts,
            REST_JSON_OBJECT_TYPE_INTEGER,
            NULL,
            "maxFailedAttempts",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pLockoutPolicy->autoUnlockIntervalSec,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "autoUnlockIntervalSec",
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToLockoutPolicyData(
    PCSSO_JSON pJson,
    IDM_LOCKOUT_POLICY_DATA** ppLockoutPolicy)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_LOCKOUT_POLICY_DATA* pLockoutPolicy = NULL;

    if (pJson == NULL || ppLockoutPolicy == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_LOCKOUT_POLICY_DATA), (void**) &pLockoutPolicy);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "description",
        (void**) &(pLockoutPolicy->description));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "failedAttemptIntervalSec",
        (void**) &(pLockoutPolicy->failedAttemptIntervalSec));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "maxFailedAttempts",
        (void**) &(pLockoutPolicy->maxFailedAttempts));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "autoUnlockIntervalSec",
        (void**) &(pLockoutPolicy->autoUnlockIntervalSec));
    BAIL_ON_ERROR(e);

    *ppLockoutPolicy = pLockoutPolicy;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmLockoutPolicyDataDelete(pLockoutPolicy);
    }

    return e;
}
