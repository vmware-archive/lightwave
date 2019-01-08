/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#ifndef _MUTENTCA_RESTHEAD_INCLUDES_H_
#define _MUTENTCA_RESTHEAD_INCLUDES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>
#include <lw/hash.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <mutentca.h>
#include <mutentcadb.h>
#include <mutentcaauthz.h>
#include <mutentcaerror.h>
#include <mutentcacommon.h>
#include <mutentcapkcs.h>
#include <mutentcasrvcommon.h>
#include <mutentcaapi.h>
#include <mutentcaoidc.h>
#include <mutentcaresthead.h>

#include <vmutil.h>

#ifdef REST_ENABLED

#include <copenapi/copenapi.h>
#include <jansson.h>
#include <vmrest.h>

#include "defines.h"
#include "structs.h"
#include "apidefs.h"
#include "prototypes.h"
#include "externs.h"

#endif

#ifdef __cplusplus
}
#endif

#endif /* _MUTENTCA_RESTHEAD_INCLUDES_H_ */
