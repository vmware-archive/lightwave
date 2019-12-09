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
IdmEventLogLdapQueryStatDataNew(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEventLogLdapQueryStat,
    PCSTRING baseDN,
    PCSTRING query,
    PCSTRING connection,
    const SSO_LONG* elapsedMillis,
    const INTEGER* count)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat = NULL;

    if (ppEventLogLdapQueryStat == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA), (void**) &pEventLogLdapQueryStat);
    BAIL_ON_ERROR(e);

    if (baseDN != NULL)
    {
        e = RestStringDataNew(&(pEventLogLdapQueryStat->baseDN), baseDN);
        BAIL_ON_ERROR(e);
    }

    if (query != NULL)
    {
        e = RestStringDataNew(&(pEventLogLdapQueryStat->query), query);
        BAIL_ON_ERROR(e);
    }

    if (connection != NULL)
    {
        e = RestStringDataNew(&(pEventLogLdapQueryStat->connection), connection);
        BAIL_ON_ERROR(e);
    }

    if (elapsedMillis != NULL)
    {
        e = RestLongDataNew(&(pEventLogLdapQueryStat->elapsedMillis), *elapsedMillis);
        BAIL_ON_ERROR(e);
    }

    if (count != NULL)
    {
        e = RestIntegerDataNew(&(pEventLogLdapQueryStat->count), *count);
        BAIL_ON_ERROR(e);
    }

    *ppEventLogLdapQueryStat = pEventLogLdapQueryStat;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogLdapQueryStatDataDelete(pEventLogLdapQueryStat);
    }

    return e;
}

void
IdmEventLogLdapQueryStatDataDelete(
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat)
{
    if (pEventLogLdapQueryStat != NULL)
    {
        RestStringDataDelete(pEventLogLdapQueryStat->baseDN);
        RestStringDataDelete(pEventLogLdapQueryStat->query);
        RestStringDataDelete(pEventLogLdapQueryStat->connection);
        RestLongDataDelete(pEventLogLdapQueryStat->elapsedMillis);
        RestIntegerDataDelete(pEventLogLdapQueryStat->count);
        SSOMemoryFree(pEventLogLdapQueryStat, sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA));
    }
}

SSOERROR
IdmEventLogLdapQueryStatDataToJson(
    const IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat,
    PSSO_JSON pJson)
{
    SSOERROR e = SSOERROR_NONE;

    if (pJson == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    if (pEventLogLdapQueryStat != NULL)
    {
        e = RestDataToJson(pEventLogLdapQueryStat->baseDN, REST_JSON_OBJECT_TYPE_STRING, NULL, "baseDN", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLogLdapQueryStat->query, REST_JSON_OBJECT_TYPE_STRING, NULL, "query", pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogLdapQueryStat->connection,
            REST_JSON_OBJECT_TYPE_STRING,
            NULL,
            "connection",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(
            pEventLogLdapQueryStat->elapsedMillis,
            REST_JSON_OBJECT_TYPE_LONG,
            NULL,
            "elapsedMillis",
            pJson);
        BAIL_ON_ERROR(e);

        e = RestDataToJson(pEventLogLdapQueryStat->count, REST_JSON_OBJECT_TYPE_INTEGER, NULL, "count", pJson);
        BAIL_ON_ERROR(e);
    }

    error:

    return e;
}

SSOERROR
IdmJsonToEventLogLdapQueryStatData(
    PCSSO_JSON pJson,
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA** ppEventLogLdapQueryStat)
{
    SSOERROR e = SSOERROR_NONE;
    IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA* pEventLogLdapQueryStat = NULL;

    if (pJson == NULL || ppEventLogLdapQueryStat == NULL)
    {
        e = SSOERROR_INVALID_ARGUMENT;
        BAIL_ON_ERROR(e);
    }

    e = SSOMemoryAllocate(sizeof(IDM_EVENT_LOG_LDAP_QUERY_STAT_DATA), (void**) &pEventLogLdapQueryStat);
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "baseDN",
        (void**) &(pEventLogLdapQueryStat->baseDN));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "query",
        (void**) &(pEventLogLdapQueryStat->query));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_STRING,
        NULL,
        "connection",
        (void**) &(pEventLogLdapQueryStat->connection));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_LONG,
        NULL,
        "elapsedMillis",
        (void**) &(pEventLogLdapQueryStat->elapsedMillis));
    BAIL_ON_ERROR(e);

    e = RestJsonToData(
        pJson,
        REST_JSON_OBJECT_TYPE_INTEGER,
        NULL,
        "count",
        (void**) &(pEventLogLdapQueryStat->count));
    BAIL_ON_ERROR(e);

    *ppEventLogLdapQueryStat = pEventLogLdapQueryStat;

    error:

    if (e != SSOERROR_NONE)
    {
        IdmEventLogLdapQueryStatDataDelete(pEventLogLdapQueryStat);
    }

    return e;
}
