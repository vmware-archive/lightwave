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
#include <mutentcasys.h>

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

#include <ldap.h>
#include <sasl/sasl.h>

#include <uuid/uuid.h>

#include <openssl/x509.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <jansson.h>

#include <vmhttpclient.h>
#include <vmafd.h>
#include <vmafdclient.h>
#include <vmafdtypes.h>
#include <vecsclient.h>
#include <ssotypes.h>
#include <oidc_types.h>
#include <oidc.h>

#include <mutentca.h>
#include <mutentcadb.h>
#include <mutentcacommon.h>
#include <mutentcaerror.h>
#include <mutentcapkcs.h>

#include <vmutil.h>
#include <vmnetworkutil.h>

#include "defines.h"
#include "structs.h"
#include "externs.h"
