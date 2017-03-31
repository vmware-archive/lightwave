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
 * Module Name: Directory main
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * Directory main module include file
 *
 */
#ifndef _WIN32

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "sasl/sasl.h"
#include "sasl/saslutil.h"
#include "sasl/saslplug.h"

#else

#pragma once

#include "targetver.h"
typedef unsigned int (*PFSRPGETFUN)(const char*, unsigned char**, unsigned int*);

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <errno.h>
#include <assert.h>

#include "sasl/sasl.h"
#include "sasl/saslutil.h"
#include "sasl/saslplug.h"

#include <vmdirerrors.h>
#endif

