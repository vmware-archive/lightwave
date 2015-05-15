#ifndef _WIN32

#include <config.h>
#include <vmauthsvcsys.h>

#include <lwrpcrt/lwrpcrt.h>
#include <dce/rpc.h>

#include <vmauthsvc.h>
#include <vmauthsvcdefines.h>

#include <vmauthsvccommon.h>

#include "defines.h"
#include "vmauthsvc_h.h"
#include "prototypes.h"

#else //_ WIN32

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <dce/rpc.h>
#include <pthread.h>
#include "banned.h" // windows banned APIs

#include <openssl/x509.h>

#include <vmauthsvc.h>
#include <vmauthsvcdefines.h>

#include <vmauthsvccommon.h>

#include "defines.h"
#include "vmauthsvc_h.h"
#include "prototypes.h"

#endif // #ifndef _WIN32
