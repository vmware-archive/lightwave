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

static
DWORD
_VmDirSASLGSSBind(
     LDAP*  pLD
     );

static
int
_VmDirSASLInteraction(
    LDAP *      pLD,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    );

static
void
_VmDirClientTestGSSAPIBind(
    PCSTR   pszLDAPURI
    );

static
void
_VmDirClientTestSimpleBind(
    PCSTR   pszLDAPURI
    );

static
void
_VmDirClientTestSimpleSSLBind(
    PCSTR   pszLDAPSURI
    );

void
TestVmDirSASLClient(
    void
    )
{
    DWORD   dwError = 0;
    char    pszServerHost[256] = {0};
    PSTR    pszLDAPURI = NULL;
    PSTR    pszLDAPSURI = NULL;

    printf( "Please entry LDAP server host:");
    scanf("%s", pszServerHost);

    dwError = VmDirAllocateStringAVsnprintf(    &pszLDAPURI,
                                                "ldap://%s:389",
                                                pszServerHost[0] != '\0' ? pszServerHost : "localhost");
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf(    &pszLDAPSURI,
                                                "ldaps://%s",
                                                pszServerHost[0] != '\0' ? pszServerHost : "localhost");
    BAIL_ON_VMDIR_ERROR(dwError);

    _VmDirClientTestSimpleBind( pszLDAPURI );

    _VmDirClientTestSimpleSSLBind( pszLDAPSURI );

    _VmDirClientTestGSSAPIBind( pszLDAPURI );


cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszLDAPURI);
    VMDIR_SAFE_FREE_MEMORY(pszLDAPSURI);

    return;

error:

	printf("TestVmDirSASLClient failed. (%d)\n", dwError);
	goto cleanup;
}

static
void
_VmDirClientTestSimpleBind(
    PCSTR   pszLDAPURI
    )
{
    DWORD   dwError = 0;
    int	    ldap_version_3 = LDAP_VERSION3;
    LDAP *  pLD = NULL;
    BerValue    ldapBindPwd = {0};

    printf("_VmDirClientTestSimpleBind ldap simple bind initialize %s\n", pszLDAPURI);
    dwError = ldap_initialize( &pLD, pszLDAPURI );
    /* Set LDAP V3 protocol version */
    ldap_set_option( pLD, LDAP_OPT_PROTOCOL_VERSION, &ldap_version_3 );

    printf("_VmDirClientTestSimpleBind ldap simple ANONYMOUS bind started.\n");
    dwError = ldap_sasl_bind_s(
                                   pLD,
                                   "",
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // no credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("_VmDirClientTestSimpleBind ldap simple ANONYMOUS bind succeeded.\n");

cleanup:
    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("_VmDirClientTestSimpleBind ldap unbind succeeded.\n\n\n");
    }

    return;

error:

	printf("_VmDirClientTestSimpleBind failed. (%d)(%s)\n\n", dwError, ldap_err2string(dwError));

	goto cleanup;
}

static
void
_VmDirClientTestSimpleSSLBind(
    PCSTR   pszLDAPSURI
    )
{
    DWORD   dwError = 0;
    int     ldap_version_3 = LDAP_VERSION3;
    int     iTLSNever = LDAP_OPT_X_TLS_NEVER;
    int     iTLSMin = LDAP_OPT_X_TLS_PROTOCOL_TLS1_0;
    LDAP *  pLD = NULL;
    BerValue    ldapBindPwd = {0};
    char    pszDefaultBindDN[256] = {0};
    char    pszDefaultPasswd[256] = {0};

    printf("_VmDirClientTestSimpleSSLBind ldaps simple bind initialize %s\n", pszLDAPSURI);
    dwError = ldap_initialize( &pLD, pszLDAPSURI );
    /* Set LDAP V3 protocol version */
    ldap_set_option( pLD, LDAP_OPT_PROTOCOL_VERSION, &ldap_version_3 );
    ldap_set_option(NULL, LDAP_OPT_X_TLS_PROTOCOL_MIN, &iTLSMin);
    ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &iTLSNever);

    printf("_VmDirClientTestSimpleSSLBind ldaps simple ANONYMOUS bind started.\n");
    dwError = ldap_sasl_bind_s(
                                   pLD,
                                   "",
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // no credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("_VmDirClientTestSimpleSSLBind ldaps simple ANONYMOUS bind succeeded.\n");

    printf( "\n\nPlease entry LDAPS Bind DN:");
    scanf("%s", pszDefaultBindDN);
    printf( "Please entry LDAPS Bind password:");
    scanf("%s", pszDefaultPasswd);

    ldapBindPwd.bv_val = pszDefaultPasswd;
    ldapBindPwd.bv_len = strlen( pszDefaultPasswd );

    printf("_VmDirClientTestSimpleSSLBind ldaps simple bind started.\n");

    dwError = ldap_sasl_bind_s(
                                   pLD,
                                   pszDefaultBindDN,
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // ldaps with credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("_VmDirClientTestSimpleSSLBind ldaps simple bind succeeded.\n");


cleanup:
    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("_VmDirClientTestSimpleSSLBind ldaps unbind succeeded.\n\n\n");
    }

    return;

error:

    printf("_VmDirClientTestSimpleSSLBind failed. (%d)(%s)\n\n", dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
void
_VmDirClientTestGSSAPIBind(
    PCSTR   pszLDAPURI
    )
{
    DWORD   dwError = 0;
    int	    ldap_version_3 = LDAP_VERSION3;
    LDAP *  pLD = NULL;

    printf("_VmDirClientTestGSSAPIBind ldap sasl GSSAPI bind initialize %s\n", pszLDAPURI);
    dwError = ldap_initialize( &pLD, pszLDAPURI );
    /* Set LDAP V3 protocol version */
    ldap_set_option( pLD, LDAP_OPT_PROTOCOL_VERSION, &ldap_version_3 );

    printf("_VmDirClientTestGSSAPIBind ldap sasl GSSAPI bind started.\n");
    dwError = _VmDirSASLGSSBind(pLD);
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("_VmDirClientTestGSSAPIBind ldap sasl GSSAPI bind succeeded.\n");

cleanup:
    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
        BAIL_ON_VMDIR_ERROR(dwError);

        printf("_VmDirClientTestGSSAPIBind ldap unbind succeeded.\n");
    }

    return;

error:

	printf("_VmDirClientTestGSSAPIBind failed. (%d)(%s)\n\n", dwError, ldap_err2string(dwError));

	goto cleanup;
}

static
DWORD
_VmDirSASLGSSBind(
     LDAP*  pLD
     )
{
    DWORD   dwError = 0;

    dwError = ldap_sasl_interactive_bind_s( pLD,
                                            NULL,
                                            "GSSAPI",
                                            NULL,
                                            NULL,
                                            LDAP_SASL_QUIET,
                                            _VmDirSASLInteraction,
                                            NULL);
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    return dwError;

error:

    VmDirLog(LDAP_DEBUG_ANY, "VmDirSASLGSSBind failed. (%d)(%s)\n", dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
int
_VmDirSASLInteraction(
    LDAP *      pLD,
    unsigned    flags,
    void *      pDefaults,
    void *      pIn
    )
{
    // dummy function to staisfy ldap_sasl_interactive_bind call
    return LDAP_SUCCESS;
}
