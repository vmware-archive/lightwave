/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

typedef struct _LWCA_TEST_STATE
{
    PLWCA_DB_FUNCTION_TABLE pFunctionTable;
    PLWCA_PLUGIN_HANDLE     pPluginHandle;
    PLWCA_DB_HANDLE         pDbHandle;
} LWCA_TEST_STATE, *PLWCA_TEST_STATE;

typedef DWORD (*PLUGIN_ADD_CA)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PLWCA_DB_CA_DATA,
    PCSTR
    );

typedef DWORD (*SERIALIZE_CA_JSON)(
    PCSTR,
    PLWCA_DB_CA_DATA,
    PCSTR,
    PCSTR,
    PSTR *
    );

typedef DWORD (*SERIALIZE_CONFIG_CA_JSON)(
    PCSTR,
    PCSTR,
    PSTR *
    );

typedef DWORD (*DESERIALIZE_JSON_CA)(
    PCSTR,
    PLWCA_DB_CA_DATA *
    );

typedef DWORD (*PLUGIN_CHECK_CA)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PBOOLEAN
    );

typedef DWORD (*PLUGIN_GET_CA)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PLWCA_DB_CA_DATA *
    );

typedef DWORD (*PLUGIN_UPDATE_CA)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PLWCA_DB_CA_DATA
    );

typedef DWORD (*PLUGIN_UPDATE_CA_REQ_BODY)(
    PLWCA_DB_CA_DATA,
    PSTR *
    );

typedef DWORD (*PLUGIN_GET_PARENT_CA)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PSTR *
    );

typedef DWORD (*JSON_GET_STRING_ATTR)(
    PCSTR,
    PCSTR,
    PSTR *
    );

typedef DWORD (*SERIALIZE_CERT_DATA_JSON)(
    PCSTR,
    PLWCA_DB_CERT_DATA,
    PCSTR,
    PSTR *
    );

typedef DWORD (*PLUGIN_ADD_CERT)(
    PLWCA_DB_HANDLE,
    PCSTR,
    PLWCA_DB_CERT_DATA
    );
