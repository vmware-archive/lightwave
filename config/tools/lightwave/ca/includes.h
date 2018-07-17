/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif

#include <curl/curl.h>
#include <time.h>
#include <jansson.h>

#include <cfgdefs.h>
#include <cfgerrors.h>
#include <cfgutils.h>

#include <vmafdclient.h>
#include <vmca.h>
#include <ssotypes.h>
#include <ssocommon.h>
#include <oidc_types.h>
#include <oidc.h>

#include "defines.h"
#include "structs.h"
#include "prototypes.h"
