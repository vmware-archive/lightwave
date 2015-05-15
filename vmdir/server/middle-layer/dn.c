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

// Relevant RFC for this code: RFC 4514, Lightweight Directory Access Protocol (LDAP): String Representation of
// Distinguished Names

DWORD
VmDirGetParentDN(
	VDIR_BERVALUE * dn,
	VDIR_BERVALUE * pdn
	)
{
	int i = 0;
	int retVal = LDAP_SUCCESS;

	assert( dn != NULL && pdn != NULL && dn->bvnorm_val != NULL );

	VmDirLog( LDAP_DEBUG_TRACE, "GetParentDN: Begin: DN = %s", dn->lberbv.bv_val );

	memset(pdn, 0, sizeof(VDIR_BERVALUE));

	// Set pdn original value.

	for (i = 0; i < dn->lberbv.bv_len; i++)
	{
		// Found the 1st non-escaped RDN_SEPARATOR_CHAR
		if (dn->lberbv.bv_val[i] == RDN_SEPARATOR_CHAR && dn->lberbv.bv_val[i - 1] != RDN_VALUE_ESC_CHAR)
		{
			i++; // Set starting position of the parent DN.
			break;
		}
	}

	if (i < dn->lberbv.bv_len)
	{
        pdn->lberbv.bv_len = dn->lberbv.bv_len - i;
        pdn->lberbv.bv_val = &(dn->lberbv.bv_val[i]);

        // Set/create pdn normalized value.

        for (i = 0; i < dn->bvnorm_len; i++)
        {
            if (dn->bvnorm_val[i] == RDN_SEPARATOR_CHAR && dn->bvnorm_val[i - 1] != RDN_VALUE_ESC_CHAR)
            {
                i++; // Set starting position of the parent DN.
                break;
            }
        }

        if (i < dn->bvnorm_len)
        {
            pdn->bvnorm_len = dn->bvnorm_len - i;

            retVal = VmDirAllocateMemory( pdn->bvnorm_len+1, (PVOID *)&pdn->bvnorm_val );
            BAIL_ON_VMDIR_ERROR( retVal );

            retVal = VmDirCopyMemory( pdn->bvnorm_val, pdn->bvnorm_len+1, &(dn->bvnorm_val[i]), pdn->bvnorm_len);
            BAIL_ON_VMDIR_ERROR( retVal );
        }
	}

cleanup:
	VmDirLog( LDAP_DEBUG_TRACE, "GetParentDN: End" );

	return retVal;

error:

    VmDirFreeBervalContent( pdn );

	goto cleanup;
}

