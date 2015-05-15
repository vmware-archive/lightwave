#ifndef _WIN32
#include <config.h>
#include <vmafdsys.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <sys/types.h>
#include <pwd.h>
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vecs_error.h>
#include <vmafdcommon.h>
#include "structs.h"
#include "prototypes.h"
#include "defines.h"
#include "externs.h"
#else

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <wincrypt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <pthread.h>
#include <direct.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <vecs_error.h>
#include "banned.h"
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include "structs.h"
#include "prototypes.h"
#include "defines.h"
#include "externs.h"
#endif

#include <ldap.h>
#include <vmdirclient.h>
#include <vmdirerrors.h>
#include <dce/rpc.h>


