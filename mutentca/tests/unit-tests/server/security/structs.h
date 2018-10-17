/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef _TEST_LWCA_SERVICE_SECURITY_STRUCTS_H_
#define _TEST_LWCA_SERVICE_SECURITY_STRUCTS_H_

typedef struct _LWCA_SECURITY_TEST_STATE_
{
    VOID *module;
    PLWCA_SECURITY_INTERFACE pInterface;
    PLWCA_SECURITY_HANDLE pHandle;
    PCSTR pszVersion;
}LWCA_SECURITY_TEST_STATE, *PLWCA_SECURITY_TEST_STATE;

#endif /* _TEST_LWCA_SERVICE_SECURITY_STRUCTS_H_ */
