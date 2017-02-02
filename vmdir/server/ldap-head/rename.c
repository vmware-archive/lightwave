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
BOOLEAN
_VmDirIsRenameSupported(
    VOID
    );

/* PerformRename: Parse the modrdn request on the wire, and call middle-layer
 * modify functionality.
 *
 * From RFC 4511, section 4.9:
 *
 *   ModifyRequest ::= [APPLICATION 6] SEQUENCE {
 *                          entry        LDAPDN,
 *                          newrdn       RelativeLDAPDN,
 *                          deleteoldrdn BOOLEAN,
 *                          newSuperior  [0] LDAPDN OPTIONAL
 *                        }
 *
 */

int
VmDirPerformRename(
   PVDIR_OPERATION pOperation
   )
{
    ModifyReq *        modReq = &(pOperation->request.modifyReq);
    int                retVal = LDAP_SUCCESS;
    PVDIR_LDAP_RESULT  pResult = &(pOperation->ldapResult);
    ber_len_t          size = 0;
    PSTR               pszLocalErrorMsg = NULL;

    if (!_VmDirIsRenameSupported())
    {
        pResult->errCode = retVal = LDAP_UNWILLING_TO_PERFORM;
        BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, pszLocalErrorMsg, "Operation is not enabled on this server or is not supported at this domain fuctional level.");
    }

    // Get entry DN. 'm' => reqDn.bv_val points to DN within (in-place) ber
    if ( ber_scanf( pOperation->ber, "{mmb", &modReq->dn, &modReq->newrdn, &modReq->bDeleteOldRdn) == LBER_ERROR )
    {
        VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "VmDirPerformRename: ber_scanf failed" );
        pResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT;
        BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg),
                                      "Decoding error while parsing the target DN");
    }

    if (ber_peek_tag(pOperation->ber, &size) == LDAP_TAG_NEWSUPERIOR)
    {
        if ( ber_scanf(pOperation->ber, "m", &modReq->newSuperior ) == LBER_ERROR ) {
        pResult->errCode = LDAP_PROTOCOL_ERROR;
        retVal = LDAP_NOTICE_OF_DISCONNECT; BAIL_ON_VMDIR_ERROR_WITH_MSG( retVal, (pszLocalErrorMsg), "Decoding error while parsing newSuperior");

        }
    }

    retVal = ParseRequestControls(pOperation, pResult);
    BAIL_ON_VMDIR_ERROR_WITH_MSG(retVal, (pszLocalErrorMsg),
                                 "Strong consistency request control parsing failed");

   if ( ber_scanf( pOperation->ber, "}") == LBER_ERROR )
   {
      VMDIR_LOG_ERROR( LDAP_DEBUG_ARGS, "PerformRename: ber_scanf failed" );
      pResult->errCode = LDAP_PROTOCOL_ERROR;
      retVal = LDAP_NOTICE_OF_DISCONNECT;
      BAIL_ON_VMDIR_ERROR_WITH_MSG(   retVal, (pszLocalErrorMsg), "Decoding error while parsing the end of message.");
   }

   retVal = pResult->errCode = VmDirMLModify( pOperation );
   BAIL_ON_VMDIR_ERROR(retVal);

cleanup:
    if (retVal != LDAP_NOTICE_OF_DISCONNECT)
    {
        VmDirSendLdapResult( pOperation );
    }
    VMDIR_SAFE_FREE_MEMORY(pszLocalErrorMsg);
    return retVal;

error:
    VMDIR_APPEND_ERROR_MSG(pResult->pszErrMsg, pszLocalErrorMsg);
    goto cleanup;
}

static
BOOLEAN
_VmDirIsRenameSupported(
    VOID
    )
{
    BOOLEAN bSupported = FALSE;
    DWORD dwRenameSupported = 0;
    DWORD dwFunctionalLevel = 0;

    (VOID)VmDirGetRegKeyValueDword(VMDIR_CONFIG_PARAMETER_V1_KEY_PATH,
                                   VMDIR_REG_KEY_ENABLE_RENAME,
                                   &dwRenameSupported,
                                   0);
    if (dwRenameSupported == 0)
    {
        // Registry is not enabling support
        goto cleanup;
    }

    (VOID)VmDirSrvGetDomainFunctionalLevel(&dwFunctionalLevel);
    if (dwFunctionalLevel < 2)
    {
        // Functional level is not known to be high enough
        goto cleanup;
    }

    bSupported = TRUE;

cleanup:
    return bSupported;
}
