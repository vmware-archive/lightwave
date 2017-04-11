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

#define VMDIR_OPTION_UPN                'D'
#define VMDIR_OPTION_SECRET             'W'
#define VMDIR_OPTION_SECRET_FILE        'x'
#define VMDIR_OPTIONS_VALID             "D:W:x:"

#else

#define VMDIR_OPTION_UPN                "-D"
#define VMDIR_OPTION_SECRET             "-W"
#define VMDIR_OPTION_SECRET_FILE        "-x"

#endif

