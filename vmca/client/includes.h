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
#ifndef _STDAFX_H_
#define _STDAFX_H_

#ifndef _WIN32 /* Linux Section */

#include <lw/types.h>
#include <lw/base.h>
#include <lwstr.h>
#include <wc16str.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>


#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <vmcatypes.h>
#include <vmca.h>
#include <vmca_h.h>
#include <vmcacommon.h>
#include <vmcaclient.h>



#else /* WIN32 section */
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <wincrypt.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <gssapi/gssapi.h>
#include <gssapi/gssapi_ext.h>

#define snprintf _snprintf

#if defined(_DEBUG)
#include "../x64/Debug/vmca_h.h"
#else
#include "../x64/Release/vmca_h.h"
#endif


#endif /* ifndef _WIN32 */

#include <ldap.h>

#include <dce/rpc.h>
#include <dce/dcethread.h>
#include <vmcasys.h>
#include <vmcatypes.h>
#include <vmca.h>

#include <vmcacommon.h>
#include <vmcaclient.h>
#include <vmca_error.h>
#include "defines.h"
#include "structs.h"
#include "externs.h"
#include "binding.h"
#include "prototypes.h"
#include "macros.h"
#include "errormap.h"

#endif // _STDAFX_H_
