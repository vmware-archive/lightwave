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
 *        Identity Manager - Local O/S Identity Provider
 *
 *        Common header 
 *
 * Authors: Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include <config.h>
#include <vmstssys.h>

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_LW_BASE_H
#include <lw/base.h>
#endif

#include <vmdirauth.h>

#include <idmcommon.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"

