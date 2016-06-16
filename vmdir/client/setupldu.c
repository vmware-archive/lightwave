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

/*
 * Public API
 */
//returns LDAP error codes
DWORD
VmDirSetupLdu(
    PCSTR pszHostURI,
    PCSTR pszDomain,
    PCSTR pszUser,
    PCSTR pszPassword)
{
    DWORD       dwError = 0;

    LDAP*       pLd = NULL;
    PSTR        pszDN = NULL;
    PSTR        pszSiteGuid = NULL;
    PSTR        pszLduGuid = NULL;

    if (IsNullOrEmptyString(pszHostURI) ||
        IsNullOrEmptyString(pszDomain) ||
        IsNullOrEmptyString(pszUser) ||
        IsNullOrEmptyString(pszPassword))
    {
        dwError =  LDAP_INVALID_SYNTAX;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    if (VmDirGenerateGUID(&pszLduGuid))
    {
        dwError = LDAP_OPERATIONS_ERROR;
        VmDirLog( LDAP_DEBUG_ANY, "VmDirSetupLdu: VmDirGenerateGUID() failed.");
        BAIL_ON_VMDIR_ERROR( dwError );
    }

    dwError = VmDirConnectLDAPServerByURI(
                            &pLd,
                            pszHostURI,
                            pszDomain,
                            pszUser,
                            pszPassword);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirGetSiteGuidInternal(pLd, pszDomain, &pszSiteGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    dwError = VmDirCreateCMSubtree(
        pLd,
        pszDomain,
        pszSiteGuid,
        pszLduGuid);
    BAIL_ON_VMDIR_ERROR(dwError);

    //Write GUIDs into registry
    dwError = VmDirConfigSetDefaultSiteandLduGuid(
                                                pszSiteGuid,
                                                pszLduGuid);
    if (dwError)
    {
        dwError = LDAP_OPERATIONS_ERROR;
    }
    BAIL_ON_VMDIR_ERROR(dwError);

cleanup:

    VMDIR_SAFE_FREE_MEMORY(pszDN);
    VMDIR_SAFE_FREE_MEMORY(pszSiteGuid);
    VMDIR_SAFE_FREE_MEMORY(pszLduGuid);

    if (pLd)
    {
        ldap_unbind_ext_s(pLd, NULL, NULL);
    }

    return dwError;

error:

    VmDirLog(LDAP_DEBUG_TRACE, "VmDirSetupLdu failed with error (%u)\n", dwError);

    goto cleanup;
}

