AC_PREREQ(2.59)

AC_INIT([vmdns], [1.0.0], [support@vmware.com])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

AC_PROG_CC
AC_PROG_LIBTOOL

dnl Check supported operating systems
dnl

case "${host_os}:${host_cpu}" in

    linux*:x86_64)
        PLATFORM_LIB_PREFIX=lib64
        ;;
    darwin*:x86_64)
        PLATFORM_LIB_PREFIX=lib
        ;;
    *)
        AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
        ;;
esac

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -Wall -Werror -fno-strict-aliasing"

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--enable-debug], [enable debugging (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AM_CFLAGS="$AM_CFLAGS -g -O0"
            AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -DLDAP_DEBUG"
        fi
    ])

# openssl component
case "${host_os}:${host_cpu}" in
    linux*:x86_64)
    OPEN_SSL_DEFAULT_PATH=/opt/vmware/lib64
    ;;
    darwin*:x86_64)
    OPEN_SSL_DEFAULT_PATH=/opt/vmware/lib
    ;;
    *)
    AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
    ;;
esac

AC_ARG_WITH([ssl],
    [AC_HELP_STRING([--with-ssl=<dir>], [use SSL binaries rooted at prefix <dir> ])],
    [
        OPENSSL_BASE_PATH="$withval"
        OPENSSL_INCLUDES="-I$withval/include"
        OPENSSL_LDFLAGS="-L$withval/$PLATFORM_LIB_PREFIX -Wl,-rpath,$OPEN_SSL_DEFAULT_PATH"
    ])

AC_ARG_WITH([ssl-includes],
    [AC_HELP_STRING([--with-ssl-includes=<dir>], [use SSL headers located in prefix <dir> ])],
    [
        OPENSSL_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([ssl-libs],
    [AC_HELP_STRING([--with-ssl-libs=<dir>], [use SSL libraries located in prefix <dir> ])],
    [
        OPENSSL_LDFLAGS="-L$withval"
    ])

AC_SUBST(OPENSSL_BASE_PATH)
AC_SUBST(OPENSSL_INCLUDES)
AC_SUBST(OPENSSL_LDFLAGS)

# Likewise components

case "${host_os}:${host_cpu}" in

    linux*:x86_64)
    LIKEWISE_DEFAULT_PATH=/opt/likewise/lib64
    ;;
    darwin*:x86_64)
    LIKEWISE_DEFAULT_PATH=/opt/likewise/lib
    ;;
    linux*:aarch64)
    LIKEWISE_DEFAULT_PATH=/opt/likewise/lib
    ;;
    *)
    AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
    ;;
esac

LW_RPATH=$LIKEWISE_DEFAULT_PATH
AC_ARG_WITH([likewise-rpath],
    [AC_HELP_STRING([--with-likewise-rpath=<dir>], [use Likewise libraries located at <dir> at runtime])],
    [
        LW_RPATH="$withval"
    ])

AC_ARG_WITH([likewise],
    [AC_HELP_STRING([--with-likewise=<dir>], [use Likewise binaries rooted at prefix <dir> ])],
    [
        LW_BASE_PATH="$withval"
        LW_INCLUDES="-I$withval/include"
        LW_LDFLAGS="-L$withval/$PLATFORM_LIB_PREFIX -Wl,-rpath,$LW_RPATH -Wl,-rpath-link,$withval/$PLATFORM_LIB_PREFIX"
    ])

