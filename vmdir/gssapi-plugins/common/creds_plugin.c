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

#include "includes.h"

//free plugin struct
static
void
free_creds_plugin(
    PCREDS_PLUGIN pPlugin
    )
{
    if(!pPlugin)
    {
        return;
    }
    if(pPlugin->pHandle)
    {
        dlclose(pPlugin->pHandle);
    }
    free(pPlugin);
}

/*
 Load credentials plugin and initialize interface.
*/
static
int
load_creds_plugin(
    const char *creds_plugin,
    PCREDS_PLUGIN *ppPlugin
    )
{
    int sts = 0;
    char *error_str = NULL;
    PCREDS_PLUGIN pPlugin = NULL;

    if(!creds_plugin || !ppPlugin)
    {
        sts = EINVAL;
        goto error;
    }

    srp_debug_printf("load_creds_plugin : %s\n", creds_plugin);

    pPlugin = (PCREDS_PLUGIN)malloc(sizeof(CREDS_PLUGIN));
    if(!pPlugin)
    {
        sts = ENOMEM;
        goto error;
    }

    //clear error
    dlerror();

    pPlugin->pHandle = dlopen(creds_plugin, RTLD_NOW);
    if(!pPlugin->pHandle)
    {
        sts = ENOENT;
        goto error;
    }

    pPlugin->pfnLoad = dlsym(pPlugin->pHandle, CREDS_PLUGIN_LOAD_INTERFACE);
    if(!pPlugin->pfnLoad)
    {
        sts = ENOSYS;
        goto error;
    }

    sts = pPlugin->pfnLoad(&pPlugin->pInterface);
    if(sts)
    {
        goto error;
    }

    *ppPlugin = pPlugin;

cleanup:
    return sts;

error:
    srp_debug_printf("creds plugin error: %d\n", sts);

    if(ppPlugin)
    {
        *ppPlugin = NULL;
    }
    free_creds_plugin(pPlugin);

    error_str = dlerror();
    if(error_str)
    {
        srp_debug_printf("creds plugin load: %s\n", error_str);
    }
    goto cleanup;
}

int
get_creds_plugin_for_type(
    int plugin_type,
    const char **pcreds_plugin
    )
{
    int sts = 0;
    const char *creds_plugin = NULL;
    #define ACCESS_TYPES 2//privileged and non privileged
    const int PRIVILEGED = 0;
    const int NON_PRIVILEGED = 1;
    int access_type = PRIVILEGED;
    struct stPluginTable
    {
        const char *plugin_override_env;
        const char *default_plugin_name;
    }
    all_plugins_array[][ACCESS_TYPES] =
    {
        //privileged access path
        {
            {GSSAPI_UNIX_CREDS_OVERRIDE, GSSAPI_UNIX_CREDS_DEFAULT_SO},
            {GSSAPI_SRP_CREDS_OVERRIDE, GSSAPI_SRP_CREDS_DEFAULT_SO}
        },
        //non privileged access path
        {
            {GSSAPI_UNIX_PRIVSEP_CREDS_OVERRIDE,
             GSSAPI_UNIX_PRIVSEP_CREDS_DEFAULT_SO},
            {GSSAPI_SRP_PRIVSEP_CREDS_OVERRIDE,
             GSSAPI_SRP_PRIVSEP_CREDS_DEFAULT_SO}
        }
    };
    struct stPluginTable *plugins_array = NULL;

    if(!pcreds_plugin ||
       (plugin_type <= PLUGIN_TYPE_MIN || plugin_type >= PLUGIN_TYPE_MAX))
    {
        sts = EINVAL;
        goto error;
    }

    //determine which access type
    access_type = getuid() == 0 ? PRIVILEGED : NON_PRIVILEGED;
    plugins_array = all_plugins_array[access_type];

    srp_debug_printf(
        "creds plugin type: %s\n",
        access_type == PRIVILEGED ? "Privileged" : "Non Privileged");

    //If there is an override, use it
    creds_plugin = getenv(plugins_array[plugin_type].plugin_override_env);

    //if not, use the default.
    if(!creds_plugin)
    {
        creds_plugin = plugins_array[plugin_type].default_plugin_name;
    }

    if(!creds_plugin)
    {
        sts = EINVAL;
        goto error;
    }

    *pcreds_plugin = creds_plugin;

cleanup:
    return sts;

error:
    if(pcreds_plugin)
    {
        *pcreds_plugin = NULL;
    }
    goto cleanup;
}

/*
  Do implementation specific creds lookup and return
  salt and srp v and s values as applicable.
*/
int
get_hashed_creds(
    int plugin_type,
    const char *username,
    char **ret_salt,
    unsigned char **ret_bytes_s,
    int *ret_len_s,
    unsigned char **ret_bytes_v,
    int *ret_len_v
    )
{
    int sts = 0;
    char *username_salt = NULL;
    PCREDS_PLUGIN pPlugin = NULL;
    PCREDS_PLUGIN_INTERFACE pInterface = NULL;
    unsigned char *bytes_s = NULL;
    int len_s = 0;
    unsigned char *bytes_v = NULL;
    int len_v = 0;
    const char *creds_plugin = NULL;

    if(!username ||
       !ret_salt ||
       !ret_bytes_s ||
       !ret_len_s ||
       !ret_bytes_v ||
       !ret_len_v)
    {
        sts = -1;
        goto error;
    }

    sts = get_creds_plugin_for_type(plugin_type, &creds_plugin);
    if(sts)
    {
        goto error;
    }

    sts = load_creds_plugin(creds_plugin, &pPlugin);
    if(sts)
    {
        goto error;
    }

    pInterface = pPlugin->pInterface;

    if(!pInterface->pfnGetHashedCreds)
    {
        sts = -1;
        goto error;
    }

    sts = pInterface->pfnGetHashedCreds(
              plugin_type,
              username,
              &username_salt,
              &bytes_s,
              &len_s,
              &bytes_v,
              &len_v);
    if(sts)
    {
        goto error;
    }

    *ret_salt = username_salt;
    *ret_bytes_s = bytes_s;
    *ret_len_s = len_s;
    *ret_bytes_v = bytes_v;
    *ret_len_v = len_v;

cleanup:
    if(pPlugin)
    {
        free_creds_plugin(pPlugin);
    }
    return sts;
error:
    if(username_salt)
    {
        free(username_salt);
    }
    goto cleanup;
}
