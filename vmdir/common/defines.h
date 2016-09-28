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

#pragma once

#define VMDIR_IPC_PACKET_SIZE 64*1024
#define VMDIR_MAX_SCHEMACHECK_ATTR_COUNT 4

#ifndef _WIN32
//#define SOCKET_FILE_PATH "/var/lib/vmware/ipc/vmdir_socket"
#define SOCKET_FILE_PATH "/tmp/vmdir_socket"
#define EVERYONE_UID -1
#endif
#if defined _WIN32
#define NAME_OF_PIPE "\\\\.\\pipe\\vmdirnamedpipe"
#define PIPE_TIMEOUT_INTERVAL 5000
#define PIPE_CLIENT_RETRY_COUNT 3
#endif