AC_ARG_WITH([likewise-includes],
    [AC_HELP_STRING([--with-likewise-includes=<dir>], [use Likewise headers located in prefix <dir> ])],
    [
        LW_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([likewise-libs],
    [AC_HELP_STRING([--with-likewise-libs=<dir>], [use Likewise libraries located in prefix <dir> ])],
    [
        LW_LDFLAGS="-L$withval -Wl,-rpath,$LW_RPATH -Wl,-rpath-link,$withval"
    ])

AC_SUBST(LW_BASE_PATH)
AC_SUBST(LW_INCLUDES)
AC_SUBST(LW_LDFLAGS)

DCERPC_PATH=$LW_BASE_PATH/bin
DCERPC_INCLUDES=$LW_INCLUDES
DCERPC_LDFLAGS=$LW_LDFLAGS

AC_ARG_WITH([dcerpc],
    [AC_HELP_STRING([--with-dcerpc=<dir>], [use DCERPC binaries rooted at prefix <dir> ])],
    [
        DCERPC_PATH="$withval/bin"
        DCERPC_INCLUDES="-I$withval/include"
        DCERPC_LDFLAGS="-L$withval/lib64"
    ])

AC_ARG_WITH([dcerpc-includes],
    [AC_HELP_STRING([--with-dcerpc-includes=<dir>], [use DCERPC headers located in prefix <dir> ])],
    [
        DCERPC_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([dcerpc-libs],
    [AC_HELP_STRING([--with-dcerpc-libs=<dir>], [use DCERPC libraries located in prefix <dir> ])],
    [
        DCERPC_LDFLAGS="-L$withval"
    ])

AC_SUBST(DCERPC_PATH)
AC_SUBST(DCERPC_INCLUDES)
AC_SUBST(DCERPC_LDFLAGS)


for i in $CPPFLAGS ${DCERPC_INCLUDES} ${LW_INCLUDES}; do
    j=`echo $i | grep '^-I'`
    if test x != x"$j"
    then
        IDLFLAGS="$IDLFLAGS $j"
    fi
done

AC_SUBST(IDLFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LW_INCLUDES"
AC_CHECK_HEADERS(lw/base.h lw/types.h uuid/uuid.h)
AC_CHECK_HEADERS(openssl/crypto.h openssl/ssl.h openssl/err.h)
CPPFLAGS="$saved_CPPFLAGS"

# VMDIR component
VMDIR_DEFAULT_PATH=/usr/lib/vmware-vmdir/lib64

VMDIR_RPATH=$VMDIR_DEFAULT_PATH
AC_ARG_WITH([vmdir-rpath],
    [AC_HELP_STRING([--with-vmdir-rpath=<dir>], [use vmdir libraries located at
<dir> at runtime])],
    [
        VMDIR_RPATH="$withval"
    ])

AC_ARG_WITH([vmdir],
    [AC_HELP_STRING([--with-vmdir=<dir>], [use VMDIR binaries rooted at prefix <dir> ])],
    [
        VMDIR_BASE_PATH="$withval"
        VMDIR_INCLUDES="-I$withval/include"
        VMDIR_LDFLAGS="-L$withval/lib64 -Wl,-rpath,$VMDIR_RPATH -Wl,-rpath-link,$withval/lib64"
    ])

AC_ARG_WITH([vmdir-includes],
    [AC_HELP_STRING([--with-vmdir-includes=<dir>], [use VMDIR headers located in prefix <dir> ])],
    [
        VMDIR_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([vmdir-libs],
    [AC_HELP_STRING([--with-vmdir-libs=<dir>], [use VMDIR libraries located in prefix <dir> ])],
    [
        VMDIR_LDFLAGS="-L$withval -Wl,-rpath,$VMDIR_RPATH -Wl,-rpath-link,$withval"
    ])

AC_SUBST(VMDIR_BASE_PATH)
AC_SUBST(VMDIR_INCLUDES)
AC_SUBST(VMDIR_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMDIR_INCLUDES $DCERPC_INCLUDES"
AC_CHECK_HEADERS(vmdirclient.h)
CPPFLAGS="$saved_CPPFLAGS"

AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h arpa/inet.h fcntl.h)
AC_CHECK_HEADERS(pwd.h grp.h nss.h nss_common.h nsswitch.h shadow.h crypt.h)

AC_C_CONST
AC_TYPE_SIZE_T

AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strerror)

AC_CHECK_LIB([dl], [dlopen], [DL_LIBS="-ldl"])
AC_CHECK_LIB([pthread], [pthread_self], [PTHREAD_LIBS="-lpthread"])
AC_CHECK_LIB([uuid],[uuid_copy], [UUID_LIBS="-luuid"], [], [$LW_LDFLAGS -luuid])
AC_CHECK_LIB(
    [gssapi_krb5],
    [gss_accept_sec_context],
    [GSSAPI_LIBS="-lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err"],
    [],
    [$LW_LDFLAGS -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err])
AC_CHECK_LIB(
    [lwbase],
    [LwRtlMemoryAllocate],
    [LWBASE_LIBS="-llwbase -llwbase_nothr"],
    [],
    [$LW_LDFLAGS -llwbase_nothr])
AC_CHECK_LIB(
    [lwmsg],
    [lwmsg_connection_new],
    [LWMSG_LIBS="-llwmsg -llwmsg_nothr"],
    [],
    [$LW_LDFLAGS -llwmsg_nothr $LWBASE_LIBS])
AC_CHECK_LIB(
    [regclient],
    [LwRegOpenKeyExW],
    [LWREG_LIBS="-lregclient -lregcommon"],
    [],
    [$LW_LDFLAGS -lregcommon $LWMSG_LIBS $LWBASE_LIBS])
AC_CHECK_LIB(
    [rsutils],
    [RegUtilSetValue],
    [LWRSUTILS_LIBS="-lrsutils"],
    [],
    [$LW_LDFLAGS -lrsutils $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS])
AC_CHECK_LIB(
    [crypto],
    [MD5_Init],
    [CRYPTO_LIBS="-lcrypto -lssl"],
    [],
    [$OPENSSL_LDFLAGS])
AC_CHECK_LIB(
    [schannel],
    [schn_init_creds],
    [SCHANNEL_LIBS="-lschannel"],
    [],
    [$LW_LDFLAGS $LWBASE_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [lwioclient],
    [LwNtCreateFile],
    [LWIO_LIBS="-llwioclient -llwioshareinfo -llwiocommon"],
    [],
    [$LW_LDFLAGS -llwioshareinfo -llwiocommon
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS $GSSAPI_LIBS
    ])
AC_CHECK_LIB(
    [dcerpc],
    [rpc__init],
    [DCERPC_LIBS="-ldcerpc"],
    [],
    [$DCERPC_LDFLAGS $GSSAPI_LIBS $SCHANNEL_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS $LWIO_LIBS
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS
    ])
AC_CHECK_LIB(
              [lber],
              [ber_scanf],
              [LBER_LIBS="-llber"],
              [],
              [$LW_LDFLAGS -llber])
#AC_CHECK_LIB([ldap_r], [ldap_initialize], [LDAP_LIBS="-lldap_r -llber -lsasl2"], [], [$LW_LDFLAGS -llber -lsasl2 $GSSAPI_LIBS])
AC_CHECK_LIB(
              [ldap_r],
              [ldap_add_ext],
              [LDAP_LIBS="-lldap_r -llber -lsasl2"],
              [],
              [$LW_LDFLAGS -llber -lsasl2 $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

AC_CHECK_LIB(
    [vmdirclient],
    [VmDirSetupHostInstance],
    [VMDIR_LIBS="-lvmdirclient"],
    [],
    [$VMDIR_LDFLAGS -lvmdirclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

AC_CHECK_LIB([shadow], [getspnam], [SHADOW_LIBS="-lshadow"])
AC_CHECK_LIB([crypt], [crypt_r], [CRYPT_LIBS="-lcrypt"])

AC_SUBST(DL_LIBS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(LWBASE_LIBS)
AC_SUBST(LWMSG_LIBS)
AC_SUBST(LWREG_LIBS)
AC_SUBST(LWRSUTILS_LIBS)
AC_SUBST(CRYPTO_LIBS)
AC_SUBST(SCHANNEL_LIBS)
AC_SUBST(LWIO_LIBS)
AC_SUBST(DCERPC_LIBS)
AC_SUBST(UUID_LIBS)
AC_SUBST(SHADOW_LIBS)
AC_SUBST(CRYPT_LIBS)
AC_SUBST(LDAP_LIBS)
AC_SUBST(LBER_LIBS)
AC_SUBST(VMDIR_LIBS)

AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

AS_AC_EXPAND(VMDNS_PREFIX_DIR, ["${prefix}"])

AS_AC_EXPAND(VMDNS_SBIN_DIR, ["${sbindir}"])

vmdnsconfdir="$datadir/config"
AC_SUBST(vmdnsconfdir)
AS_AC_EXPAND(VMDNS_CONFIG_DIR, $vmdnsconfdir)
AC_DEFINE_UNQUOTED(VMDNS_CONFIG_DIR, "$VMDNS_CONFIG_DIR", [Config dns])

initddir=$sysconfdir/init.d
AC_SUBST(initddir)

bootstrapdir=$VMDNS_PREFIX_DIR/firstboot
AC_SUBST(bootstrapdir)

firewalldir=$VMDNS_PREFIX_DIR/firewall
AC_SUBST(firewalldir)

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/public/Makefile
                 config/Makefile
                 config/vmdns.reg
                 config/vmdns-client.reg
                 common/Makefile
                 vmsock/Makefile
                 vmsock/posix/Makefile
                 vmsock/api/Makefile
                 server/Makefile
                 server/common/Makefile
                 server/vmdns/Makefile
                 client/Makefile
                 tools/Makefile
                 tools/cli/Makefile
                 test/Makefile
                ])
AC_OUTPUT

