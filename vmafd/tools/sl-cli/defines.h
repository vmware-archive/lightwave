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



#ifndef _SLCLI_DEFINES_H_
#define _SLCLI_DEFINES_H_

#ifndef _WIN32

#define SLCLI_OPTION_NETWORK_ADDRESS        'h'
#define SLCLI_LONG_OPTION_NETWORK_ADDRESS   "address"
#define SLCLI_OPTION_DOMAIN                 'd'
#define SLCLI_LONG_OPTION_DOMAIN            "domain"
#define SLCLI_OPTION_USERNAME               'u'
#define SLCLI_LONG_OPTION_USERNAME          "username"
#define SLCLI_OPTION_PASSWORD               'w'
#define SLCLI_LONG_OPTION_PASSWORD          "password"

#define SLCLI_OPTION_NODE_DATA          'N'
#define SLCLI_LONG_OPTION_NODE_DATA     "nodedata"
#define SLCLI_OPTION_ENABLE             'E'
#define SLCLI_LONG_OPTION_ENABLE        "enable"
#define SLCLI_OPTION_IS_ENABLED         'I'
#define SLCLI_LONG_OPTION_IS_ENABLED    "isenabled"
#define SLCLI_OPTION_DISABLE            'D'
#define SLCLI_LONG_OPTION_DISABLE       "disable"
#define SLCLI_OPTION_SET_SIZE           'S'
#define SLCLI_LONG_OPTION_SET_SIZE      "setsize"
#define SLCLI_OPTION_GET_SIZE           'G'
#define SLCLI_LONG_OPTION_GET_SIZE      "getsize"
#define SLCLI_OPTION_RETRIEVE           'R'
#define SLCLI_LONG_OPTION_RETRIEVE      "retrieve"
#define SLCLI_OPTION_FLUSH              'F'
#define SLCLI_LONG_OPTION_FLUSH         "flush"
#define SLCLI_OPTION_AGGREGATE          'A'
#define SLCLI_LONG_OPTION_AGGREGATE     "aggregate"

#define SLCLI_COLUMN_LOGIN_DN           'l'
#define SLCLI_LONG_COLUMN_LOGIN_DN      "logindn"
#define SLCLI_COLUMN_IP                 'i'
#define SLCLI_LONG_COLUMN_IP            "ip"
#define SLCLI_COLUMN_PORT               'p'
#define SLCLI_LONG_COLUMN_PORT          "port"
#define SLCLI_COLUMN_OPERATION          'o'
#define SLCLI_LONG_COLUMN_OPERATION     "operation"
#define SLCLI_COLUMN_STRING             's'
#define SLCLI_LONG_COLUMN_STRING        "string"
#define SLCLI_COLUMN_ERROR_CODE         'e'
#define SLCLI_LONG_COLUMN_ERROR_CODE    "errorcode"
#define SLCLI_COLUMN_TIME               't'
#define SLCLI_LONG_COLUMN_TIME          "time"

#define SLCLI_OPTIONS                   "h:d:u:w:NEIDS:GRFAliposet"

#else

#define SLCLI_OPTION_NETWORK_ADDRESS        "-h"
#define SLCLI_LONG_OPTION_NETWORK_ADDRESS   "--address"
#define SLCLI_OPTION_DOMAIN                 "-d"
#define SLCLI_LONG_OPTION_DOMAIN            "--domain"
#define SLCLI_OPTION_USERNAME               "-u"
#define SLCLI_LONG_OPTION_USERNAME          "--username"
#define SLCLI_OPTION_PASSWORD               "-w"
#define SLCLI_LONG_OPTION_PASSWORD          "--password"

#define SLCLI_OPTION_NODE_DATA          "-N"
#define SLCLI_LONG_OPTION_NODE_DATA     "--nodedata"
#define SLCLI_OPTION_ENABLE             "-E"
#define SLCLI_LONG_OPTION_ENABLE        "--enable"
#define SLCLI_OPTION_IS_ENABLED         "-I"
#define SLCLI_LONG_OPTION_IS_ENABLED    "--isenabled"
#define SLCLI_OPTION_DISABLE            "-D"
#define SLCLI_LONG_OPTION_DISABLE       "--disable"
#define SLCLI_OPTION_SET_SIZE           "-S"
#define SLCLI_LONG_OPTION_SET_SIZE      "--setsize"
#define SLCLI_OPTION_GET_SIZE           "-G"
#define SLCLI_LONG_OPTION_GET_SIZE      "--getsize"
#define SLCLI_OPTION_RETRIEVE           "-R"
#define SLCLI_LONG_OPTION_RETRIEVE      "--retrieve"
#define SLCLI_OPTION_FLUSH              "-F"
#define SLCLI_LONG_OPTION_FLUSH         "--flush"
#define SLCLI_OPTION_AGGREGATE          "-A"
#define SLCLI_LONG_OPTION_AGGREGATE     "--aggregate"

#define SLCLI_COLUMN_LOGIN_DN           "-l"
#define SLCLI_LONG_COLUMN_LOGIN_DN      "--logindn"
#define SLCLI_COLUMN_IP                 "-i"
#define SLCLI_LONG_COLUMN_IP            "--ip"
#define SLCLI_COLUMN_PORT               "-p"
#define SLCLI_LONG_COLUMN_PORT          "--port"
#define SLCLI_COLUMN_OPERATION          "-o"
#define SLCLI_LONG_COLUMN_OPERATION     "--operation"
#define SLCLI_COLUMN_STRING             "-s"
#define SLCLI_LONG_COLUMN_STRING        "--string"
#define SLCLI_COLUMN_ERROR_CODE         "-e"
#define SLCLI_LONG_COLUMN_ERROR_CODE    "--errorcode"
#define SLCLI_COLUMN_TIME               "-t"
#define SLCLI_LONG_COLUMN_TIME          "--time"

#endif

#endif /* _SLCLI_DEFINES_H_ */
