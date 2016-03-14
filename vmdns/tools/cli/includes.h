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

#ifndef _WIN32
#include <lwrpcrt/lwrpcrt.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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

#include <pthread.h>
#include <signal.h>
