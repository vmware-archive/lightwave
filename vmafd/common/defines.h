#pragma once

#ifndef _WIN32
#define VmAfd_SF_INIT( fieldName, fieldValue ) fieldName = fieldValue
#else
#define VmAfd_SF_INIT( fieldName, fieldValue ) fieldValue
#endif

#define VMAFD_IPC_PACKET_SIZE 64*1024

#ifndef _WIN32
#define SOCKET_FILE_PATH "/tmp/vmafd_socket"
#define EVERYONE_UID -1
#endif
#if defined _WIN32
#define NAME_OF_PIPE "\\\\.\\pipe\\vmafd_pipe"
#define PIPE_TIMEOUT_INTERVAL 5000
#define PIPE_CLIENT_RETRY_COUNT 3
#endif
