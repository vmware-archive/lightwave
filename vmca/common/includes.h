#ifndef _WIN32
#include <config.h>
#include <vmcasys.h>

#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>
#include <lwmem.h>
#include <stdio.h>
#include <stdlib.h>
#include <krb5.h>

#include <netdb.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>

#else

#pragma once
#include "targetver.h"
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4995)
#endif
#include <windows.h>
#include <aclapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <krb5.h>
#include <direct.h>
#include <pthread.h>
#include <io.h>
#include <time.h>

#include "banned.h"

#endif


#include <ldap.h>
#include <sasl/sasl.h>

#include <openssl/x509.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <macros.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmcacommon.h>
#include <vmca_error.h>
#include <vmcadb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

