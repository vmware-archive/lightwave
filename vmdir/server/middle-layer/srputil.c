 /*
  * Copyright (c) VMware Inc.  All rights Reserved.
  */

/*
 * Module Name: Directory middle layer
 *
 * Filename: srputil.c
 *
 * Abstract:
 *
 * Adding SRP attribute.
 *
 */

#include "includes.h"

/*
 * Create SRP ATTR_SRP_SECRET_KEY
 */
DWORD
VmDirSRPSetSecret(
    PVDIR_OPERATION  pOperation,
    PVDIR_ENTRY      pEntry,
    PVDIR_BERVALUE   pBervClearTextPasswd
    )
{
    DWORD           dwError = 0;
    VDIR_BERVALUE   bervSecretBlob = VDIR_BERVALUE_INIT;
    PVDIR_ATTRIBUTE pAttrUPN = NULL;


    if ( !pOperation || !pEntry || !pBervClearTextPasswd )
    {
        dwError = ERROR_INVALID_PARAMETER;
        BAIL_ON_VMDIR_ERROR(dwError);
    }

    // lower case UPN is our I(Identifier) in SRP
    pAttrUPN = VmDirFindAttrByName(pEntry, ATTR_KRB_UPN);

    if ( VMDIR_ATTR_WITH_SINGLE_VALUE( pAttrUPN )
         &&
         pBervClearTextPasswd->lberbv_len > 0
       )
    {
        dwError = VmDirSRPCreateSecret(&(pAttrUPN->vals[0]), pBervClearTextPasswd, &bervSecretBlob);
        BAIL_ON_VMDIR_ERROR(dwError);

        switch ( pOperation->reqCode)
        {
            case LDAP_REQ_ADD:
                dwError = VmDirEntryAddSingleValueAttribute(
                                pEntry,
                                ATTR_SRP_SECRET,
                                bervSecretBlob.lberbv_val,
                                bervSecretBlob.lberbv_len);
                BAIL_ON_VMDIR_ERROR(dwError);

                break;

            case LDAP_REQ_MODIFY:

                dwError = VmDirAppendAMod(  pOperation,
                                            MOD_OP_REPLACE,
                                            ATTR_SRP_SECRET,
                                            ATTR_SRP_SECRET_LEN,
                                            bervSecretBlob.lberbv_val,
                                            bervSecretBlob.lberbv_len);
                BAIL_ON_VMDIR_ERROR(dwError);

                break;

            //TODO, need to handle userpassword attribute delete case as well.
            default:
                break;
        }
    }
    else
    {   // no clear password value, delete corresponding srp attributes
        switch ( pOperation->reqCode)
        {
            case LDAP_REQ_MODIFY:
                // add mod structure to operation->request.modifyReq
                dwError = VmDirAppendAMod(  pOperation,
                                            MOD_OP_DELETE,
                                            ATTR_SRP_SECRET,
                                            ATTR_SRP_SECRET_LEN,
                                            NULL,
                                            0);
                BAIL_ON_VMDIR_ERROR(dwError);

                break;

            default:
                break;
        }
    }

cleanup:

    VmDirFreeBervalContent(&bervSecretBlob);

    return dwError;

error:

    VMDIR_LOG_ERROR( VMDIR_LOG_MASK_ALL,
                     "VmDirSRPSetSecret failed. (%u)(%s)",
                     dwError, VDIR_SAFE_STRING(pEntry->dn.lberbv_val));

    goto cleanup;
}



