/*
* Copyright (c) VMware Inc.  All rights Reserved.
*/

/*
* Module Name: dns socket library
*
* Filename: includes.h
*
* Abstract:
*
* dns main module include file
*
*/

#pragma once
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <errno.h>
#include <assert.h>
#include <Ws2tcpip.h>

#include <vmdnstypes.h>
#include <vmdnscommon.h>
#include <vmsock.h>
#include <vmsockapi.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"
#include "prototypes.h"
