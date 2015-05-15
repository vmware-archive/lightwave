#ifndef _WIN32
#include <config.h>
#include <vmkdcsys.h>

#else

#pragma once
#include "targetver.h"
#include <time.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <assert.h>
#include <stddef.h>

#endif

#include <vmkdc.h>
#include <vmkdctypes.h>
#include <vmkdcdefines.h>
#include <vmkdcerrorcode.h>

#include <dce/rpc.h>
#include <pthread.h>
#include <vmkdccommon.h>
#include <kdcsrvcommon.h>
#include <vmkdcserver.h>
