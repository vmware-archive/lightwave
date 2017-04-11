/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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
#if defined(HAVE_DCERPC_WIN32)
#  if defined(_DEBUG) /* Use stubs built by makefile.x64 in Idl directory */
#    include "../../vmdird/x64/Debug/vmdir_c.c"
#  else
#    include "../../vmdird/x64/Release/vmdir_c.c"
#  endif
#else /* !defined(HAVE_DCERPC_WIN32) */
#include "./vmdir_c.c"
#endif
