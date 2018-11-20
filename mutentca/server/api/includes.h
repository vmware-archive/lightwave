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

#ifndef _LWCA_SERVICE_API_INCLUDES_H_
#define _LWCA_SERVICE_API_INCLUDES_H_

#include <config.h>
#include <mutentcasys.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <mutentca.h>
#include <mutentcadb.h>
#include <mutentcaauthz.h>
#include <mutentcacommon.h>
#include <mutentcaerror.h>
#include <mutentcapkcs.h>
#include <mutentcasrvcommon.h>
#include <mutentcasrvauthz.h>
#include <mutentcadbapi.h>
#include <mutentcaapi.h>
#include <mutentcapkcs.h>
#include <mutentcapolicy.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"


#endif /* _LWCA_SERVICE_API_INCLUDES_H_ */
