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

#ifndef _MUTENTCA_AUTHZ_H_
#define _MUTENTCA_AUTHZ_H_

#ifdef __cplusplus
extern "C" {
#endif


#define     LWCA_CAADMINS_GROUP         "CAAdmins"
#define     LWCA_CAOPERATORS_GROUP      "CAOperators"

/*
 * Indicates the API which requires an authorization check
 *     - LWCA_AUTHZ_CA_CREATE_PERMISSION:    AuthZ check for CA creation API
 *     - LWCA_AUTHZ_CA_REVOKE_PERMISSION:    AuthZ check for CA revoke API
 *     - LWCA_AUTHZ_CSR_PERMISSION:          AuthZ check for CSR API
 *     - LWCA_AUTHZ_CRL_PERMISSION:          Authz check for CRL API
 */
typedef enum _LWCA_AUTHZ_API_PERMISSION
{
    LWCA_AUTHZ_ALLOW_PERMISSION          = 0x0,
    LWCA_AUTHZ_CA_CREATE_PERMISSION      = 0x1,
    LWCA_AUTHZ_CA_REVOKE_PERMISSION      = 0x2,
    LWCA_AUTHZ_CSR_PERMISSION            = 0x4,
    LWCA_AUTHZ_CRL_PERMISSION            = 0x8
} LWCA_AUTHZ_API_PERMISSION;


/**
 * @brief    Initialize AuthZ Plugin
 *
 * @details  PFN_AUTHZ_PLUGIN_INITIALIZE is the signature for a plugin's function.
 *           This function will load the specified config file and extract config
 *           values from it.
 *           This function is not required for a plugin--if it is not implemented
 *           in a plugin, the MutentCA AuthZ service context will not call it during
 *           plugin load.
 *
 * @param    pcszConfigPath is the path to the JSON config file for the plugin.
 *
 * @return   DWORD indicating function success/failure
 */
typedef DWORD
(*PFN_LWCA_AUTHZ_PLUGIN_INIT)(
    PCSTR       pcszConfigPath
    );

/**
 * @brief    Retrieve AuthZ Plugin Version
 *
 * @details  PFN_LWCA_AUTHZ_GET_VERSION is the signature for a plugin's get
 *           version API.
 *
 * @param    VOID
 *
 * @return   PCSTR indicating the plugin version
 */
typedef PCSTR
(*PFN_LWCA_AUTHZ_GET_VERSION)(
    VOID
    );

/**
 * @brief    Retrieve a Human Readable Message for a Plugin Error
 *
 * @details  PFN_LWCA_AUTHZ_ERROR_TO_STRING is the signature for a plugin function
 *           which converts a plugin error code to a human readable string.
 *
 * @param    dwErrorCode is the plugin error code
 *
 * @return   PCSTR representing the human readable error information
 */
typedef PCSTR
(*PFN_LWCA_AUTHZ_ERROR_TO_STRING)(
    DWORD       dwErrorCode            // IN
    );

/**
 * @brief    Plugin Check Access Function that Service AuthZ Layer Calls
 *
 * @details  PFN_LWCA_AUTHZ_CHECK_ACCESS is the signature for a plugin function
 *           which will determine is a requestor is authorized to run an API.
 *
 * @param    pReqCtx is the MutentCA request context, which holds requestor info.
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct.
 * @param    apiPermissions indicates what API permissions to authorize the request
 *           against.
 * @param    pbAuthorized will be filled with true/false notifying the caller if
 *           request is allowed to make the API call.
 *
 * @return   DWORD indicating function success/failure
 */
typedef DWORD
(*PFN_LWCA_AUTHZ_CHECK_ACCESS)(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    PCSTR                           pcszCAId,               // IN
    X509_REQ                        *pX509Request,          // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    );

typedef struct _LWCA_AUTHZ_FUNCTION_TABLE
{
    PFN_LWCA_AUTHZ_GET_VERSION          pfnAuthZGetVersion;
    PFN_LWCA_AUTHZ_ERROR_TO_STRING      pfnAuthZErrorToString;
    PFN_LWCA_AUTHZ_CHECK_ACCESS         pfnAuthZCheckAccess;
    PFN_LWCA_AUTHZ_PLUGIN_INIT          pfnAuthZPluginInit;
} LWCA_AUTHZ_FUNCTION_TABLE, *PLWCA_AUTHZ_FUNCTION_TABLE;


#ifdef __cplusplus
}
#endif

#endif /* _MUTENTCA_AUTHZ_H_ */
