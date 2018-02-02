/* saslauthd.h.  Generated from saslauthd.h.in by configure.  */
/* saslauthd.h.in.  Generated from configure.in by autoheader.  */


#ifndef _SASLAUTHD_H
#define _SASLAUTHD_H

#include <stdio.h>


/* Include SASLdb Support */
/* #undef AUTH_SASLDB */

/* Define if your getpwnam_r()/getspnam_r() functions take 5 arguments */
#define GETXXNAM_R_5ARG 1

/* Define to 1 if you have the `asprintf' function. */
#define HAVE_ASPRINTF 1

/* Define to 1 if you have the <crypt.h> header file. */
#define HAVE_CRYPT_H 1

/* Define to 1 if you have the `dns_lookup' function. */
/* #undef HAVE_DNS_LOOKUP */

/* Define to 1 if you have the `dn_expand' function. */
/* #undef HAVE_DN_EXPAND */

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Does the compiler understand __func__ */
#define HAVE_FUNC /**/

/* Does compiler understand __FUNCTION__ */
/* #undef HAVE_FUNCTION */

/* Do we have a getaddrinfo? */
#define HAVE_GETADDRINFO /**/

/* Define to 1 if you have the `gethostname' function. */
#define HAVE_GETHOSTNAME 1

/* Do we have a getnameinfo() function? */
#define HAVE_GETNAMEINFO /**/

/* Define to 1 if you have the `getspnam' function. */
#define HAVE_GETSPNAM 1

/* Define to 1 if you have the `getuserpw' function. */
/* #undef HAVE_GETUSERPW */

/* Include GSSAPI/Kerberos 5 Support */
#define HAVE_GSSAPI /**/

/* Define to 1 if you have the <gssapi/gssapi_ext.h> header file. */
#define HAVE_GSSAPI_GSSAPI_EXT_H 1

/* Define if you have the gssapi.h header file */
#define HAVE_GSSAPI_H /**/

/* Define to 1 if you have the `gsskrb5_register_acceptor_identity' function.
   */
/* #undef HAVE_GSSKRB5_REGISTER_ACCEPTOR_IDENTITY */

/* Define if your GSSAPI implementation defines GSS_C_NT_HOSTBASED_SERVICE */
#define HAVE_GSS_C_NT_HOSTBASED_SERVICE /**/

/* Define if your GSSAPI implementation defines GSS_C_NT_USER_NAME */
#define HAVE_GSS_C_NT_USER_NAME /**/

/* Define to 1 if you have the `gss_decapsulate_token' function. */
#define HAVE_GSS_DECAPSULATE_TOKEN 1

/* Define to 1 if you have the `gss_encapsulate_token' function. */
#define HAVE_GSS_ENCAPSULATE_TOKEN 1

/* Define to 1 if you have the `gss_get_name_attribute' function. */
#define HAVE_GSS_GET_NAME_ATTRIBUTE 1

/* Define to 1 if you have the `gss_oid_equal' function. */
#define HAVE_GSS_OID_EQUAL 1

/* Define if your GSSAPI implementation supports SPNEGO */
#define HAVE_GSS_SPNEGO /**/

/* Include HTTP form Support */
/* #undef HAVE_HTTPFORM */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Do we have Kerberos 4 Support? */
/* #undef HAVE_KRB */

/* Define to 1 if you have the <krb5.h> header file. */
#define HAVE_KRB5_H 1

/* Define to 1 if you have the `krb_get_err_text' function. */
/* #undef HAVE_KRB_GET_ERR_TEXT */

/* Support for LDAP? */
/* #undef HAVE_LDAP */

/* Define to 1 if you have the `resolv' library (-lresolv). */
#define HAVE_LIBRESOLV 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Do we have OpenSSL? */
#define HAVE_OPENSSL /**/

/* Support for PAM? */
#define HAVE_PAM /**/

/* Does compiler understand __PRETTY_FUNCTION__ */
/* #undef HAVE_PRETTY_FUNCTION */

/* Include support for saslauthd? */
#define HAVE_SASLAUTHD /**/

/* Include SIA Support */
/* #undef HAVE_SIA */

/* Does sockaddr have an sa_len? */
/* #undef HAVE_SOCKADDR_SA_LEN */

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Do we have a socklen_t? */
#define HAVE_SOCKLEN_T /**/

