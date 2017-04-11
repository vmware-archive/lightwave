/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
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

#include <vmafdclient.h>
#include <vmafdtypes.h>

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

#include <vmafdclient.h>
#include <vmafdtypes.h>

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

