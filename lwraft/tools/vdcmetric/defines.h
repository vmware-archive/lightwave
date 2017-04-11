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



#ifndef _VDCMETRIC_DEFINES_H_
#define _VDCMETRIC_DEFINES_H_

#ifndef _WIN32

#define VDCMETRIC_OPTION_NETWORK_ADDRESS        'h'
#define VDCMETRIC_LONG_OPTION_NETWORK_ADDRESS   "address"
#define VDCMETRIC_OPTION_DOMAIN                 'd'
#define VDCMETRIC_LONG_OPTION_DOMAIN            "domain"
#define VDCMETRIC_OPTION_USERNAME               'u'
#define VDCMETRIC_LONG_OPTION_USERNAME          "username"
#define VDCMETRIC_OPTION_PASSWORD               'w'
#define VDCMETRIC_LONG_OPTION_PASSWORD          "password"

#define VDCMETRIC_OPTION_NODE_DATA          'N'
#define VDCMETRIC_LONG_OPTION_NODE_DATA     "nodedata"
#define VDCMETRIC_OPTION_ENABLE             'E'
#define VDCMETRIC_LONG_OPTION_ENABLE        "enable"
#define VDCMETRIC_OPTION_IS_ENABLED         'I'
#define VDCMETRIC_LONG_OPTION_IS_ENABLED    "isenabled"
#define VDCMETRIC_OPTION_DISABLE            'D'
#define VDCMETRIC_LONG_OPTION_DISABLE       "disable"
#define VDCMETRIC_OPTION_SET_SIZE           'S'
#define VDCMETRIC_LONG_OPTION_SET_SIZE      "setsize"
#define VDCMETRIC_OPTION_GET_SIZE           'G'
#define VDCMETRIC_LONG_OPTION_GET_SIZE      "getsize"
#define VDCMETRIC_OPTION_RETRIEVE           'R'
#define VDCMETRIC_LONG_OPTION_RETRIEVE      "retrieve"
#define VDCMETRIC_OPTION_FLUSH              'F'
#define VDCMETRIC_LONG_OPTION_FLUSH         "flush"
#define VDCMETRIC_OPTION_AGGREGATE          'A'
#define VDCMETRIC_LONG_OPTION_AGGREGATE     "aggregate"

#define VDCMETRIC_COLUMN_LOGIN_DN           'l'
#define VDCMETRIC_LONG_COLUMN_LOGIN_DN      "logindn"
#define VDCMETRIC_COLUMN_IP                 'i'
#define VDCMETRIC_LONG_COLUMN_IP            "ip"
#define VDCMETRIC_COLUMN_PORT               'p'
#define VDCMETRIC_LONG_COLUMN_PORT          "port"
#define VDCMETRIC_COLUMN_OPERATION          'o'
#define VDCMETRIC_LONG_COLUMN_OPERATION     "operation"
#define VDCMETRIC_COLUMN_STRING             's'
#define VDCMETRIC_LONG_COLUMN_STRING        "string"
#define VDCMETRIC_COLUMN_ERROR_CODE         'e'
#define VDCMETRIC_LONG_COLUMN_ERROR_CODE    "errorcode"
#define VDCMETRIC_COLUMN_TIME               't'
#define VDCMETRIC_LONG_COLUMN_TIME          "time"

#define VDCMETRIC_OPTIONS                   "h:d:u:w:NEIDS:GRFAliposet"

#else

#define VDCMETRIC_OPTION_NETWORK_ADDRESS        "-h"
#define VDCMETRIC_LONG_OPTION_NETWORK_ADDRESS   "--address"
#define VDCMETRIC_OPTION_DOMAIN                 "-d"
#define VDCMETRIC_LONG_OPTION_DOMAIN            "--domain"
#define VDCMETRIC_OPTION_USERNAME               "-u"
#define VDCMETRIC_LONG_OPTION_USERNAME          "--username"
#define VDCMETRIC_OPTION_PASSWORD               "-w"
#define VDCMETRIC_LONG_OPTION_PASSWORD          "--password"

#define VDCMETRIC_OPTION_NODE_DATA          "-N"
#define VDCMETRIC_LONG_OPTION_NODE_DATA     "--nodedata"
#define VDCMETRIC_OPTION_ENABLE             "-E"
#define VDCMETRIC_LONG_OPTION_ENABLE        "--enable"
#define VDCMETRIC_OPTION_IS_ENABLED         "-I"
#define VDCMETRIC_LONG_OPTION_IS_ENABLED    "--isenabled"
#define VDCMETRIC_OPTION_DISABLE            "-D"
#define VDCMETRIC_LONG_OPTION_DISABLE       "--disable"
#define VDCMETRIC_OPTION_SET_SIZE           "-S"
#define VDCMETRIC_LONG_OPTION_SET_SIZE      "--setsize"
#define VDCMETRIC_OPTION_GET_SIZE           "-G"
#define VDCMETRIC_LONG_OPTION_GET_SIZE      "--getsize"
#define VDCMETRIC_OPTION_RETRIEVE           "-R"
#define VDCMETRIC_LONG_OPTION_RETRIEVE      "--retrieve"
#define VDCMETRIC_OPTION_FLUSH              "-F"
#define VDCMETRIC_LONG_OPTION_FLUSH         "--flush"
#define VDCMETRIC_OPTION_AGGREGATE          "-A"
#define VDCMETRIC_LONG_OPTION_AGGREGATE     "--aggregate"

#define VDCMETRIC_COLUMN_LOGIN_DN           "-l"
#define VDCMETRIC_LONG_COLUMN_LOGIN_DN      "--logindn"
#define VDCMETRIC_COLUMN_IP                 "-i"
#define VDCMETRIC_LONG_COLUMN_IP            "--ip"
#define VDCMETRIC_COLUMN_PORT               "-p"
#define VDCMETRIC_LONG_COLUMN_PORT          "--port"
#define VDCMETRIC_COLUMN_OPERATION          "-o"
#define VDCMETRIC_LONG_COLUMN_OPERATION     "--operation"
#define VDCMETRIC_COLUMN_STRING             "-s"
#define VDCMETRIC_LONG_COLUMN_STRING        "--string"
#define VDCMETRIC_COLUMN_ERROR_CODE         "-e"
#define VDCMETRIC_LONG_COLUMN_ERROR_CODE    "--errorcode"
#define VDCMETRIC_COLUMN_TIME               "-t"
#define VDCMETRIC_LONG_COLUMN_TIME          "--time"

#endif

#endif /* _VDCMETRIC_DEFINES_H_ */
