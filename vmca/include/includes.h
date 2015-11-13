#ifndef _WIN32
#include <vmcasys.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>

#include <macros.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmcacommon.h>
#include <ldap.h>

#include "structs.h"
#include "externs.h"

#else

#pragma once
//#include <targetver.h>
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <winsock.h>
#include <pthread.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

//#include "banned.h"

#include <macros.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmcacommon.h>

#include "structs.h"
#include "externs.h"

#endif

