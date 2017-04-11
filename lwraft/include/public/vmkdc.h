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




#ifndef VMKDC_H_
#define VMKDC_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "vmkdctypes.h"

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

// Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096

#ifndef _WIN32
#define VMKDC_NCALRPC_END_POINT "vmkdcsvc"
#else
// note: keep in sync with /vmkdc/main/idl/vmkdc.idl
#define VMKDC_NCALRPC_END_POINT "VMwareKdcService"
#endif

#define VMKDC_RPC_TCP_END_POINT "2013"
#define VMKDC_MAX_SERVER_ID     255


// Logging stuff
#define MAX_LOG_MESSAGE_LEN    4096

// Add public APIs which are interface to RPC subsystem here.
UINT32
VmKdcServiceInitialize(
    PSTR str);

UINT32
VmKdcServiceFinalize(
    PSTR str);

#ifdef __cplusplus
}
#endif

#endif /* VMKDC_H_ */



