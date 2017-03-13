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

#ifndef _SSO_TYPES_H_
#define _SSO_TYPES_H_

typedef unsigned char bool;
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

typedef int INTEGER;
typedef long long SSO_LONG;

typedef unsigned int SSOERROR;

typedef       char*  PSTRING;
typedef const char* PCSTRING;

#endif
