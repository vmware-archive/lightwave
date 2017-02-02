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

/* PerformDelete: Parse the delete request on the wire, and call middle-layer Delete functionality.
 *
 * From RFC 4511:
 *
 *   DelRequest ::= [APPLICATION 10] LDAPDN
 *
 */

int
VmDirPerformDelete(
   PVDIR_OPERATION pOperation
   )
{
    int         retVal = LDAP_SUCCESS;
    DeleteReq * dr = &(pOperation->request.deleteReq);

    // Get entry DN. 'm' => pOperation->reqDn.lberbv.bv_val points to DN within (in-place) ber
    if ( ber_scanf( pOperation->ber, "m", &(pOperation->reqDn.lberbv) ) == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "PerformDelete: ber_scanf failed" );
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR( retVal );
    }

    VMDIR_LOG_INFO( LDAP_DEBUG_ARGS, "Delete Request: dn (%s)", pOperation->reqDn.lberbv.bv_val );

    memset( dr, 0, sizeof( DeleteReq ));

    // NOTE: pOperation->reqDn.lberbv.bv_val is NULL terminated (TODO, verify this)
    dr->dn.lberbv.bv_val = pOperation->reqDn.lberbv.bv_val;
    dr->dn.lberbv.bv_len = pOperation->reqDn.lberbv.bv_len;

    retVal = ParseRequestControls(pOperation, &pOperation->ldapResult);
    BAIL_ON_VMDIR_ERROR(retVal);

    retVal = VmDirMLDelete( pOperation );
    BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    if (retVal != LDAP_NOTICE_OF_DISCONNECT)
    {
        VmDirSendLdapResult( pOperation );
    }
    return retVal;

error:
    goto cleanup;
}

void
VmDirFreeDeleteRequest(
   DeleteReq * dr,
   BOOLEAN     freeSelf
   )
{
    if (dr != NULL)
    {
        VmDirFreeBervalContent( &(dr->dn) );

        if (freeSelf)
        {
            VMDIR_SAFE_FREE_MEMORY( dr );
        }
    }

    return;
}
