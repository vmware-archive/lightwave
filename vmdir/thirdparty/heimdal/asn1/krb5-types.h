/* krb5-types.h -- this file was generated for x86_64-unknown-linux-gnu by
                   $Id$ */

#ifndef __krb5_types_h__
#define __krb5_types_h__

#include <stdint.h>
#include <sys/types.h>

#include "roken.h"

typedef int krb5_socket_t;

#ifndef HEIMDAL_DEPRECATED
#if defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1 )))
#define HEIMDAL_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER) && (_MSC_VER>1200)
#define HEIMDAL_DEPRECATED __declspec(deprecated)
#else
#define HEIMDAL_DEPRECATED
#endif
#endif
#ifndef HEIMDAL_PRINTF_ATTRIBUTE
#if defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1 )))
#define HEIMDAL_PRINTF_ATTRIBUTE(x) __attribute__((format x))
#else
#define HEIMDAL_PRINTF_ATTRIBUTE(x)
#endif
#endif
#ifndef HEIMDAL_NORETURN_ATTRIBUTE
#if defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1 )))
#define HEIMDAL_NORETURN_ATTRIBUTE __attribute__((noreturn))
#else
#define HEIMDAL_NORETURN_ATTRIBUTE
#endif
#endif
#ifndef HEIMDAL_UNUSED_ATTRIBUTE
#if defined(__GNUC__) && ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1 )))
#define HEIMDAL_UNUSED_ATTRIBUTE __attribute__((unused))
#else
#define HEIMDAL_UNUSED_ATTRIBUTE
#endif
#endif
#endif /* __krb5_types_h__ */
