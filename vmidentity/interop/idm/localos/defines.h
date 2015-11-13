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
 * Module Name:
 *
 *        defines.h
 *
 * Abstract:
 *
 *        Identity Manager - Local O/S Identity Provider
 *
 *        Definitions
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *          Adam Bernstein (abernstein@vmware.com)
 *
 */

#ifndef IsNullOrEmptyString
#define IsNullOrEmptyString(str) (!(str) || !(*str))
#endif

#define BAIL_ON_IDM_ERROR(dwError) \
    if (dwError != 0) goto error;

#define IDM_SAFE_FREE_MEMORY(pMemory) \
    if (pMemory != NULL) IDMFreeMemory(pMemory);
