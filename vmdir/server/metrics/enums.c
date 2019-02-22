/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "includes.h"

PSTR
VmDirMetricsLdapOperationString(
    METRICS_LDAP_OPS    operation
    )
{
    static PSTR pszLdapOperations[METRICS_LDAP_OP_COUNT] =
    {
            "add",
            "modify",
            "delete",
            "search"
    };

    return pszLdapOperations[operation];
}

PSTR
VmDirMetricsLdapOpTypeString(
    METRICS_LDAP_OP_TYPES   opType
    )
{
    static PSTR pszLdapOpTypes[METRICS_LDAP_OP_TYPE_COUNT] =
    {
            "external",
            "internal",
            "replication"
    };

    return pszLdapOpTypes[opType];
}

PSTR
VmDirMetricsLdapErrorString(
    METRICS_LDAP_ERRORS error
    )
{
    static PSTR pszLdapErrorCodes[METRICS_LDAP_ERROR_COUNT] =
    {
            "LDAP_SUCCESS",
            "LDAP_UNAVAILABLE",
            "LDAP_SERVER_DOWN",
            "LDAP_UNWILLING_TO_PERFORM",
            "LDAP_INVALID_DN_SYNTAX",
            "LDAP_NO_SUCH_ATTRIBUTE",
            "LDAP_INVALID_SYNTAX",
            "LDAP_UNDEFINED_TYPE",
            "LDAP_TYPE_OR_VALUE_EXISTS",
            "LDAP_OBJECT_CLASS_VIOLATION",
            "LDAP_ALREADY_EXISTS",
            "LDAP_CONSTRAINT_VIOLATION",
            "LDAP_NOT_ALLOWED_ON_NONLEAF",
            "LDAP_PROTOCOL_ERROR",
            "LDAP_INVALID_CREDENTIALS",
            "LDAP_INSUFFICIENT_ACCESS",
            "LDAP_AUTH_METHOD_NOT_SUPPORTED",
            "LDAP_SASL_BIND_IN_PROGRESS",
            "LDAP_TIMELIMIT_EXCEEDED",
            "LDAP_SIZELIMIT_EXCEEDED",
            "LDAP_NO_SUCH_OBJECT",
            "LDAP_BUSY",
            "LDAP_OTHER"
    };

    return pszLdapErrorCodes[error];
}

PSTR
VmDirMetricsSrvStatString(
    METRICS_SRV_STAT srvStat
    )
{
    static PSTR pszSrvStat[METRICS_SRV_STAT_COUNT] =
    {
            "DBSizeInMB",
            "BackupTimeTakenInSec"
    };

    return pszSrvStat[srvStat];
}

PSTR
VmDirMetricsRpcOperationString(
    METRICS_RPC_OPS operation
    )
{
    static PSTR pszRpcOperations[METRICS_RPC_OP_COUNT] =
    {
            "GeneratePassword",
            "GetKeyTabRecBlob",
            "CreateUser",
            "CreateUserEx",
            "SetLogLevel",
            "SetLogMask",
            "SuperLogQueryServerData",
            "SuperLogEnable",
            "SuperLogDisable",
            "IsSuperLogEnabled",
            "SuperLogFlush",
            "SuperLogSetSize",
            "SuperLogGetSize",
            "SuperLogGetEntriesLdapOperation",
            "OpenDatabaseFile",
            "ReadDatabaseFile",
            "CloseDatabaseFile",
            "SetBackendState",
            "BackupDB",
            "GetState",
            "GetLogLevel",
            "GetLogMask"
    };

    return pszRpcOperations[operation];
}

METRICS_LDAP_OPS
VmDirMetricsMapLdapOperationToEnum(
    ber_tag_t   operation
    )
{
    METRICS_LDAP_OPS    match = METRICS_LDAP_OP_IGNORE;

    switch (operation)
    {
    case LDAP_REQ_ADD:
        match = METRICS_LDAP_OP_ADD;
        break;

    case LDAP_REQ_MODIFY:
        match = METRICS_LDAP_OP_MODIFY;
        break;

    case LDAP_REQ_DELETE:
        match = METRICS_LDAP_OP_DELETE;
        break;

    case LDAP_REQ_SEARCH:
        match = METRICS_LDAP_OP_SEARCH;
        break;

    default:
        break;// keep it IGNORE
    }

    return match;
}

