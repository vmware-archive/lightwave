#ifndef _WIN32
#include <config.h>
#include <vmeventsys.h>
#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>
#include <lwmem.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <vmevent.h>
#include <vmeventtypes.h>
#include <vmeventcommon.h>

#include "structs.h"
#include "externs.h"
#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <direct.h>

#include "banned.h"
#include <openssl/x509.h>

#include <vmevent.h>
#include <vmeventtypes.h>
#include <vmeventcommon.h>

#include "structs.h"

#endif
