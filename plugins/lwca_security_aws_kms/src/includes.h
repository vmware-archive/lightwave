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

#ifndef _LWCA_SECURITY_AWS_KMS_INCLUDES_H_
#define _LWCA_SECURITY_AWS_KMS_INCLUDES_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <config.h>

#include <openssl/x509.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include <lwca_security.h>
#include <vmmemory.h>
#include <vmjsonresult.h>
#include <vmhttpclient.h>

#include "security_aws_kms_errors.h"
#include "defines.h"
#include "structs.h"
#include "prototypes.h"

#endif /* _LWCA_SECURITY_AWS_KMS_INCLUDES_H_ */
