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



/*
 * Module Name: afd main
 *
 * Filename: includes.h
 *
 * Abstract:
 *
 * afd main module include file
 *
 */

#ifndef _WIN32/* ============= LINUX ONLY ================ */
#include <config.h>
#include <vmafdsys.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <fcntl.h>
#include <lwrpcrt/lwrpcrt.h>
#include <dce/rpc.h>
#include <reg/lwreg.h>
#include <reg/regutil.h>
#include <djapi.h>
#include <lwnet.h>
#include <lwnet-utils.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <pthread.h>

#if defined(NOTIFY_VMDIR_PROVIDER)
#include <lsa/vmdir.h>
#include <lsa/vmdir-types.h>
#endif

#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <vmsuperlogging.h>
#include <vmafcfg.h>
#include <vecsdb.h>
#include <cdcdb.h>
#include <securitystructs.h>
#include <vecsauth.h>
#include <authdb.h>
#include <regdb.h>
#include <vmeventrpc.h>
#include <wchar.h>
#include <vmdns.h>
#include <vmafdclient.h>
#include <vmdirclient.h>
#include <vmdirerrors.h>
#include <vmdns.h>
#include <dirent.h>
#include <vecs_error.h>
#include <linux/limits.h>
#include <time.h>
#ifdef USE_DEFAULT_KRB5_PATHS
#include <profile.h>
#endif


#else
/* ========================= WIN32 ONLY ======================== */

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <dsrole.h>
#include <Lm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <tchar.h>
#include <errno.h>
#include <winsvc.h>
#include <dce/rpc.h>
#include <assert.h>
#include <Ws2tcpip.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <openssl/x509.h>

#include <winsock.h>
#include <vmdns.h>
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vmafdcommon.h>
#include <vmsuperlogging.h>
#include <vmafcfg.h>
#include <vmeventrpc.h>
#include <securitystructs.h>
#include <vecsdb.h>
#include <cdcdb.h>
#include <vecsauth.h>
#include <authdb.h>
#include <regdb.h>
#include <vmafdclient.h>
#include <vmdirclient.h>
#include <vmdirerrors.h>
#include <vmdns.h>
#include <vecs_error.h>

#endif

#include <ldap.h>

#include "defines.h"
#include "structs.h"
#include "authprototypes.h"
#include "prototypes.h"
#include "vmafd60_h.h"
#include "vmafd_h.h"
#include "vmafdsuperlog_h.h"
#include "externs.h"
