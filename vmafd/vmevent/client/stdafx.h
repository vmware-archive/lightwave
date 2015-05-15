#ifndef _STDAFX_H_
#define _STDAFX_H_

#ifndef _WIN32

#include <vmeventsys.h>

#include <locale.h>
#include <lwadvapi.h>
#include <lw/types.h>
#include <lw/base.h>
#include <lwio/io-types.h>
#include <compat/dcerpc.h>
#include <dce/rpc.h>
#include <vmeventcommon.h>
#include <vmevent_h.h>
#include <netdb.h>

#include "prototypes.h"
#include "defines.h"

#else // _WIN32


#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include "banned.h" // windows banned APIs
#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <locale.h>

#include <vmeventsys.h>
#include <vmevent.h>
#include <vmeventcommon.h>
#include <vmeventclient.h>
#include <vmevent_h.h>

#include "defines.h"
#include "prototypes.h"

#endif // #ifndef _WIN32

#endif // _STDAFX_H_
