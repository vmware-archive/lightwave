#pragma once

#define VMDIR_IPC_PACKET_SIZE 64*1024

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
