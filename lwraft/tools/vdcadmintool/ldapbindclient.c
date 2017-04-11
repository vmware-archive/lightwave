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
VOID
_VdcadminClientTestGSSAPIBind(
    PCSTR   pszLDAPURI
    );

static
VOID
_VdcadminClientTestSRPBind(
    PCSTR   pszLDAPURI,
    PCSTR   pszDefaultBindUPN,
    PCSTR   pszDefaultPasswd
    );

static
VOID
_VdcadminClientTestSimpleBind(
    PCSTR   pszLDAPURI,
    PCSTR   pszDefaultBindDN,
    PCSTR   pszDefaultPasswd
    );

static
VOID
_VdcadminClientTestSimpleSSLBind(
    PCSTR   pszLDAPURI,
    PCSTR   pszDefaultBindDN,
    PCSTR   pszDefaultPasswd
    );

VOID
VdcadminTestSASLClient(
    VOID
    )
{
    DWORD   dwError = 0;
    char    pszServerHost[SIZE_256] = {0};
    char    pszServerPort[SIZE_256] = {0};
    char    pszServerSSLPort[SIZE_256] = {0};
    char    pszBindDN[SIZE_256] = {0};
    char    pszBindUPN[SIZE_256] = {0};
    char    pszPassword[SIZE_256] = {0};
    PSTR    pszLDAPURI = NULL;
    PSTR    pszLDAPSURI = NULL;

    VmDirReadString(
        "Please enter LDAP server host: ",
        pszServerHost,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "Please enter LDAP server port: ",
        pszServerPort,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "Please enter LDAP server SSL port: ",
        pszServerSSLPort,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "Please enter LDAP Bind DN: ",
        pszBindDN,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "Please enter LDAP Bind UPN: ",
        pszBindUPN,
        SIZE_256,
        FALSE);

    VmDirReadString(
        "Please enter LDAP Bind password: ",
        pszPassword,
        SIZE_256,
        TRUE);

    printf("\n");

    dwError = VmDirAllocateStringAVsnprintf( &pszLDAPURI,
                                             "ldap://%s:%s",
                                             pszServerHost[0] != '\0' ? pszServerHost : "localhost",
                                             pszServerPort);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirAllocateStringAVsnprintf( &pszLDAPSURI,
                                             "ldaps://%s:%s",
                                             pszServerHost[0] != '\0' ? pszServerHost : "localhost",
                                             pszServerSSLPort);
    BAIL_ON_VMDIR_ERROR(dwError);

    _VdcadminClientTestSimpleBind( pszLDAPURI, pszBindDN, pszPassword );

    _VdcadminClientTestSimpleSSLBind( pszLDAPSURI, pszBindDN, pszPassword );

    _VdcadminClientTestSRPBind( pszLDAPURI, pszBindUPN, pszPassword );

    _VdcadminClientTestGSSAPIBind( pszLDAPURI );


cleanup:

    memset(pszPassword, 0, sizeof(pszPassword));

    VMDIR_SAFE_FREE_MEMORY(pszLDAPURI);
    VMDIR_SAFE_FREE_MEMORY(pszLDAPSURI);

    return;

error:

    printf("TestVdcadminSASLClient failed. (%d)\n", dwError);
    goto cleanup;
}

