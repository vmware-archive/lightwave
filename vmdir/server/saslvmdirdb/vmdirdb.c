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


// Cyrus SASL changed the type for auxprop_lookup at some point after 2.1.23,
// but I don't know a good way to handle the type change. Using this for now.
#define VMDIR_CYRUS_SASL_VERSION_IS_OLD 1
#if SASL_VERSION_MAJOR > 2
   #undef VMDIR_CYRUS_SASL_VERSION_IS_OLD
#else
   #if SASL_VERSION_MINOR > 1
   #undef VMDIR_CYRUS_SASL_VERSION_IS_OLD
   #else
      #if SASL_VERSION_STEP > 23
      #undef VMDIR_CYRUS_SASL_VERSION_IS_OLD
      #endif
   #endif
#endif


// this name matches with srp.c srp_server_mech_step1 SASL_AUX_PASSWORD property name
const char* gSRPPasswdPropertyName = "*cmusaslsecretSRP";

#ifndef _WIN32
extern
unsigned int
VmDirSRPGetIdentityData(
    const char*         pszUPN,
    unsigned char**     ppByteSecret,
    unsigned int*       pdwSecretLen
    );
#else
PFSRPGETFUN gpfGetSrpSecret = NULL;
#endif

unsigned int
_VmDirSRPGetIdentityData(
    const char*         pszUPN,
    unsigned char**     ppByteSecret,
    unsigned int*       pdwSecretLen
    )
{
    unsigned int dwError = 0;
#ifndef _WIN32
    dwError = VmDirSRPGetIdentityData(pszUPN, ppByteSecret, pdwSecretLen);
#else
    if (gpfGetSrpSecret)
    {
        dwError = gpfGetSrpSecret(pszUPN, ppByteSecret, pdwSecretLen);
    }
    else
    {
        dwError = VMDIR_ERROR_ENTRY_NOT_FOUND;
    }

#endif
    return dwError;
}

///////////////////////////////////////////////////////////////////////////////////////
// likewise and vmware-cyrus-sasl use different versions of SASL. Thus incompatible API
///////////////////////////////////////////////////////////////////////////////////////
static
#ifndef VMDIR_CYRUS_SASL_VERSION_IS_OLD
int
#else
void
#endif
vmdirdb_auxprop_lookup (
    void*                   glob_context,
    sasl_server_params_t*   pSrvParams,
    unsigned                flags,
    const char *            pszUserID,
    unsigned                ulen
    )
{

    int             iSASLRtn = 0;
    unsigned char*  pSecret = NULL;
    unsigned int    iSecretLen = 0;
    const struct propval*   pVal = NULL;

    pVal = pSrvParams->utils->prop_get(pSrvParams->propctx);
    if (pVal != NULL)
    {
        int iIdx = 0;

        for ( iIdx = 0; pVal[iIdx].name != NULL; iIdx++ )
        {
            if (strcmp(gSRPPasswdPropertyName, pVal[iIdx].name) == 0)
            {
                if (_VmDirSRPGetIdentityData( pszUserID, &pSecret, &iSecretLen ) == 0)
                {
                    // if have value, erase it then add new value back
                    pSrvParams->utils->prop_erase( pSrvParams->propctx, pVal[iIdx].name );

                    iSASLRtn = pSrvParams->utils->prop_set(
                                                pSrvParams->propctx,
                                                pVal[iIdx].name,
                                                pSecret,
                                                iSecretLen);
                    // SASL code ldapdb.c/sasldb.c ignore this return code
                    // If this property is not set, subsequent srp code treats it as
                    //     user not found.
                }

                break;
            }
        }
    }

    if ( pSecret )
    {
        free(pSecret);
    }
#ifndef VMDIR_CYRUS_SASL_VERSION_IS_OLD
    return SASL_OK;
#endif
}

static
sasl_auxprop_plug_t
vmdirdb_auxprop_plugin =
{
    0,
    0,
    NULL,
    NULL,
    vmdirdb_auxprop_lookup,
    "vmdirdb",
    NULL
};

int
vmdirdb_auxprop_plug_init (
    const sasl_utils_t *     utils,
    int                      max_version,
    int *                    out_version,
    sasl_auxprop_plug_t **   plug,
    const char *             plugname
    )
{

    *out_version = SASL_AUXPROP_PLUG_VERSION;
    *plug = &vmdirdb_auxprop_plugin;

    return SASL_OK;
}
