/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#ifndef INCLUDES_H_
#define INCLUDES_H_

// system headers
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// third party headers

// common headers
#include "ssotypes.h"
#include "defines.h"
#include "common_types.h"
#include "common.h"
#include "ssoerrors.h"

// core client headers
#include "ssocoreclient.h"
#include "coreclient_structs.h"
#include "coreclient_prototypes.h"
#include "coreclient_debug.h"

// project public headers
#include "ssoidmclient.h"

// local headers
#include "prototypes.h"

#endif /* INCLUDES_H_ */
