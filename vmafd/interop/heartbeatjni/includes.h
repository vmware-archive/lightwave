#ifndef _WIN32

#include <config.h>

#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>
#include <stdint.h>
#include <tchar.h>

#include "banned.h"
#include <pthread.h>
#include <openssl/x509.h>
#endif

#include <vmafdsys.h>
#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafdclient.h>
#include "heartbeatjni.h"
