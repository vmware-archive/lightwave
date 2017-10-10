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
int
postdb_auxprop_lookup (
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

    return SASL_OK;
}

static
sasl_auxprop_plug_t
postdb_auxprop_plugin =
{
    0,
    0,
    NULL,
    NULL,
    postdb_auxprop_lookup,
    "postdb",
    NULL
};

int
postdb_auxprop_plug_init (
    const sasl_utils_t *     utils,
    int                      max_version,
    int *                    out_version,
    sasl_auxprop_plug_t **   plug,
    const char *             plugname
    )
{

    *out_version = SASL_AUXPROP_PLUG_VERSION;
    *plug = &postdb_auxprop_plugin;

    return SASL_OK;
}
