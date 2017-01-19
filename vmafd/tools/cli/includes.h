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
#include <reg/lwreg.h>

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <tchar.h>
#include <winsock2.h>

#include "banned.h"
#include <pthread.h>
#include <openssl/x509.h>
#endif

#include <vmafdsys.h>
#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafdclient.h>
#include <vmdirclient.h>
#include <vmafcfg.h>

#include <vmafderrorcode.h>
#include <vmdirerrors.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
