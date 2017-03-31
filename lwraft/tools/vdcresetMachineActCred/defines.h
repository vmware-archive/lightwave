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

#ifndef _WIN32

#define VMDIR_OPTION_USER_NAME                 'u'
#define VMDIR_OPTION_TARGET_HOSTNAME           'H'
#define VMDIR_OPTION_SOURCE_PASSWORD           'w'
#define VMDIR_OPTIONS_VALID                    "u:H:w:"

#else

#define VMDIR_OPTION_USER_NAME                 "-u"
#define VMDIR_OPTION_TARGET_HOSTNAME           "-H"
#define VMDIR_OPTION_SOURCE_PASSWORD           "-w"

#endif

#define VMDIR_MAX_PWD_LEN     128
