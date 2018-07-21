/*
 * Copyright © 2017 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the ?~@~\License?~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ?~@~\AS IS?~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#include <lw/types.h>
#include <lw/hash.h>

#include <curl/curl.h>
#include <jansson.h>

#include "defines.h"
#include "errorcode.h"
#include "memory.h"
#include "vmstring.h"
#include "structs.h"
#include "vmhttpclient.h"
#include "vmjsonresult.h"
#include "vmmetrics.h"
