/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : includes.h
 *
 * Abstract :
 *
 */
#ifndef _WIN32

#include <config.h>
#include <vmafdsys.h>

#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafcfg.h>
#include <vmafcfgapi.h>
#include <posixcfg.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"

#else //_ WIN32

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "banned.h" // windows banned APIs
#include <openssl/x509.h>

#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafcfg.h>
#include <vmafcfgapi.h>
#include <wincfg.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"

#endif // #ifndef _WIN32
