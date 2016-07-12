/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
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

/*
 * Module Name:
 *
 *        includes.h
 *
 * Abstract:
 *
 *        Identity Manager - Active Directory Integration
 *
 *        Common include header 
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#ifdef _WIN32
/* --------------- Windows implementation -------------------- */

#define SECURITY_WIN32  1
#include "targetver.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sspi.h>
#include <dsgetdc.h>
#include <lm.h>
#include <sddl.h>
#define toupper(c) _toupper(c)

#else
/* --------------- Linux implementation -------------------- */

#include <config.h>

#include <krb5/krb5.h>
#include <gssapi/gssapi_generic.h>
#include <gssapi/gssapi_ext.h>

// #include "../../gssapi-plugins/srp/gssapi_srp.h"

#include <ldap.h>

#include <lw/base.h>
#include <lw/types.h>
#include <lsa/lsa.h>
#include <lsa/ad.h>
#include <lwadvapi.h>
#include <lwmem.h>

#include <vmstssys.h>

#endif


/* Common headers used between Windows and Linux */
#include <stdlib.h>
#include <vmidmgr.h>
#include <idmcommon.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
#include "externs.h"