static
VOID
_VdcadminClientTestSimpleBind(
    PCSTR   pszLDAPURI,
    PCSTR   pszDefaultBindDN,
    PCSTR   pszDefaultPasswd
    )
{
    DWORD   dwError = 0;
    int	    ldap_version_3 = LDAP_VERSION3;
    LDAP *  pLD = NULL;
    BerValue    ldapBindPwd = {0};

    dwError = ldap_initialize( &pLD, pszLDAPURI );
    /* Set LDAP V3 protocol version */
    ldap_set_option( pLD, LDAP_OPT_PROTOCOL_VERSION, &ldap_version_3 );

    dwError = ldap_sasl_bind_s(    pLD,
                                   "",
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // no credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("%s (ANONYMOUS) bind succeeded.\n\n", pszLDAPURI);

cleanup:

    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
    }

    return;

error:

    printf("\n\n++++++++++++++++++++ %s SimpleBind failed. (%d)(%s)\n\n",
           pszLDAPURI, dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
VOID
_VdcadminClientTestSimpleSSLBind(
    PCSTR   pszLDAPSURI,
    PCSTR   pszDefaultBindDN,
    PCSTR   pszDefaultPasswd
    )
{
    DWORD   dwError = 0;
    int     ldap_version_3 = LDAP_VERSION3;
    int     iTLSNever = LDAP_OPT_X_TLS_NEVER;
    int     iTLSMin = LDAP_OPT_X_TLS_PROTOCOL_TLS1_0;
    LDAP *  pLD = NULL;
    BerValue    ldapBindPwd = {0};

    dwError = ldap_initialize( &pLD, pszLDAPSURI );
    /* Set LDAP V3 protocol version */
    ldap_set_option( pLD, LDAP_OPT_PROTOCOL_VERSION, &ldap_version_3 );
    ldap_set_option(NULL, LDAP_OPT_X_TLS_PROTOCOL_MIN, &iTLSMin);
    ldap_set_option(NULL, LDAP_OPT_X_TLS_REQUIRE_CERT, &iTLSNever);

    dwError = ldap_sasl_bind_s(    pLD,
                                   "",
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // no credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("%s (ANONYMOUS) bind succeeded.\n", pszLDAPSURI);

    ldapBindPwd.bv_val = (PSTR)pszDefaultPasswd;
    ldapBindPwd.bv_len = strlen( pszDefaultPasswd );
    dwError = ldap_sasl_bind_s(    pLD,
                                   pszDefaultBindDN,
                                   LDAP_SASL_SIMPLE,
                                   &ldapBindPwd,  // ldaps with credentials
                                   NULL,
                                   NULL,
                                   NULL);
    BAIL_ON_VMDIR_ERROR(dwError);
    printf("%s (%s) bind succeeded.\n\n", pszLDAPSURI, VDIR_SAFE_STRING(pszDefaultBindDN) );

cleanup:

    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
    }

    return;

error:

    printf("\n\n++++++++++++++++++++ %s SSL bind failed. (%d)(%s)\n\n",
           pszLDAPSURI, dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
VOID
_VdcadminClientTestSRPBind(
    PCSTR   pszLDAPURI,
    PCSTR   pszDefaultBindUPN,
    PCSTR   pszDefaultPasswd
    )
{
    DWORD   dwError = 0;
    LDAP *  pLD = NULL;

    dwError = VmDirSASLSRPBind( &pLD, pszLDAPURI, pszDefaultBindUPN, pszDefaultPasswd );
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("%s SRP bind succeeded.\n\n", pszLDAPURI);

cleanup:

    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
    }

    return;

error:

    printf("++++++++++++++++++++ %s SRP bind failed. (%d)(%s)\n\n",
           pszLDAPURI, dwError, ldap_err2string(dwError));

    goto cleanup;
}

static
VOID
_VdcadminClientTestGSSAPIBind(
    PCSTR   pszLDAPURI
    )
{
    DWORD   dwError = 0;
    LDAP *  pLD = NULL;

    dwError = VmDirSASLGSSAPIBind( &pLD, pszLDAPURI );
    BAIL_ON_VMDIR_ERROR(dwError);

    printf("%s GSSAPI bind succeeded.\n\n", pszLDAPURI);

cleanup:

    if (pLD)
    {
        dwError = ldap_unbind_ext_s(    pLD,
                                        NULL,
                                        NULL);
    }

    return;

error:

    printf("++++++++++++++++++++ %s GSSAPI bind failed. (%d)(%s)\n\n",
           pszLDAPURI, dwError, ldap_err2string(dwError));

    goto cleanup;
}
