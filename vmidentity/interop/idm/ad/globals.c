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
 *        globals.c
 *
 * Abstract:
 *
 *        Identity Manager - Active Directory Integration
 *
 *        Global variables
 *
 * Authors: Krishna Ganugapati (krishnag@vmware.com)
 *          Sriram Nambakam (snambakam@vmware.com)
 *
 */

#include "includes.h"

#ifndef _WIN32

static
IDM_KRB_CONTEXT gIdmKrbContext =
	{
			.mutex_rw     = PTHREAD_RWLOCK_INITIALIZER,
			.state        = IDM_KRB_CONTEXT_STATE_UNKNOWN,
			.pszAccount   = NULL,
			.pszDomain    = NULL,
			.pszCachePath = NULL,
			.expiryTime   = 0
	};

PIDM_KRB_CONTEXT pgIdmKrbContext = &gIdmKrbContext;

IDM_AUTH_MUTEX gIdmAuthMutex = 
       {
            .mutex = PTHREAD_MUTEX_INITIALIZER
       };

PIDM_AUTH_MUTEX pgIdmAuthMutex = &gIdmAuthMutex;

#endif /* !_WIN32 */

