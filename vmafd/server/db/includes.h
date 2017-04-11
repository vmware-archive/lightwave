/*
 * Copyright (C) 2014 VMware, Inc. All rights reserved.
 *
 * Module   : includes.h
 *
 * Abstract :
 *
 */
#ifndef _WIN32
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <config.h>

#include <vmafdsys.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>

#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>

#include <vmafdcommon.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <vecsdb.h>
#include <cdcdb.h>
#include <regdb.h>
#include <vmafcfg.h>
#include <securitystructs.h>
#include <vecsauth.h>
#include <authdb.h>
#include <assert.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else

#pragma once

#define WIN32_LEAN_AND_MEAN 1  // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <pthread.h>
#include <assert.h>

#include <sqlite3.h>
#include <openssl/x509.h>

#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <securitystructs.h>
#include <vecsdb.h>
#include <cdcdb.h>
#include <vecsauth.h>
#include <authdb.h>
#include <vmafcfg.h>
#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#endif

