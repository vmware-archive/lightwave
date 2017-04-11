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



/*
 * Module Name: Directory middle layer
 *
 * Filename: saslsockbuf.c
 *
 * Abstract:
 *
 * sasl ssf support
 *
 */

#include "includes.h"

// WARNING WARNING - use ldap_pvt.h to hookup SASL security layer.
#include "ldap_pvt.h"

DWORD
VmDirSASLSockbufInstall(
    Sockbuf*                pSockbuf,
    PVDIR_SASL_BIND_INFO    pSaslBindInfo
    )
{
    DWORD   dwError = 0;

    if ( pSaslBindInfo->saslSSF > 0 )
    {
        // install sasl encode/decode sockbuf i/o
        dwError = ldap_pvt_sasl_install(    pSockbuf,
                                            pSaslBindInfo->pSaslCtx );
        BAIL_ON_VMDIR_ERROR(dwError);
    }

cleanup:

    return dwError;

error:

    VmDirLog( LDAP_DEBUG_ANY, "VmDirSASLSockbufInstall failed (%d)", dwError);
    dwError = LDAP_NOTICE_OF_DISCONNECT;
    goto cleanup;
}


VOID
VmDirSASLSockbufRemove(
    Sockbuf*    pSockbuf
    )
{
    ldap_pvt_sasl_remove(pSockbuf);

    return;
}
