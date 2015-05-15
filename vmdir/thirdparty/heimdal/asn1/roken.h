#ifndef _ROKEN_H
#define _ROKEN_H

#define ROKEN_LIB_FUNCTION 
#define ROKEN_LIB_CALL
#ifndef _WIN32
#define RCSID(X) static char __rcsid__[] __attribute__((unused))  = X
#else
#define RCSID(X) static char __rcsid__[]  = X
#define __attribute__(x)
#endif

struct rk_strpool;

#ifndef rk_SOCK_EXIT
#ifdef _WIN32
#define rk_SOCK_EXIT() WSACleanup()
#define strcasecmp _stricmp
#else
#define rk_SOCK_EXIT() do { } while(0)
#endif
#endif /* rk_SOCK_EXIT */

ROKEN_LIB_FUNCTION char * ROKEN_LIB_CALL
rk_strpoolcollect(struct rk_strpool *);

ROKEN_LIB_FUNCTION struct rk_strpool * ROKEN_LIB_CALL
rk_strpoolprintf(struct rk_strpool *, const char *, ...)
    __attribute__ ((format (printf, 2, 3)));

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
rk_strpoolfree(struct rk_strpool *);

#ifndef HAVE_BSWAP32
#define bswap32 rk_bswap32
ROKEN_LIB_FUNCTION unsigned int ROKEN_LIB_CALL bswap32(unsigned int);
#endif

#ifndef HAVE_BSWAP16
#define bswap16 rk_bswap16
ROKEN_LIB_FUNCTION unsigned short ROKEN_LIB_CALL bswap16(unsigned short);
#endif

#ifdef _WIN32
#ifdef _WIN64
// You will be in trouble here if ssize_t/size_t are already typedef'ed
typedef __int64 ssize_t;
typedef unsigned __int64 size_t;
#else
typedef __int32 ssize_t;
typedef unsigned __int32 size_t;
#endif
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//typedef unsigned int * uintptr_t;
#define rk_UNCONST(x) ((void *)(uintptr_t)(const void *)(x))
#ifdef _WIN32
#define strdup _strdup

#define UNREACHABLE(x) x
#ifndef EPROTO
/* TBD: FIXME */
#define EPROTO 71  
#endif

#ifndef ENODATA
/* TBD: FIXME */
#define ENODATA 61
#endif

#else
#define UNREACHABLE(x)
#endif

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef strlcat
#define strlcat strncat
#endif

#ifndef ct_memcmp
#define ct_memcmp memcmp
#endif


#endif
