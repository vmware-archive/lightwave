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
 * Module Name: afd main
 *
 * Filename: globals.c
 *
 * Abstract:
 *
 * Globals
 *
 */

#include "includes.h"

FILE * gVmafdLogFile;

VM_AFD_VTABLE gIPCVtable = {
    VmAfd_SF_INIT(.pfnOpenServerConnection, &VmAfdOpenServerConnectionImpl),
    VmAfd_SF_INIT(.pfnCloseServerConnection, &VmAfdCloseServerConnectionImpl),
    VmAfd_SF_INIT(.pfnOpenClientConnection, &VmAfdOpenClientConnectionImpl),
    VmAfd_SF_INIT(.pfnCloseClientConnection, &VmAfdCloseClientConnectionImpl),
    VmAfd_SF_INIT(.pfnAcceptConnection, &VmAfdAcceptConnectionImpl),
    VmAfd_SF_INIT(.pfnFreeConnection, &VmAfdFreeConnectionImpl),
    VmAfd_SF_INIT(.pfnReadData, &VmAfdReadDataImpl),
    VmAfd_SF_INIT(.pfnWriteData, &VmAfdWriteDataImpl),
    VmAfd_SF_INIT(.pfnInitializeSecurityContext, &VmAfdInitializeSecurityContextImpl),
    VmAfd_SF_INIT(.pfnFreeSecurityContext, &VmAfdFreeSecurityContextImpl),
    VmAfd_SF_INIT(.pfnGetSecurityContextSize, &VmAfdGetSecurityContextSizeImpl),
    VmAfd_SF_INIT(.pfnEncodeSecurityContext, &VmAfdEncodeSecurityContextImpl),
    VmAfd_SF_INIT(.pfnDecodeSecurityContext, &VmAfdDecodeSecurityContextImpl),
    VmAfd_SF_INIT(.pfnIsRootSecurityContext, &VmAfdIsRootSecurityContextImpl),
    VmAfd_SF_INIT(.pfnEqualsContext, &VmAfdEqualsSecurityContextImpl),
    VmAfd_SF_INIT(.pfnAllocateContextFromName, &VmAfdAllocateContextFromNameImpl),
    VmAfd_SF_INIT(.pfnCopySecurityContext, &VmAfdCopySecurityContextImpl),
    VmAfd_SF_INIT(.pfnCreateAnonymousConnectionContext, &VmAfdCreateAnonymousConnectionContextImpl),
    VmAfd_SF_INIT(.pfnCreateWellKnownContext, &VmAfdCreateWellKnownContextImpl),
    VmAfd_SF_INIT(.pfnContextBelongsToGroup, &VmAfdContextBelongsToGroupImpl),
    VmAfd_SF_INIT(.pfnCheckAclContext, &VmAfdCheckAclContextImpl)
};

