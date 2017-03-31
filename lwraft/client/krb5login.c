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

#define KRB5_CC_NAME   "KRB5CCNAME"
#define KRB5_CONF_PATH "KRB5_CONFIG"

static
DWORD
VmDirMapKrbError(
    int krbError
    );

/*
 * VmDirLoginUser Creates a TGT cache for the Process to
 * communicate with the VmDir
*/
DWORD
VmDirKrb5LoginUser(
    PCSTR pszUserName,
    PCSTR pszPassword,
    PCSTR pszKrb5ConfPath /* Optional */
    )
{
    DWORD dwError = 0;
    krb5_context context = NULL;
    krb5_get_init_creds_opt *opt = NULL;
    krb5_principal principal = NULL;
    krb5_ccache ccache = NULL;
    krb5_creds creds = { 0 };
    PSTR pszCacheName = NULL;

    if (!IsNullOrEmptyString(pszKrb5ConfPath))
    {
        setenv(KRB5_CONF_PATH, pszKrb5ConfPath, 1);
    }

    dwError = VmDirMapKrbError(
                    krb5_init_context(&context)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMapKrbError(
                    krb5_get_init_creds_opt_alloc(context, &opt)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    krb5_get_init_creds_opt_set_tkt_life(opt, 8 * 60 * 60); //8 hours ticket
    krb5_get_init_creds_opt_set_forwardable(opt, 1);

    // Creates a File based Credential cache based on defaults
    dwError = VmDirMapKrbError(
                    krb5_cc_new_unique(
                        context,
                        "FILE",
                        "hint",
                        &ccache)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // it is assumed that pszUserName is in user@REALM format.
    dwError = VmDirMapKrbError(
                    krb5_parse_name(context, pszUserName, &principal)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    // Let us get the Creds from the Kerberos Server
    dwError =  VmDirMapKrbError(
                    krb5_get_init_creds_password(
                        context,
                        &creds,
                        principal,
                        (PSTR) pszPassword,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        opt)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMapKrbError(
                    krb5_cc_initialize(context, ccache, principal)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirMapKrbError(
                    krb5_cc_store_cred(context, ccache, &creds)
                    );
    BAIL_ON_VMDIR_ERROR(dwError);

    pszCacheName = (PSTR)krb5_cc_get_name(context, ccache);
    if ( pszCacheName == NULL)
    {
        dwError = ERROR_NO_CRED_CACHE_NAME;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // let us set the Value to the Env. Variable so GSSAPI can find it.
    setenv(KRB5_CC_NAME, pszCacheName, 1);

    krb5_cc_set_default_name(context, pszCacheName);

error:

    if (principal != NULL)
    {
        krb5_free_principal(context, principal);
    }

    if (opt != NULL)
    {
        krb5_get_init_creds_opt_free(context,opt);
    }

    krb5_free_cred_contents(context, &creds);

    if (context != NULL)
    {
        krb5_free_context(context);
    }

    return dwError;
}

/*
 * VmDirLogOut Clears the TGT Cache that was created for this user
*/
DWORD
VmDirKrb5LogOut(
    VOID
    )
{
    DWORD dwError = 0;
    PSTR pszCacheName = NULL;

    pszCacheName = getenv("KRB5CCNAME");

    if (pszCacheName == NULL)
    {
        return ERROR_NO_CRED_CACHE_FOUND;
    }
    unlink(pszCacheName);
    unsetenv(KRB5_CC_NAME);
    unsetenv(KRB5_CONF_PATH);
    return dwError;
}

static
DWORD
VmDirMapKrbError(
    int krbError
    )
{
    DWORD dwError = 0;

    if (krbError != 0)
    {
        // TODO: Map proper error codes
        switch ( krbError )
        {
            case KRB5_KDC_UNREACH :
            case KRB5_PREAUTH_FAILED :
            case KRB5_LIBOS_PWDINTR :
            case KRB5_REALM_CANT_RESOLVE :
            case KRB5KDC_ERR_KEY_EXP :
            case KRB5_LIBOS_BADPWDMATCH :
            case KRB5_CHPW_PWDNULL :
            case KRB5_CHPW_FAIL :
            default:
                 dwError = ERROR_KERBEROS_ERROR;
        }
    }

    return dwError;
}

