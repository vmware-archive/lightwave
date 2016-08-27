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



#include <config.h>

#include <cfgsys.h>

#ifdef HAVE_LWSM_LWSM_H
#include <lwsm/lwsm.h>
#endif

#ifdef HAVE_REG_LWREG_H
#include <reg/lwreg.h>
#endif

#ifdef HAVE_REG_REGUTIL_H
#include <reg/regutil.h>
#endif

#ifdef HAVE_LDAP_H
#include <ldap.h>
#endif

#ifdef HAVE_VMDIRCLIENT_H
#include <vmdirclient.h>
#endif

#ifdef HAVE_VMCA_H
#include <vmca.h>
#endif

#ifdef HAVE_VMDNS_H
#include <vmdns.h>
#endif

#ifdef HAVE_VMAFDCLIENT_H
#include <vmafdclient.h>
#endif

#include <cfgdefs.h>

#include <cfgutils.h>

#include "defines.h"
#include "structs.h"

#include "prototypes.h"
#include "externs.h"

