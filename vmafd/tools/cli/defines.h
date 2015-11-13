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



#define ERROR_LOCAL_BASE                            (100000)
#define ERROR_LOCAL_OPTION_UNKNOWN                  (ERROR_LOCAL_BASE + 1) // This option does not exist
#define ERROR_LOCAL_OPTION_INVALID                  (ERROR_LOCAL_BASE + 2) // The options are not semantically valid
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN        (ERROR_LOCAL_BASE + 3)
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_READ        (ERROR_LOCAL_BASE + 4)
#define ERROR_LOCAL_PASSWORD_EMPTY                  (ERROR_LOCAL_BASE + 5)

