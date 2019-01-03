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


#include "includes.h"

static
DWORD
InitializeLog(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    );

static
VOID
LwCAEnableSuidDumpable(
    VOID
    ) __attribute__((unused));


// init global/static in single thread mode during server startup
DWORD
LwCAInitialize(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    )
{
    DWORD dwError = 0;

    //LwCAEnableSuidDumpable();

    //dwError = LwCACommonInit();
    //BAIL_ON_LWCA_ERROR(dwError);

    dwError = InitializeLog(FALSE, FALSE);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = OidcClientGlobalInit();
    BAIL_ON_LWCA_ERROR(dwError);

    // TODO: When final implementation is in place, don't bail on Error , this
    // just sets up the current state
    dwError = LwCASrvInitCA();
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAMetricsInitialize();
    BAIL_ON_LWCA_ERROR(dwError);

#ifdef REST_ENABLED
    dwError = LwCARestServerInit();
    BAIL_ON_LWCA_ERROR(dwError);
#endif

error:

    return dwError;
}

VOID
LwCAShutdown(
    VOID
    )
{
    /*
     * temporarily disable oidc global cleanup
     * there appears to be conflicting global free
     * issues with openssl global cleanup in oidc
     * and security plugin's openssl cleanup
    */
    //OidcClientGlobalCleanup();

#ifdef REST_ENABLED
    if (LwCARestServerStop() == 0)
    {
        LwCARestServerShutdown();
    }
#endif
    LwCAMetricsShutdown();
    LwCAAuthZDestroy();
    LwCASecurityFreeCtx();
    LwCAPolicyFreeCtx(gpPolicyCtx);
    LwCADbFreeCtx();
    LwCAFreeCACtx();
    //LwCAServiceShutdown();
    LwCATerminateLogging();
    //LwCASrvCleanupGlobalState();
    //LwCACommonShutdown();
}

static
DWORD
InitializeLog(
    BOOL overrideLogLevel,
    BOOL overrideLogType
    )
{
    return LwCAInitLog();
}

DWORD
LwCASrvInitCA(
    VOID
    )
{
    DWORD dwError = 0;
    PLWCA_JSON_OBJECT pJsonConfig = NULL;
    PLWCA_JSON_OBJECT pAuthZConfig = NULL;
    PLWCA_JSON_OBJECT pSecurityConfig = NULL;
    PLWCA_JSON_OBJECT pPolicyConfig = NULL;
    PCSTR pcszPolicyConfigPath = NULL;
    PLWCA_JSON_OBJECT pPolicyConfigContent = NULL;
    PLWCA_JSON_OBJECT pCAConfig = NULL;
    PLWCA_JSON_OBJECT pDbConfig = NULL;

    dwError = LwCAConfigLoadFile(LWCA_CONFIG_FILE_PATH, &pJsonConfig);
    if (dwError == LWCA_JSON_FILE_LOAD_ERROR)
    {
        LWCA_LOG_INFO(
                "[%s,%d] Failed to open LwCA config file (%s). Service cannot start without config...",
                __FUNCTION__,
                __LINE__,
                LWCA_CONFIG_FILE_PATH);
    }
    BAIL_ON_LWCA_ERROR(dwError);

    /* get the authorization config section */
    dwError = LwCAJsonGetObjectFromKey(
                  pJsonConfig,
                  TRUE,
                  LWCA_CONFIG_AUTHZ_KEY,
                  &pAuthZConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAAuthZInitialize(pAuthZConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    /* get the security config section */
    dwError = LwCAJsonGetObjectFromKey(
                  pJsonConfig,
                  FALSE,
                  LWCA_SECURITY_PLUGIN_KEY,
                  &pSecurityConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCASecurityInitCtx(pSecurityConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    /* get the policy config section */
    dwError = LwCAJsonGetObjectFromKey(
                  pJsonConfig,
                  FALSE,
                  LWCA_CONFIG_POLICY_KEY,
                  &pPolicyConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAJsonGetConstStringFromKey(
                  pPolicyConfig,
                  FALSE,
                  LWCA_CONFIG_POLICY_CONFIG_PATH_KEY,
                  &pcszPolicyConfigPath);

    dwError = LwCAJsonLoadObjectFromFile(
                  pcszPolicyConfigPath,
                  &pPolicyConfigContent);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAPolicyInitCtx(pPolicyConfigContent, &gpPolicyCtx);
    BAIL_ON_LWCA_ERROR(dwError);

    /* get the db config section */
    dwError = LwCAJsonGetObjectFromKey(
                  pJsonConfig,
                  FALSE,
                  LWCA_DB_CONFIG_KEY,
                  &pDbConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCADbInitCtx(pDbConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    /* get the root CA config section */
    dwError = LwCAJsonGetObjectFromKey(
                  pJsonConfig,
                  FALSE,
                  LWCA_CA_CONFIG_KEY,
                  &pCAConfig);
    BAIL_ON_LWCA_ERROR(dwError);

    dwError = LwCAInitCA(pCAConfig);
    BAIL_ON_LWCA_ERROR(dwError);

error:

    LwCAJsonCleanupObject(pJsonConfig);

    return dwError;
}

/*
 * Any process which has changed privilege levels will not be dumped.
 * Enable it explicitly
 */
static
VOID
LwCAEnableSuidDumpable(
    VOID
    )
{
    if (prctl(PR_SET_DUMPABLE, 1, 0, 0, 0) == -1)
    {
        LWCA_LOG_ERROR(
            "%s: coredumps will not be generated error: %d",
            __FUNCTION__,
            errno);
    }
}
