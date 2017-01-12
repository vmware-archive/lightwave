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

DWORD
VmDirCreateGroup(
    PCSTR pszGroupname, /* IN              */
    PSTR* ppszUPN       /*       OPTIONAL  */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pszGroupname, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGroupAddMember(
    PCSTR pszGroupUPN,  /* IN              */
    PCSTR pszMemberUPN  /* IN              */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pszGroupUPN, dwError);
    BAIL_ON_VMDIR_INVALID_POINTER(pszMemberUPN, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}

DWORD
VmDirGroupRemoveMember(
    PCSTR pszGroupUPN,  /* IN              */
    PCSTR pszMemberUPN  /* IN              */
    )
{
    DWORD dwError = 0;

    BAIL_ON_VMDIR_INVALID_POINTER(pszGroupUPN, dwError);
    BAIL_ON_VMDIR_INVALID_POINTER(pszMemberUPN, dwError);

cleanup:

    return dwError;

error:

    goto cleanup;
}
