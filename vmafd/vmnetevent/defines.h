/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
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

#define VMDDNS_BUFFER_SIZE        8*1024
#ifndef _WIN32
#define VmNet_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VmNet_SF_INIT( fieldName, fieldValue ) fieldValue
#endif
#define BAIL_ON_VMNETEVENT_ERROR(dwError) \
        if (dwError) \
           goto error;

#ifdef WIN32
#define inet_pton(x, y, z) InetPtonA(x, y, z)
#endif
