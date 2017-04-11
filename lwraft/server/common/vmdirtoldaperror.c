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



/*
 * Module Name: Directory common
 *
 * Filename: vmdirtoldaperror.c
 *
 * Abstract: Map VmDir error to LDAP error
 *
 */

#include "includes.h"

/*
 * Map vmdir to ldap error
 */
DWORD
VmDirToLDAPError(
    DWORD   dwVmDirError
    )
{
    DWORD   dwError = dwVmDirError;

    if ( IS_VMDIR_ERROR_SPACE( dwError ) )
    {
        switch (dwVmDirError)
        {
            case VMDIR_SUCCESS:
                dwError = LDAP_SUCCESS;
                break;

            case VMDIR_ERROR_BUSY:
                dwError = LDAP_BUSY;
                break;

            case VMDIR_ERROR_UNAVAILABLE:
                dwError = LDAP_UNAVAILABLE;
                break;

            case VMDIR_ERROR_UNWILLING_TO_PERFORM:
                dwError = LDAP_UNWILLING_TO_PERFORM;
                break;

            case VMDIR_ERROR_INVALID_DN:
                dwError = LDAP_INVALID_DN_SYNTAX;
                break;

            case VMDIR_ERROR_NO_SUCH_ATTRIBUTE:
                dwError = LDAP_NO_SUCH_ATTRIBUTE;
                break;

            case VMDIR_ERROR_INVALID_SYNTAX:
            case VMDIR_ERROR_BAD_ATTRIBUTE_DATA:
                dwError = LDAP_INVALID_SYNTAX;
                break;

            case VMDIR_ERROR_UNDEFINED_TYPE:
                dwError = LDAP_UNDEFINED_TYPE;
                break;

            case VMDIR_ERROR_TYPE_OR_VALUE_EXISTS:
                dwError = LDAP_TYPE_OR_VALUE_EXISTS;
                break;

            case VMDIR_ERROR_OBJECTCLASS_VIOLATION:
            case VMDIR_ERROR_STRUCTURE_VIOLATION:
                dwError = LDAP_OBJECT_CLASS_VIOLATION;
                break;

            case VMDIR_ERROR_ENTRY_ALREADY_EXIST:
            case VMDIR_ERROR_BACKEND_ENTRY_EXISTS:
                dwError = LDAP_ALREADY_EXISTS;
                break;

            case  VMDIR_ERROR_ENTRY_NOT_FOUND:
            case  VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND:
            case  VMDIR_ERROR_BACKEND_PARENT_NOTFOUND:
                dwError = LDAP_NO_SUCH_OBJECT;
                break;

            case VMDIR_ERROR_PASSWORD_POLICY_VIOLATION:
            case VMDIR_ERROR_INVALID_POLICY_DEFINITION:
            case VMDIR_ERROR_PASSWORD_TOO_LONG:
            case VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION:
            case VMDIR_ERROR_RID_LIMIT_EXCEEDED:
            case VMDIR_ERROR_BACKEND_CONSTRAINT:
                dwError = LDAP_CONSTRAINT_VIOLATION;
                break;

            case VMDIR_ERROR_USER_INVALID_CREDENTIAL:
                dwError = LDAP_INVALID_CREDENTIALS;
                break;

            case VMDIR_ERROR_PASSWORD_EXPIRED:
            case VMDIR_ERROR_ACCOUNT_LOCKED:
            case VMDIR_ERROR_ACCOUNT_DISABLED:
            case VMDIR_ERROR_USER_LOCKOUT:
            case VMDIR_ERROR_USER_NO_CREDENTIAL:
            case VMDIR_ERROR_INSUFFICIENT_ACCESS:
                dwError = LDAP_INSUFFICIENT_ACCESS;
                break;

            case VMDIR_ERROR_AUTH_METHOD_NOT_SUPPORTED:
                dwError = LDAP_AUTH_METHOD_NOT_SUPPORTED;
                break;

            case VMDIR_ERROR_TIMELIMIT_EXCEEDED:
                dwError = LDAP_TIMELIMIT_EXCEEDED;
                break;

            case VMDIR_ERROR_SIZELIMIT_EXCEEDED:
                dwError = LDAP_SIZELIMIT_EXCEEDED;
                break;

            case VMDIR_ERROR_NOT_ALLOWED_ON_NONLEAF:
                dwError = LDAP_NOT_ALLOWED_ON_NONLEAF;
                break;

            case VMDIR_ERROR_SASL_BIND_IN_PROGRESS:
                dwError = LDAP_SASL_BIND_IN_PROGRESS;
                break;

            case VMDIR_ERROR_INVALID_REQUEST:
                dwError = LDAP_PROTOCOL_ERROR;
                break;

            default:
                dwError = LDAP_OPERATIONS_ERROR;
                break;
        }
    }
    else if ( NOT_LDAP_ERROR_SPACE( dwVmDirError ) )
    {   // for all non-VmDir/LDAP error case
        dwError = LDAP_OPERATIONS_ERROR;
    }

    return dwError;
}
