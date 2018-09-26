/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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

#ifndef __LWCA_COMMON_DEFINES_H__
#define __LWCA_COMMON_DEFINES_H__

typedef enum
{
    ATTR_NOT_FOUND = 0,
    ATTR_MATCH,
    ATTR_DIFFER,
} ATTR_SEARCH_RESULT;

#define STAT stat
#define UNLINK unlink
#define RENAME rename

#define LWCA_PATH_SEPARATOR "/"
#define LWCA_INSTALL_DIR MUTENTCA_INSTALL_DIR
#define LWCA_ROOT_CERT_DIR MUTENTCA_DB_DIR
#define LWCA_LOG_DIR MUTENTCA_LOG_DIR

#ifdef LIGHTWAVE_BUILD

#define VMAFD_VECS_CLIENT_LIBRARY   "/libvmafdclient.so"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_KEY_ROOT
#define VMAFD_LIB_KEY               VMAFD_REG_KEY_PATH

#else

#define VMAFD_VECS_CLIENT_LIBRARY   "/lib64/libvmafdclient.so"
#define VMAFD_KEY_ROOT              VMAFD_CONFIG_KEY_ROOT
#define VMAFD_LIB_KEY               VMAFD_REG_KEY_PATH

#endif /* LIGHTWAVE_BUILD */

#define MAX_CN_LENGTH 64

#define LWCA_MAX_PATH_LEN 512

#endif //__LWCA_COMMON_DEFINES_H__
