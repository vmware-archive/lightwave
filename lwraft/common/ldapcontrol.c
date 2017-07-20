/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

VOID
VmDirFreeCtrlContent(
    LDAPControl*    pCtrl
    )
{
    if (pCtrl)
    {
        if (pCtrl->ldctl_value.bv_val)
        {
            ber_memfree(pCtrl->ldctl_value.bv_val);
        }
        memset(pCtrl, 0, sizeof(LDAPControl));
    }
}

int
VmDirCreateCondWriteCtrlContent(
    PCSTR           pszFilter,
    LDAPControl*    pCondWriteCtrl
    )
{
    int             retVal = LDAP_SUCCESS;
    BerElement*     pBer = NULL;

    if (!pszFilter || !pCondWriteCtrl)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_INVALID_PARAMETER);
    }

    if ((pBer = ber_alloc()) == NULL)
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    if ( ber_printf( pBer, "{s}", pszFilter) == -1)
    {
        VMDIR_LOG_ERROR(VMDIR_LOG_MASK_ALL, "%s: ber_printf failed.", __FUNCTION__ );
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

    memset( pCondWriteCtrl, 0, sizeof( LDAPControl ));
    pCondWriteCtrl->ldctl_oid = LDAP_CONTROL_CONDITIONAL_WRITE;
    pCondWriteCtrl->ldctl_iscritical = '1';
    if (ber_flatten2(pBer, &pCondWriteCtrl->ldctl_value, 1))
    {
        BAIL_WITH_VMDIR_ERROR(retVal, VMDIR_ERROR_NO_MEMORY);
    }

cleanup:

    if (pBer)
    {
        ber_free(pBer, 1);
    }
    return retVal;

error:
    VmDirFreeCtrlContent(pCondWriteCtrl);
    goto cleanup;
}
