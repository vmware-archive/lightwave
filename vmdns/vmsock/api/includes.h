#ifndef _WIN32
#include <config.h>
#include <vmdnssys.h>
#else
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <Ws2tcpip.h>
#endif

#include <vmdns.h>
#include <vmdnsdefines.h>

#include <vmdnscommon.h>
#include <vmsock.h>
#include <vmsockapi.h>

#ifdef _WIN32
#include <vmwinsock.h>
#else
#include <vmsockposix.h>
#endif

#include "defines.h"
#include "externs.h"
