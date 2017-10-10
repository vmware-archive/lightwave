/*
 * Copyright © 2012-2017 VMware, Inc.  All Rights Reserved.
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

#include <config.h>
#include <vmafdsys.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <openssl/pem.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <vmafd.h>
#include <vmafdtypes.h>
#include <vmafddefines.h>
#include <vmafderrorcode.h>
#include <vecs_error.h>
#include <vmafdcommon.h>
#include <djapi.h>
#include <lwnet.h>
#include <lwnet-utils.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "defines.h"
#include "structs.h"
#include "externs.h"
#include <vmnetevent.h>
#include "prototypes.h"
