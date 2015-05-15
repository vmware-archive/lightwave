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
 * Module Name: ThinAppRepoService
 *
 * Filename: reposys.h
 *
 * Abstract:
 *
 * System Headers
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS 1
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_LW_BASE_H
#include <lw/base.h>
#endif

#ifdef HAVE_OPENSSL_CRYPTO_H
#include <openssl/crypto.h>
#endif

#ifdef HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#endif

#ifdef HAVE_OPENSSL_ERR_H
#include <openssl/err.h>
#endif

#ifdef HAVE_OPENSSL_X509_H
#include <openssl/x509.h>
#endif

#ifdef HAVE_UUID_UUID_H
#include <uuid/uuid.h>
#endif

#ifdef HAVE_LWRPCRT_LWRPCRT_H
#include <lwrpcrt/lwrpcrt.h>
#endif

#ifndef _WIN32

#include <inttypes.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/param.h>
#include <limits.h>
#include <sys/stat.h>
#include <dirent.h>

#endif

#ifdef __cplusplus
}
#endif
