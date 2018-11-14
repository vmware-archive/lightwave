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

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief    Default Lightwave AuthZ Access Check Function
 *
 * @details  LwCAAuthZLWCheckAccess is the entry point to authorize a request
 *           against the default Lightwave policy.
 *
 * @param    pReqCtx holds information about the requestor
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct
 * @param    apiPermissions indicates what API permissions to authorize the request
 *           against.
 * @param    pbAuthorized returns a boolean to indicate if the user is authorized
 *
 * @return   DWORD indicating function success/failure
 */
DWORD
LwCAAuthZLWCheckAccess(
    PLWCA_REQ_CONTEXT               pReqCtx,                // IN
    PCSTR                           pcszCAId,               // IN
    X509_REQ                        *pX509Request,          // IN
    LWCA_AUTHZ_API_PERMISSION       apiPermissions,         // IN
    PBOOLEAN                        pbAuthorized            // OUT
    );

/**
 * @brief    Default Lightwave Access Check for Create CA
 *
 * @details  LwCAAuthZLWCheckCACreate will determine if a requestor is allowed to
 *           create a CA.  The default Lightwave rules are:
 *               - Must be a member of either CAAdmins or CAOperators
 *
 * @param    pReqCtx holds information about the requestor
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct
 * @param    pbAuthorized returns a boolean to indicate if the user is authorized
 *
 * @return   DWORD indicating function success/failure
 */
DWORD
LwCAAuthZLWCheckCACreate(
    PLWCA_REQ_CONTEXT       pReqCtx,                // IN
    PCSTR                   pcszCAId,               // IN
    X509_REQ                *pX509Request,          // IN
    PBOOLEAN                pbAuthorized            // OUT
    );

/**
 * @brief    Default Lightwave Access Check for Revoke CA
 *
 * @details  LwCAAuthZLWCheckCARevoke will determine if a requestor is allowed to
 *           revoke a CA.  The default Lightwave rules are:
 *               - Must be a member of CAAdmins group
 *
 * @param    pReqCtx holds information about the requestor
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct
 * @param    pbAuthorized returns a boolean to indicate if the user is authorized
 *
 * @return   DWORD indicating function success/failure
 */
DWORD
LwCAAuthZLWCheckCARevoke(
    PLWCA_REQ_CONTEXT       pReqCtx,                // IN
    PCSTR                   pcszCAId,               // IN
    X509_REQ                *pX509Request,          // IN
    PBOOLEAN                pbAuthorized            // OUT
    );

/**
 * @brief    Default Lightwave Access Check for CSR
 *
 * @details  LwCAAuthZLWCheckCSR will determine if a requestor is allowed to
 *           submit a CSR.  The default Lightwave rules are:
 *               - Any Lightwave user in the same tenant as requested in the request
 *
 * @param    pReqCtx holds information about the requestor
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct
 * @param    pbAuthorized returns a boolean to indicate if the user is authorized
 *
 * @return   DWORD indicating function success/failure*
 */
DWORD
LwCAAuthZLWCheckCSR(
    PLWCA_REQ_CONTEXT       pReqCtx,                // IN
    PCSTR                   pcszCAId,               // IN
    X509_REQ                *pX509Request,          // IN
    PBOOLEAN                pbAuthorized            // OUT
    );

/**
 * @brief    Default Lightwave Access Check for CRL
 *
 * @details  LwCAAuthZLWCheckCRL will determine if a requestor is allowed to
 *           submit a CRL.  The default Lightwave rules are:
 *               - Must be a member of CAAdmins
 *
 * @param    pReqCtx holds information about the requestor
 * @param    pcszCAId is CA that the request is for.
 * @param    pX509Request is the request represented as the openssl X509_REQ struct
 * @param    pbAuthorized returns a boolean to indicate if the user is authorized
 *
 * @return   DWORD indicating function success/failure*
 */
DWORD
LwCAAuthZLWCheckCRL(
    PLWCA_REQ_CONTEXT       pReqCtx,                // IN
    PCSTR                   pcszCAId,               // IN
    X509_REQ                *pX509Request,          // IN
    PBOOLEAN                pbAuthorized            // OUT
    );


#ifdef __cplusplus
}
#endif
