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
 * Module Name: VDIR
 *
 * Filename: vmdirerrorcode.h
 *
 * Abstract:
 *
 * Common error code map
 *
 */

#ifndef __VDIR_ERRORCODE_H__
#define __VDIR_ERRORCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VMDIR_SET_BACKEND_ERROR(dwError)            \
    if (! IS_VMDIR_BACKEND_ERROR_SPACE(dwError))    \
    {                                               \
        dwError = ERROR_BACKEND_ERROR;              \
    }

/////////////////////////////////////////////////////////
// keep this mapping during transition to VMDIR_ERROR_XXX
/////////////////////////////////////////////////////////
#define ERROR_OPERATION_NOT_PERMITTED           VMDIR_ERROR_OPERATION_NOT_PERMITTED
#define ERROR_NO_SUCH_FILE_OR_DIRECTORY         VMDIR_ERROR_NO_SUCH_FILE_OR_DIRECTORY
#define ERROR_IO                                VMDIR_ERROR_IO
#define ERROR_NO_MEMORY                         VMDIR_ERROR_NO_MEMORY
//#define ERROR_INVALID_PARAMETER                 VMDIR_ERROR_INVALID_PARAMETER
//#define ERROR_NOT_FOUND                       VMDIR_ERROR_NOT_FOUND
#define ERROR_GENERIC                           VMDIR_ERROR_GENERIC
#define ERROR_INVALID_CONFIGURATION             VMDIR_ERROR_INVALID_CONFIGURATION
#define ERROR_DATA_CONSTRAINT_VIOLATION          VMDIR_ERROR_DATA_CONSTRAINT_VIOLATION
#define ERROR_BAD_ATTRIBUTE_DATA                VMDIR_ERROR_BAD_ATTRIBUTE_DATA
#define ERROR_INVALID_REALM                     VMDIR_ERROR_INVALID_REALM
#define ERROR_PARENT_NOT_FOUND                  VMDIR_ERROR_PARENT_NOT_FOUND
#define ERROR_ENTRY_NOT_FOUND                   VMDIR_ERROR_ENTRY_NOT_FOUND
#define ERROR_ENTRY_ALREADY_EXIST               VMDIR_ERROR_ENTRY_ALREADY_EXIST
#define ERROR_RID_LIMIT_EXCEEDED                VMDIR_ERROR_RID_LIMIT_EXCEEDED
#define ERROR_NO_OBJECT_SID_GEN                 VMDIR_ERROR_NO_OBJECT_SID_GEN
#define ERROR_NO_SECURITY_DESCRIPTOR            VMDIR_ERROR_NO_SECURITY_DESCRIPTOR
#define ERROR_NO_OBJECTSID_ATTR                 VMDIR_ERROR_NO_OBJECTSID_ATTR
#define ERROR_TOKEN_IN_USE                      VMDIR_ERROR_TOKEN_IN_USE
#define ERROR_NO_MYSELF                         VMDIR_ERROR_NO_MYSELF
#define ERROR_PASSWORD_TOO_LONG                 VMDIR_ERROR_PASSWORD_TOO_LONG
#define ERROR_PASSWORD_HASH                     VMDIR_ERROR_PASSWORD_HASH
#define ERROR_PASSWORD_POLICY_VIOLATION         VMDIR_ERROR_PASSWORD_POLICY_VIOLATION
#define ERROR_USER_LOCKOUT                      VMDIR_ERROR_USER_LOCKOUT
#define ERROR_USER_INVALID_CREDENTIAL           VMDIR_ERROR_USER_INVALID_CREDENTIAL
#define ERROR_KERBEROS_ERROR                    VMDIR_ERROR_KERBEROS_ERROR
#define ERROR_KERBEROS_REALM_OFFLINE            VMDIR_ERROR_KERBEROS_REALM_OFFLINE
#define ERROR_NO_CRED_CACHE_NAME                VMDIR_ERROR_NO_CRED_CACHE_NAME
#define ERROR_NO_CRED_CACHE_FOUND               VMDIR_ERROR_NO_CRED_CACHE_FOUND
#define ERROR_VDCPROMO                          VMDIR_ERROR_VDCPROMO
#define ERROR_VDCMERGE                          VMDIR_ERROR_VDCMERGE
#define ERROR_VDCSPLIT                          VMDIR_ERROR_VDCSPLIT
#define ERROR_VDCREPADMIN_GENERAL               VMDIR_ERROR_VDCREPADMIN_GENERAL
#define ERROR_VDCREPADMIN_TOO_FEW_REPLICATION_PARTNERS  VMDIR_ERROR_VDCREPADMIN_TOO_FEW_REPLICATION_PARTNERS
#define ERROR_NO_SCHEMA                         VMDIR_ERROR_NO_SCHEMA
#define ERROR_INVALID_SCHEMA                    VMDIR_ERROR_INVALID_SCHEMA
#define ERROR_INVALID_DN                        VMDIR_ERROR_INVALID_DN
#define ERROR_INVALID_SYNTAX                    VMDIR_ERROR_INVALID_SYNTAX
#define ERROR_INVALID_ENTRY                     VMDIR_ERROR_INVALID_ENTRY
#define ERROR_INVALID_ATTRIBUTETYPES            VMDIR_ERROR_INVALID_ATTRIBUTETYPES
#define ERROR_INVALID_OBJECTCLASSES             VMDIR_ERROR_INVALID_OBJECTCLASSES
#define ERROR_INVALID_DITCONTENTRULES           VMDIR_ERROR_INVALID_DITCONTENTRULES
#define ERROR_INVALID_NAMEFORMS                 VMDIR_ERROR_INVALID_NAMEFORMS
#define ERROR_INVALID_DITSTRUCTURERULES         VMDIR_ERROR_INVALID_DITSTRUCTURERULES
#define ERROR_NO_SUCH_SYNTAX                    VMDIR_ERROR_NO_SUCH_SYNTAX
#define ERROR_NO_SUCH_ATTRIBUTE                 VMDIR_ERROR_NO_SUCH_ATTRIBUTE
#define ERROR_NO_SUCH_OBJECTCLASS               VMDIR_ERROR_NO_SUCH_OBJECTCLASS
#define ERROR_NO_SUCH_DITCONTENTRULES           VMDIR_ERROR_NO_SUCH_DITCONTENTRULES
#define ERROR_SCHEMA_MISMATCH                   VMDIR_ERROR_SCHEMA_MISMATCH
#define ERROR_NO_SUCH_NAMEFORMS                 VMDIR_ERROR_NO_SUCH_NAMEFORMS
#define ERROR_NO_SUCH_DITSTRUCTURERULES         VMDIR_ERROR_NO_SUCH_DITSTRUCTURERULES
#define ERROR_BACKEND_ERROR                     VMDIR_ERROR_BACKEND_ERROR
#define ERROR_BACKEND_MAX_RETRY                 VMDIR_ERROR_BACKEND_MAX_RETRY
#define ERROR_BACKEND_DEADLOCK                  VMDIR_ERROR_BACKEND_DEADLOCK
#define ERROR_BACKEND_ENTRY_NOTFOUND            VMDIR_ERROR_BACKEND_ENTRY_NOTFOUND
#define ERROR_BACKEND_PARENT_NOTFOUND           VMDIR_ERROR_BACKEND_PARENT_NOTFOUND
#define ERROR_BACKEND_CONSTRAINT                VMDIR_ERROR_BACKEND_CONSTRAINT
#define ERROR_BACKEND_ENTRY_EXISTS              VMDIR_ERROR_BACKEND_ENTRY_EXISTS
#define ERROR_BACKEND_OPERATIONS                VMDIR_ERROR_BACKEND_OPERATIONS
#define ERROR_BACKEND_ATTR_META_DATA_NOTFOUND   VMDIR_ERROR_BACKEND_ATTR_META_DATA_NOTFOUND
#define ERROR_NO_FUNC_LVL                       VMDIR_ERROR_NO_FUNC_LVL
#define ERROR_INVALID_FUNC_LVL                  VMDIR_ERROR_INVALID_FUNC_LVL
#define ERROR_INCOMPLETE_MAX_DFL                VMDIR_ERROR_INCOMPLETE_MAX_DFL

#ifdef __cplusplus
}
#endif

#endif /* __VDIR_ERRORCODE_H__ */

