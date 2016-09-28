/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

DWORD
VMCALoginUserPrivate(
    PCSTR pszUserName,
    const char* pszPassword
    )
/*
VMCALoginUser Creates a TGT cache for the Process to
communicate with the VMCA
*/
{
    DWORD dwError = 0;
    krb5_context context = NULL;
    krb5_get_init_creds_opt *opt = NULL;
    krb5_principal principal = NULL;
    krb5_ccache ccache = NULL;
    krb5_creds creds = { 0 };
    krb5_error_code err_code = 0;
#ifdef DEBUG_KRB
    PSTR pszErrstr = NULL;
#endif

    err_code = krb5_init_context(&context);
    BAIL_ON_ERROR(err_code);

    // it is assumed that pszUserName is in user@REALM format.
    err_code = krb5_parse_name_flags(
                                    context,
                                    pszUserName,
                                    KRB5_PRINCIPAL_PARSE_REQUIRE_REALM,
                                    &principal);
    BAIL_ON_ERROR(err_code);

    // Find the default cred cache
    err_code = krb5_cc_default(context, &ccache);
    BAIL_ON_ERROR(err_code);

#ifdef DEBUG_KRB
    printf("Default Cache Name is : %s\n", krb5_cc_default_name(context));
#endif

    err_code = krb5_get_init_creds_opt_alloc(context, &opt);
    BAIL_ON_ERROR(err_code);

    krb5_get_init_creds_opt_set_tkt_life(opt, 2 * 60 * 60); //2 hours ticket
    krb5_get_init_creds_opt_set_forwardable(opt, 1);

    // Let us get the Creds from the Kerberos Server

    err_code =  krb5_get_init_creds_password(
                        context,
                        &creds,
                        principal,
                        (PSTR) pszPassword,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        opt);
    BAIL_ON_ERROR(err_code);

    err_code = krb5_cc_store_cred(context, ccache, &creds);
    if ( err_code == KRB5_FCC_NOFILE){
        krb5_cc_initialize(context, ccache, principal);
        err_code = krb5_cc_store_cred(context, ccache, &creds);
        BAIL_ON_ERROR(err_code);
    }
    BAIL_ON_ERROR(err_code);
cleanup:

    if(principal != NULL) {
        krb5_free_principal(context, principal);
    }

    if(opt != NULL){
        krb5_get_init_creds_opt_free(context,opt);
    }

    if ( ccache != NULL) {
        krb5_cc_close(context, ccache);
    }

    krb5_free_cred_contents(context, &creds);

    if(context != NULL){
        krb5_free_context(context);
    }

    return dwError;
error :
#ifdef DEBUG_KRB
        pszErrstr   = (PSTR) krb5_get_error_message(context, err_code);
        printf("Error code : %d\n", err_code);
        printf("Error Message : %s\n", pszErrstr);
        krb5_free_error_message(context, (const char *)pszErrstr);
#endif
    dwError = err_code;
    goto cleanup;

}



DWORD
VMCALogOutPrivate(
    )
/*
    Executes Kdestroy and clears out the TGT cache.
    This is equivalent to kdestroy
*/
{
    DWORD dwError = 0;
    krb5_context context = NULL;
    krb5_ccache ccache = NULL;

    dwError = krb5_init_context(&context);
    BAIL_ON_ERROR(dwError);

    dwError = krb5_cc_default(
                context,
                &ccache);
    BAIL_ON_ERROR(dwError);

    krb5_cc_destroy(context, ccache);


error:
    if(context != NULL){
        krb5_free_context(context);
    }
    return dwError;
}


