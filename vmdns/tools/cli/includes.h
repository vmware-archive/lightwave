#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#include <sys/socket.h>
#include <netdb.h>
#else //_ WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef unsigned char uuid_t[16];  // typedef dce_uuid_t uuid_t;
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <dce/rpc.h>
#include <dce/uuid.h>
#include <dce/dcethread.h>

#include <vmdns.h>
#include <vmdnscommon.h>
#include <vmdnsdefines.h>

#include "structs.h"
#include "prototypes.h"
