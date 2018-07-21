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

#ifndef VM_JSONRESULT_H_
#define VM_JSONRESULT_H_

/* client handle */
typedef struct _VM_JSON_RESULT VM_JSON_RESULT, *PVM_JSON_RESULT;

/* json navigation handle */
typedef PVOID PVM_JSON_POSITION;

typedef enum _VM_JSON_RESULT_TYPE
{
    JSON_RESULT_INVALID=-1,
    JSON_RESULT_OBJECT,
    JSON_RESULT_ARRAY,
    JSON_RESULT_STRING,
    JSON_RESULT_INTEGER,
    JSON_RESULT_REAL,
    JSON_RESULT_BOOLEAN,
    JSON_RESULT_NULL
}VM_JSON_RESULT_TYPE;

typedef struct _VM_JSON_RESULT_VALUE
{
    union
    {
        PVM_JSON_POSITION pObject;
        PVM_JSON_POSITION pArray;
        PCSTR pszValue;
        int nValue;
        double dValue;
        BOOLEAN bValue;
    }value;
    VM_JSON_RESULT_TYPE nType;
}VM_JSON_RESULT_VALUE, *PVM_JSON_RESULT_VALUE;

/* callback for arrays */
typedef DWORD (*PFN_JSON_RESULT_ARRAY_CB)(
                  PVOID pUserData,
                  size_t,
                  PVM_JSON_POSITION);

/* callback for objects */
typedef DWORD (*PFN_JSON_RESULT_OBJECT_CB)(
                  PVOID pUserData,
                  PCSTR,
                  PVM_JSON_RESULT_VALUE);
/*
 * Initialize json result
 */
DWORD
VmJsonResultInit(
    PVM_JSON_RESULT *ppResult
    );

/* parse and load json string. if error, returns error details in result */
DWORD
VmJsonResultLoadString(
    PCSTR pszJson,
    PVM_JSON_RESULT pResult
    );

DWORD
VmJsonResultGetRootPosition(
    PVM_JSON_RESULT pResult,
    PVM_JSON_POSITION *ppPosition
    );

DWORD
VmJsonResultIterateObjectAt(
    PVM_JSON_POSITION pPosition,
    PVOID pUserData,
    PFN_JSON_RESULT_OBJECT_CB pfnCB
    );
/*
 * Free handle
 */
VOID
VmJsonResultFreeHandle(
    PVM_JSON_RESULT pResult
    );

#endif /* VM_JSONRESULT_H_ */