/* Is there an ss_family in sockaddr_storage? */
#define HAVE_SS_FAMILY /**/

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcat' function. */
/* #undef HAVE_STRLCAT */

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Do we have a sockaddr_storage struct? */
#define HAVE_STRUCT_SOCKADDR_STORAGE /**/

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/uio.h> header file. */
#define HAVE_SYS_UIO_H 1

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* define if your compiler has __attribute__ */
#define HAVE___ATTRIBUTE__ 1

/* Using Heimdal */
/* #undef KRB5_HEIMDAL */

/* Name of package */
#define PACKAGE "saslauthd"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Location of saslauthd socket */
#define PATH_SASLAUTHD_RUNDIR "/run/saslauthd"

/* Define as the return type of signal handlers (`int' or `void'). */
#define RETSIGTYPE void

/* Saslauthd runs threaded? */
/* #undef SASLAUTHD_THREADED */

/* Use BerkeleyDB for SASLdb */
/* #undef SASL_BERKELEYDB */

/* Path to default SASLdb database */
/* #undef SASL_DB_PATH */

/* Use GDBM for SASLdb */
/* #undef SASL_GDBM */

/* Use NDBM for SASLdb */
/* #undef SASL_NDBM */

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 8

/* User KERBEROS_V4 Staticly */
/* #undef STATIC_KERBEROS4 */

/* Link SASLdb Staticly */
/* #undef STATIC_SASLDB */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Use the doors IPC API */
/* #undef USE_DOORS */

/* Version number of package */
#define VERSION "2.1.26"

/* Use DES */
#define WITH_DES /**/

/* Use OpenSSL DES Implementation */
#define WITH_SSL_DES /**/

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */



#ifndef HAVE___ATTRIBUTE__
/* Can't use attributes... */
#define __attribute__(foo)
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifndef WIN32
# include <netdb.h>   
# include <sys/param.h>
#else /* WIN32 */
# include <winsock2.h>
#endif /* WIN32 */ 
#include <string.h>

#include <netinet/in.h>

#ifndef HAVE_SOCKLEN_T
typedef unsigned int socklen_t;
#endif /* HAVE_SOCKLEN_T */

#ifndef HAVE_STRUCT_SOCKADDR_STORAGE
#define _SS_MAXSIZE     128     /* Implementation specific max size */
#define _SS_PADSIZE     (_SS_MAXSIZE - sizeof (struct sockaddr))

struct sockaddr_storage {
        struct  sockaddr ss_sa;
        char            __ss_pad2[_SS_PADSIZE];
};
# define ss_family ss_sa.sa_family
#endif /* !HAVE_STRUCT_SOCKADDR_STORAGE */

#ifndef AF_INET6
/* Define it to something that should never appear */
#define AF_INET6        AF_MAX
#endif

/* Create a struct iovec if we need one */
#if !defined(HAVE_SYS_UIO_H)
struct iovec {
    long iov_len;
    char *iov_base;
};
#else
#include <sys/types.h>
#include <sys/uio.h>
#endif

#ifndef HAVE_GETADDRINFO
#define getaddrinfo     sasl_getaddrinfo
#define freeaddrinfo    sasl_freeaddrinfo
#define getnameinfo     sasl_getnameinfo
#define gai_strerror    sasl_gai_strerror
#include "gai.h"
#endif

#ifndef AI_NUMERICHOST   /* support glibc 2.0.x */
#define	AI_NUMERICHOST	4
#define NI_NUMERICHOST	2
#define NI_NAMEREQD	4
#define NI_NUMERICSERV	8
#endif

/* handy string manipulation functions */
#ifndef HAVE_STRLCPY
extern size_t saslauthd_strlcpy(char *dst, const char *src, size_t len);
#define strlcpy(x,y,z) saslauthd_strlcpy((x),(y),(z))
#endif
#ifndef HAVE_STRLCAT
extern size_t saslauthd_strlcat(char *dst, const char *src, size_t len);
#define strlcat(x,y,z) saslauthd_strlcat((x),(y),(z))
#endif
#ifndef HAVE_ASPRINTF
extern int asprintf(char **str, const char *fmt, ...);
#endif

#endif