METRICS_LDAP_OP_TYPES
VmDirMetricsMapLdapOpTypeToEnum(
    VDIR_OPERATION_TYPE opType
    )
{
    METRICS_LDAP_OP_TYPES   match = METRICS_LDAP_OP_TYPE_INTERNAL;

    switch (opType)
    {
    case VDIR_OPERATION_TYPE_EXTERNAL:
        match = METRICS_LDAP_OP_TYPE_EXTERNAL;
        break;

    case VDIR_OPERATION_TYPE_INTERNAL:
        match = METRICS_LDAP_OP_TYPE_INTERNAL;
        break;

    case VDIR_OPERATION_TYPE_REPL:
        match = METRICS_LDAP_OP_TYPE_REPL;
        break;

    default:
        // default to internal type
        match = METRICS_LDAP_OP_TYPE_INTERNAL;
        break;
    }

    return match;
}

METRICS_LDAP_ERRORS
VmDirMetricsMapLdapErrorToEnum(
    int error
    )
{
    METRICS_LDAP_ERRORS match = METRICS_LDAP_OTHER;

    switch (error)
    {
    case LDAP_SUCCESS:
        match = METRICS_LDAP_SUCCESS;
        break;

    case LDAP_UNAVAILABLE:
        match = METRICS_LDAP_UNAVAILABLE;
        break;

    case LDAP_SERVER_DOWN:
        match = METRICS_LDAP_SERVER_DOWN;
        break;

    case LDAP_UNWILLING_TO_PERFORM:
        match = METRICS_LDAP_UNWILLING_TO_PERFORM;
        break;

    case LDAP_INVALID_DN_SYNTAX:
        match = METRICS_LDAP_INVALID_DN_SYNTAX;
        break;

    case LDAP_NO_SUCH_ATTRIBUTE:
        match = METRICS_LDAP_NO_SUCH_ATTRIBUTE;
        break;

    case LDAP_INVALID_SYNTAX:
        match = METRICS_LDAP_INVALID_SYNTAX;
        break;

    case LDAP_UNDEFINED_TYPE:
        match = METRICS_LDAP_UNDEFINED_TYPE;
        break;

    case LDAP_TYPE_OR_VALUE_EXISTS:
        match = METRICS_LDAP_TYPE_OR_VALUE_EXISTS;
        break;

    case LDAP_OBJECT_CLASS_VIOLATION:
        match = METRICS_LDAP_OBJECT_CLASS_VIOLATION;
        break;

    case LDAP_ALREADY_EXISTS:
        match = METRICS_LDAP_ALREADY_EXISTS;
        break;

    case LDAP_CONSTRAINT_VIOLATION:
        match = METRICS_LDAP_CONSTRAINT_VIOLATION;
        break;

    case LDAP_NOT_ALLOWED_ON_NONLEAF:
        match = METRICS_LDAP_NOT_ALLOWED_ON_NONLEAF;
        break;

    case LDAP_PROTOCOL_ERROR:
        match = METRICS_LDAP_PROTOCOL_ERROR;
        break;

    case LDAP_INVALID_CREDENTIALS:
        match = METRICS_LDAP_INVALID_CREDENTIALS;
        break;

    case LDAP_INSUFFICIENT_ACCESS:
        match = METRICS_LDAP_INSUFFICIENT_ACCESS;
        break;

    case LDAP_AUTH_METHOD_NOT_SUPPORTED:
        match = METRICS_LDAP_AUTH_METHOD_NOT_SUPPORTED;
        break;

    case LDAP_SASL_BIND_IN_PROGRESS:
        match = METRICS_LDAP_SASL_BIND_IN_PROGRESS;
        break;

    case LDAP_TIMELIMIT_EXCEEDED:
        match = METRICS_LDAP_TIMELIMIT_EXCEEDED;
        break;

    case LDAP_SIZELIMIT_EXCEEDED:
        match = METRICS_LDAP_SIZELIMIT_EXCEEDED;
        break;

    case LDAP_NO_SUCH_OBJECT:
        match = METRICS_LDAP_NO_SUCH_OBJECT;
        break;

    case LDAP_BUSY:
        match = METRICS_LDAP_BUSY;
        break;

    default:
        match = METRICS_LDAP_OTHER;
        break;
    }

    return match;
}
