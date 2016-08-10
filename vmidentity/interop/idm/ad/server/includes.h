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
 *        Common header 
 *
 * Authors: Adam Bernstein (abernstein@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#ifndef _WIN32
#include <lw/base.h>
#include <lw/types.h>
#include <ldap.h>
#include <krb5.h>
#include <vmidmgr.h>
#else
#include <vmidmgr.h>
#include <krb5.h>
#endif
