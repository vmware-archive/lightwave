
#ifndef _WIN32

#include <vmcasys.h>

#include <sqlite3.h>

#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <lw/types.h>
#include <lw/base.h>

#include <vmcatypes.h>
#include <vmcacommon.h>
#include <vmcadb.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

#else
#pragma once
#include "targetver.h"
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <Sddl.h>
#include <process.h>
#include <assert.h>
#include <pthread.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include "banned.h"

#include <macros.h>
#include <vmcatypes.h>
#include <vmca.h>
#include <vmcasys.h>
#include <vmcacommon.h>
#include <vmcadb.h>

#include "sqlite3.h"
#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"
#endif
