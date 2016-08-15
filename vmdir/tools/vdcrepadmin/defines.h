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



#ifndef _VDCREPADMIN_DEFINE_H_
#define _VDCREPADMIN_DEFINE_H_

#define VDCREPADMIN_FEATURE_SHOW_PARTNERS              "showpartners"
#define VDCREPADMIN_FEATURE_SHOW_PARTNER_STATUS        "showpartnerstatus"
#define VDCREPADMIN_FEATURE_SHOW_FEDERATION_STATUS     "showfederationstatus"
#define VDCREPADMIN_FEATURE_SHOW_SERVER_ATTRIBUTE      "showservers"
#define VDCREPADMIN_FEATURE_CREATE_AGREEMENT           "createagreement"
#define VDCREPADMIN_FEATURE_REMOVE_AGREEMENT           "removeagreement"
#define VDCREPADMIN_FEATURE_DUMMY_DOMAIN_WRITE         "dummydomainwrite"
#define VDCREPADMIN_QUERY_IS_FIRST_CYCLE_DONE          "isfirstcycledone"
#define VDCREPADMIN_FEATURE_SHOW_ATTRIBUTE_METADATA    "showattributemetadata"


#ifndef _WIN32

#define VDCREPADMIN_OPTION_SOURCE_HOSTNAME    'h'
#define VDCREPADMIN_OPTION_TARGET_HOSTNAME    'H'
#define VDCREPADMIN_OPTION_SOURCE_PORT        'p'
#define VDCREPADMIN_OPTION_TARGET_PORT        'P'
#define VDCREPADMIN_OPTION_SOURCE_USERNAME    'u'
#define VDCREPADMIN_OPTION_SOURCE_PASSWORD    'w'
#define VDCREPADMIN_OPTION_VERBOSE            'v'
#define VDCREPADMIN_OPTION_TWO_WAY_REPL       '2'
#define VDCREPADMIN_OPTION_FEATURE_SET        'f'
#define VDCREPADMIN_OPTION_ENTRY_DN           'e'
#define VDCREPADMIN_OPTION_ATTRIBUTE          'a'
#define VDCREPADMIN_OPTIONS_VALID             "2h:H:p:P:D:u:w:vf:e:a:"

#else
#define VDCREPADMIN_OPTION_SOURCE_HOSTNAME    "-h"
#define VDCREPADMIN_OPTION_TARGET_HOSTNAME    "-H"
#define VDCREPADMIN_OPTION_SOURCE_PORT        "-p"
#define VDCREPADMIN_OPTION_TARGET_PORT        "-P"
#define VDCREPADMIN_OPTION_SOURCE_USERNAME    "-u"
#define VDCREPADMIN_OPTION_SOURCE_PASSWORD    "-w"
#define VDCREPADMIN_OPTION_VERBOSE            "-v"
#define VDCREPADMIN_OPTION_TWO_WAY_REPL       "-2"
#define VDCREPADMIN_OPTION_FEATURE_SET        "-f"
#define VDCREPADMIN_OPTION_ENTRY_DN           "-e"
#define VDCREPADMIN_OPTION_ATTRIBUTE          "-a"

#endif

#endif // ifndef _VDCREPADMIN_DEFINE_H_

