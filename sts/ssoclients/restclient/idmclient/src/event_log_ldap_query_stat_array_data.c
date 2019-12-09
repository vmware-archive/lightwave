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
IdmEventLogLdapQueryStatArrayDataNew(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA** ppEventLogLdapQueryStatArray,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEntry,
    size_t length)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray = NULL;
    size_t i = 0;

    if (ppEventLogLdapQueryStatArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA), (void**) &pEventLogLdapQueryStatArray);
    BAIL_ON_ERROR(e);

    if (ppEntry != NULL)
    {
        pEventLogLdapQueryStatArray->length = length;

        e = SSOMemoryAllocateArray(
            length,
            sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA*),
            (void**) &(pEventLogLdapQueryStatArray->ppEntry));
        BAIL_ON_ERROR(e);

        for (i = 0; i < length; i++)
        {
            e = IdmEventLogLdapQueryStatDataNew(
                &(pEventLogLdapQueryStatArray->ppEntry[i]),
                ppEntry[i]->baseDN,
                ppEntry[i]->query,
                ppEntry[i]->connection,
                ppEntry[i]->elapsedMillis,
                ppEntry[i]->count);
            BAIL_ON_ERROR(e);
        }
    }

    *ppEventLogLdapQueryStatArray = pEventLogLdapQueryStatArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogLdapQueryStatArrayDataDelete(pEventLogLdapQueryStatArray);
    }

    return e;
}

void
IdmEventLogLdapQueryStatArrayDataDelete(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray)
{
    if (pEventLogLdapQueryStatArray != NULL)
    {
        SSOMemoryFreeArrayOfObjects(
            (void**) pEventLogLdapQueryStatArray->ppEntry,
            pEventLogLdapQueryStatArray->length,
            (GenericDestructorFunction) IdmEventLogLdapQueryStatDataDelete);
        SSOMemoryFree(pEventLogLdapQueryStatArray, sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA));
    }
}

SSOERROR
IdmEventLogLdapQueryStatArrayDataToJson(
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogLdapQueryStatArray != NULL)
    {
        e = RestArrayDataToJson(
            (const REST_GENERIC_ARRAY_DATA*) pEventLogLdapQueryStatArray,
            (DataObjectToJsonFunc) IdmEventLogLdapQueryStatDataToJson,
            pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogLdapQueryStatArrayData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA** ppEventLogLdapQueryStatArray)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_LDAP_QUERY_STAT_ARRAY_DATA* pEventLogLdapQueryStatArray = NULL;

    if (pJson == NULL || ppEventLogLdapQueryStatArray == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = RestJsonToArrayData(
        pJson,
        (JsonToDataObjectFunc) IdmJsonToEventLogLdapQueryStatData,
        (REST_GENERIC_ARRAY_DATA**) &pEventLogLdapQueryStatArray);
    BAIL_ON_ERROR(e);

    *ppEventLogLdapQueryStatArray = pEventLogLdapQueryStatArray;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogLdapQueryStatArrayDataDelete(pEventLogLdapQueryStatArray);
    }

    return e;
}
