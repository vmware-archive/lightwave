/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#pragma once

//default name for unix creds lib
#define GSSAPI_UNIX_CREDS_DEFAULT_SO         "libgssapi_unix_creds.so"

//env variable to override default unix creds lib
#define GSSAPI_UNIX_CREDS_OVERRIDE           "GSSAPI_UNIX_CREDS_OVERRIDE"

//default name for unix creds lib that will provide privilege separation
//for processes that use gssapi_unix without privileges for shadow apis
#define GSSAPI_UNIX_PRIVSEP_CREDS_DEFAULT_SO "libgssapi_unix_privsep_creds.so"

//env variable to override default unix privilege separation creds lib
#define GSSAPI_UNIX_PRIVSEP_CREDS_OVERRIDE   "GSSAPI_UNIX_PRIVSEP_CREDS_OVERRIDE"

//default name for srp creds lib
#define GSSAPI_SRP_CREDS_DEFAULT_SO         "libgssapi_srp_creds.so"

//env variable to override default srp creds lib
#define GSSAPI_SRP_CREDS_OVERRIDE           "GSSAPI_SRP_CREDS_OVERRIDE"

//default name for srp creds lib that will provide privilege separation
//for processes that use gssapi_srp without privileges for shadow apis
#define GSSAPI_SRP_PRIVSEP_CREDS_DEFAULT_SO "libgssapi_unix_privsep_creds.so"

//env variable to override default srp rivilege separation creds lib
#define GSSAPI_SRP_PRIVSEP_CREDS_OVERRIDE   "GSSAPI_SRP_PRIVSEP_CREDS_OVERRIDE"

//supported types. used to lookup creds lib
typedef enum
{
    PLUGIN_TYPE_MIN = -1,
    PLUGIN_TYPE_UNIX,
    PLUGIN_TYPE_SRP,
    //Add new types before PLUGIN_TYPE_MAX
    PLUGIN_TYPE_MAX
}GSSAPI_PLUGIN_TYPE;

//creds lib implements this function.
//takes in plugin type, username,
//returns salt and srp s and v values.
typedef
int
(*PFN_GET_HASHED_CREDS)(
    int plugin_type,
    const char *user_name,
    char **ret_salt,
    unsigned char **ret_bytes_s,
    int *ret_len_s,
    unsigned char **ret_bytes_v,
    int *ret_len_v
    );

typedef struct _CREDS_PLUGIN_INTERFACE_
{
    PFN_GET_HASHED_CREDS pfnGetHashedCreds;
}CREDS_PLUGIN_INTERFACE, *PCREDS_PLUGIN_INTERFACE;

//Function name defs
#define CREDS_PLUGIN_LOAD_INTERFACE   "creds_plugin_load_interface"
#define CREDS_PLUGIN_UNLOAD_INTERFACE "creds_plugin_unload_interface"

//load
typedef int
(*PFN_CREDS_PLUGIN_LOAD_INTERFACE)(
    PCREDS_PLUGIN_INTERFACE *ppCredsInterface
    );

//unload
typedef int
(*PFN_CREDS_PLUGIN_UNLOAD_INTERFACE)(
    PCREDS_PLUGIN_INTERFACE pCredsInterface
    );

typedef struct _CREDS_PLUGIN_
{
    //dlopen handle
    void *pHandle;

    //Mandatory entry point
    PFN_CREDS_PLUGIN_LOAD_INTERFACE pfnLoad;
    //Optional unload
    PFN_CREDS_PLUGIN_UNLOAD_INTERFACE pfnUnload;

    //interface fn table returned by load.
    PCREDS_PLUGIN_INTERFACE pInterface;
}CREDS_PLUGIN, *PCREDS_PLUGIN;



int
get_hashed_creds(
    int nPluginType,
    const char *username,
    char **ret_salt,
    unsigned char **ret_bytes_s,
    int *ret_len_s,
    unsigned char **ret_bytes_v,
    int *ret_len_v
    );
