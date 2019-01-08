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

#ifndef __MUTENTCA_SRV_AUTHZ_H_
#define __MUTENTCA_SRV_AUTHZ_H_

#ifdef __cplusplus
extern "C" {
#endif

#define LWCA_CONFIG_AUTHZ_KEY               "authorization"
#define LWCA_AUTHZ_PLUGIN_PATH_KEY          "pluginPath"
#define LWCA_AUTHZ_PLUGIN_CONFIG_PATH_KEY   "pluginConfigPath"


/**
 * @brief    Initialize Authorization Context
 *
 * @details  LwCAAuthZInitialize parses its JSON config to see if it should load
 *           a plugin.  If the config does not have a path to an AuthZ plugin,
 *           then the default AuthZ rules for Lightwave will be used.
 *
 *           Example JSON config:
 *               {
 *                  "pluginPath": "/opt/vmware/share/lib64/mutentcaplugins/libauthz.so",
 *                  "pluginConfigPath": "/opt/vmware/share/config/mutentcaauthz.json"
 *               }
 *
 * @param    pJsonConfig holds the JSON config for the AuthZ layer.
 *
 * @return   DWORD indicating function success/failure.
 */
DWORD
LwCAAuthZInitialize(
    PLWCA_JSON_OBJECT       pJsonConfig         // IN
    );

/**
 * @brief    Entry Point for APIs to Check if Requestor is Authorized to Call It
 *
 * @details  LwCAAuthZCheckAccess will pass the request context (caller
 *           information) and the PKCS10 request blob to either the loaded
 *           plugin's or the defualt Lightwave AuthZ check function, for the
 *           appropriate API operation type.
 *
 * @param    pReqCtx is the MutentCA request context, which holds requestor info.
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Data is a wrapper which holds a pointer to the X509 data to use
 *           to authorize the API
 * @param    apiPermissions indicates what API permissions to authorize the request
 *           against.
 * @param    pbAuthorized will be filled with true/false notifying the caller if
 *           request is allowed to make the API call.
 *
 * @return   DWORD indicating function success/failure
 */
DWORD
LwCAAuthZCheckAccess(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    PCSTR                           pcszCAId,               // IN
    LWCA_AUTHZ_X509_DATA            *pX509Data,             // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    );

/**
 * @brief    Destroys the Authorization Context
 *
 * @details  LwCAAuthZDestroy is to be called during MutentCA service shutdown.
 *           It will unload the AuthZ plugin--if there is one--and then free all
 *           memory that it allocated.
 *
 * @param    VOID
 *
 * @return   VOID
 */
VOID
LwCAAuthZDestroy(
    VOID
    );


#ifdef __cplusplus
}
#endif

#endif /* __MUTENTCA_SRV_AUTHZ_H_ */
