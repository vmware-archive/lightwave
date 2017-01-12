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



#define VMKDC_ADMIN_DEFAULT_REALM "VSPHERE.LOCAL"

#ifndef _WIN32

#define VMKDC_ADMIN_OPTION_PASSWORD 'p'
#define VMKDC_ADMIN_OPTION_KEYTAB 'k'
#define VMKDC_ADMIN_OPTION_RANDKEY 'r'
#define VMKDC_ADMIN_OPTION_HELP 'h'
#define VMKDC_ADMIN_ADDPRINC_OPTIONS_VALID "p:r"
#define VMKDC_ADMIN_KTADD_OPTIONS_VALID "k:"

#else

#define VMKDC_ADMIN_OPTION_PASSWORD "-p"
#define VMKDC_ADMIN_OPTION_KEYTAB "-k"
#define VMKDC_ADMIN_OPTION_RANDKEY "-r"
#define VMKDC_ADMIN_OPTION_HELP "-h"

#endif
