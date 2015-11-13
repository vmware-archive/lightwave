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

#define VMAFD_OPTION_DOMAIN          'd'
#define VMAFD_OPTION_USER_NAME       'u'
#define VMAFD_OPTION_PASSWORD        'w'
#define VMAFD_OPTION_SITE_NAME       's'
#define VMAFD_OPTION_REPL_HOST_NAME  'H'
#define VMDIR_OPTION_LOTUS_SERVER_NAME  'h'
#define VMAFD_OPTION_INIT_DNS        'n'
#define VMAFD_OPTION_FIRST_REPL_MODE 'R'
#define VMAFD_OPTION_PWD_FILE        'x'
#define VMAFD_OPTIONS_VALID "td:w:u:H:h:nR:s:x:"

#else

#define VMAFD_OPTION_DOMAIN          "-d"
#define VMAFD_OPTION_USER_NAME       "-u"
#define VMAFD_OPTION_PASSWORD        "-w"
#define VMAFD_OPTION_SITE_NAME       "-s"
#define VMAFD_OPTION_TENANT          "-t"
#define VMAFD_OPTION_REPL_HOST_NAME  "-H"
#define VMDIR_OPTION_LOTUS_SERVER_NAME  "-h"
#define VMAFD_OPTION_INIT_DNS        "-n"
#define VMAFD_OPTION_FIRST_REPL_MODE "-R"
#define VMAFD_OPTION_PWD_FILE        "-x"

#endif

#define MAX_PATH 260
#define VMAFD_MAX_PWD_LEN     128

#define ERROR_LOCAL_BASE                            (100000)
#define ERROR_LOCAL_OPTION_UNKNOWN                  (ERROR_LOCAL_BASE + 1) // This option does not exist
#define ERROR_LOCAL_OPTION_INVALID                  (ERROR_LOCAL_BASE + 2) // The options are not semantically valid
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_OPEN        (ERROR_LOCAL_BASE + 3)
#define ERROR_LOCAL_PASSWORDFILE_CANNOT_READ        (ERROR_LOCAL_BASE + 4)
#define ERROR_LOCAL_PASSWORD_EMPTY                  (ERROR_LOCAL_BASE + 5)
#define ERROR_LOCAL_OPTION_VALUE_INVALID            (ERROR_LOCAL_BASE + 6)

#ifndef _WIN32
#define VMAFD_LOG_PATH "/var/log/vmware/vmafd/"
#else
#define VMAFD_SOFTWARE_KEY_PATH "SOFTWARE\\VMware, Inc.\\VMware Afd Services"
#define VMAFD_LOGPATH_KEY_VALUE "LogsPath"

extern FILE gVmAfdLogFile;

#endif

typedef enum _DNS_INIT_FLAG
{
    DNS_NONE,
    DNS_INIT,
    DNS_UNINIT
} DNS_INIT_FLAG;

