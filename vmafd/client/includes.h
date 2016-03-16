#ifndef _WIN32

#include <config.h>
#include <vmafdsys.h>

#include <lwrpcrt/lwrpcrt.h>
#include <dce/rpc.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafdclient.h>
#include <vmsuperlogging.h>
#include <vecsclient.h>
#include <cdcclient.h>
#include <vecslocalclient.h>
#include <vecs_error.h>


#include "defines.h"
#include "vmafd_h.h"
#include "structs.h"
#include "prototypes.h"
#include "vmafdsuperlog_h.h"

#else //_ WIN32

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include <dce/rpc.h>
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>
#include "banned.h" // windows banned APIs
#include <openssl/x509.h>
#include <vmafd.h>
#include <vmafddefines.h>

#include <vmafdcommon.h>
#include <vmafdclient.h>
#include <vmsuperlogging.h>
#include <vecsclient.h>
#include <cdcclient.h>
#include <vecslocalclient.h>
#include <vecs_error.h>

#include "defines.h"
#include "vmafd_h.h"
#include "structs.h"
#include "prototypes.h"
#include "vmafdsuperlog_h.h"

#endif // #ifndef _WIN32
