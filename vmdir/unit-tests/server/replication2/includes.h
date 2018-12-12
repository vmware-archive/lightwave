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

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <setjmp.h>
#include <cmocka.h>

#include <config.h>

#include <execinfo.h>
#include <vmdirsys.h>

#include <uuid/uuid.h>

#include <sasl/sasl.h>
#include <sasl/saslutil.h>

#include <openssl/sha.h>
#include <openssl/rand.h>

#include <vmdir.h>
#include <vmdirtypes.h>
#include <vmdirdefines.h>
#include <vmdirerrors.h>
#include <vmdirerrorcode.h>

#include <vmdircommon.h>
#include <srvcommon.h>

#include <vmsuperlogging.h>
#include <vmdirserver.h>
#include <ldaphead.h>
#include <schema.h>
#include <vmacl.h>

#include <backend.h>
#include <middlelayer.h>
#include <replication.h>
#include <replication2/structs.h>
#include <replication2/prototypes.h>

#include "prototypes.h"
