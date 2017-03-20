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
 * Module Name: vdcpromo
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * vdcpromo main module include file
 *
 */
#ifndef _WIN32

#include <config.h>

#include <vmdirsys.h>
#include <ldap.h>
#include <lber.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdircommon.h>
#include <vmdirclient.h>
#include "defines.h"
#include "structs.h"
#include "prototypes.h"

#else
#pragma once

#include "targetver.h"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <tchar.h>
#include <winsock2.h>
#include "ldap-int.h"
#include "ldap.h"
#define LDAP_UNICODE 0

#include "banned.h"
#include <vmdir.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdircommon.h>
#include <vmdirclient.h>
#include "defines.h"
#include "structs.h"
#include "prototypes.h"

#endif
