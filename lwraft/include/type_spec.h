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
 * Filename: type_spec.h
 *
 * Abstract:
 *
 * IPC Type Spec
 *
 */

#ifndef __TYPE_SPEC_H__
#define __TYPE_SPEC_H__

typedef enum
{
    VMW_IPC_TYPE_UNDEFINED = 0,
    VMW_IPC_TYPE_BOOL,
    VMW_IPC_TYPE_BOOLEAN,
    VMW_IPC_TYPE_UINT16,
    VMW_IPC_TYPE_INT16,
    VMW_IPC_TYPE_UINT32,
    VMW_IPC_TYPE_INT32,
    VMW_IPC_TYPE_UINT64,
    VMW_IPC_TYPE_INT64,
    VMW_IPC_TYPE_STRING,
    VMW_IPC_TYPE_WSTRING,
    VMW_IPC_TYPE_BLOB_SIZE,
    VMW_IPC_TYPE_BLOB
} VMW_IPC_TYPE;

typedef union _VMW_DATA_SPEC_
{
    PVOID pVoid;
    PBYTE pByte;
    PUINT16 pUint16;
    PINT16 pInt16;
    PUINT32 pUint32;
    PINT32 pInt32;
    PUINT64 pUint54;
    PINT64 iInt64;
    PSTR pString;
    PWSTR pWString;
    PBOOL pBool;
    PBOOLEAN pBoolean;
} VMW_DATA_SPEC, *PVMW_DATA_SPEC;

typedef struct _VMW_TYPE_SPEC_
{
    PSTR pszName;
    VMW_IPC_TYPE type;
    VMW_DATA_SPEC data;
}VMW_TYPE_SPEC, *PVMW_TYPE_SPEC;

#define RESPONSE_PARAMS \
{\
    {\
        "Return Code",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    }\
}

#define INITIALIZE_HOST_INPUT_PARAMS \
{\
    {\
        "Naming Context",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "User Name",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "Password",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "Site Name",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "Replication URI",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "First Replication Cycle Mode",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    }\
}

#define INITIALIZE_TENANT_INPUT_PARAMS \
{\
    {\
        "Naming Context",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "User Name",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "Password",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
}

#define FORCE_RESET_PASSWORD_INPUT_PARAMS \
{\
    {\
        "Target DN",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
}

#define FORCE_RESET_PASSWORD_OUTPUT_PARAMS \
{\
    {\
        "Return Code",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Password Data Size",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Password Data",\
        VMW_IPC_TYPE_BLOB,\
        {NULL}\
    }\
}

#define GET_SRP_SECRET_INPUT_PARAMS \
{\
    {\
        "UPN",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
}

#define GET_SRP_SECRET_OUTPUT_PARAMS \
{\
    {\
        "Return Code",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Secret Data Size",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Secret Data",\
        VMW_IPC_TYPE_BLOB,\
        {NULL}\
    }\
}

#define GET_SERVER_STATE_OUTPUT_PARAMS \
{\
    {\
        "Return Code",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Server State",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
}

#define SET_SRP_SECRET_INPUT_PARAMS \
{\
    {\
        "UPN",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
    {\
        "Secret",\
        VMW_IPC_TYPE_WSTRING,\
        {NULL}\
    },\
}

#define GENERATE_PASSWORD_OUTPUT_PARAMS \
{\
    {\
        "Return Code",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Password Data Length",\
        VMW_IPC_TYPE_UINT32,\
        {NULL}\
    },\
    {\
        "Password Data",\
        VMW_IPC_TYPE_BLOB,\
        {NULL}\
    }\
}

#endif /* __TYPE_SPEC_H__ */
