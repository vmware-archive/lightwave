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
 * Module Name: ThinAppRepoClient
 *
 * Filename: prototypes.h
 *
 * Abstract:
 *
 * Private function prototypes
 *
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* binding.c */

DWORD
EventLogCreateBindingHandleA(
	RP_PCSTR pszNetworkAddress,
	RP_PCSTR pszNetworkEndpoint,
	handle_t * ppBinding
	);

DWORD
EventLogFreeBindingHandle(
	handle_t * pBindingHandle
	);

#ifdef __cplusplus
}
#endif
