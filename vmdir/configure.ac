AC_PREREQ(2.59)

AC_INIT([vmdir], [1.0.0], [support@vmware.com])
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
    linux*:aarch64)
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

ADDACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([addacl],
    [AC_HELP_STRING([--enable-addacl], [enable acl check on add (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            ADDACL_CHECK_ENABLED="no"
        fi
    ])

if test x"$ADDACL_CHECK_ENABLED" = x"no"
then
AC_DEFINE_UNQUOTED(ADDACL_CHECK_DISABLED, "1", [ Disable ACL checks on add ])
fi

MODACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([modacl],
    [AC_HELP_STRING([--enable-modacl], [enable acl check on modify (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            MODACL_CHECK_ENABLED="no"
        fi
    ])

if test x"$MODACL_CHECK_ENABLED" = x"no"
then
AC_DEFINE_UNQUOTED(MODACL_CHECK_DISABLED, "1", [ Disable ACL checks on modify ])
fi

DELACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([delacl],
    [AC_HELP_STRING([--enable-delacl], [enable acl check on delete (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            DELACL_CHECK_ENABLED="no"
        fi
    ])

if test x"$DELACL_CHECK_ENABLED" = x"no"
then
AC_DEFINE_UNQUOTED(DELACL_CHECK_DISABLED, "1", [ Disable ACL checks on delete ])
fi

SEAACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([seaacl],
    [AC_HELP_STRING([--enable-seaacl], [enable acl check on search (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            SEAACL_CHECK_ENABLED="no"
        fi
    ])

if test x"$SEAACL_CHECK_ENABLED" = x"no"
then
AC_DEFINE_UNQUOTED(SEAACL_CHECK_DISABLED, "1", [ Disable ACL checks on search ])
fi

# datastore flag

AC_ARG_WITH([datastore],
    [AC_HELP_STRING([--with-datastore=<datastore>], [use backend datatore <datastore> ])],
    [
        VMDIRD_DATASTORE="$withval"
    ])

ENABLE_LIGHTWAVE_BUILD=no
AC_ARG_ENABLE([lightwave-build],
    [AC_HELP_STRING([--enable-lightwave-build], [enable lightwave build(default: no)])],
    [
        if test x"$enableval" = x"yes"
        then
            AC_DEFINE_UNQUOTED(LIGHTWAVE_BUILD, 1, [ enable lightwave specific build ])
        fi
    ])


# client only build

ENABLE_SERVER=false
AC_ARG_ENABLE([server],
    [AC_HELP_STRING([--enable-server], [enable full build(default: no)])],
    [
        if test x"$enableval" = x"yes"
        then
            ENABLE_SERVER=true
        fi
    ])

AM_CONDITIONAL(ENABLE_SERVER, [$ENABLE_SERVER])

# openssl component

case "${host_os}:${host_cpu}" in
    linux*:x86_64)
    OPEN_SSL_DEFAULT_PATH=/opt/vmware/lib64
    ;;
    darwin*:x86_64)
    OPEN_SSL_DEFAULT_PATH=/opt/vmware/lib
    ;;
    linux*:aarch64)
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

# zlib component

AC_ARG_WITH([zlib],
    [AC_HELP_STRING([--with-zlib=<dir>], [use zlib binaries rooted at prefix <dir> ])],
    [
        ZLIB_BASE_PATH="$withval"
        ZLIB_INCLUDES="-I$withval/include"
        ZLIB_LDFLAGS="-L$withval/lib -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

AC_ARG_WITH([zlib-includes],
    [AC_HELP_STRING([--with-zlib-includes=<dir>], [use ZLIB headers located in prefix <dir> ])],
    [
        ZLIB_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([zlib-libs],
    [AC_HELP_STRING([--with-zlib-libs=<dir>], [use ZLIB libraries located in prefix <dir> ])],
    [
        ZLIB_LDFLAGS="-L$withval"
    ])

AC_SUBST(ZLIB_BASE_PATH)
AC_SUBST(ZLIB_INCLUDES)
AC_SUBST(ZLIB_LDFLAGS)

# bzip component

AC_ARG_WITH([bzip],
    [AC_HELP_STRING([--with-bzip=<dir>], [use BZIP binaries rooted at prefix <dir> ])],
    [
        BZIP_BASE_PATH="$withval"
        BZIP_INCLUDES="-I$withval/include"
        BZIP_LDFLAGS="-L$withval/lib -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

AC_ARG_WITH([bzip-includes],
    [AC_HELP_STRING([--with-bzip-includes=<dir>], [use BZIP headers located in prefix <dir> ])],
    [
        BZIP_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([bzip-libs],
    [AC_HELP_STRING([--with-bzip-libs=<dir>], [use BZIP libraries located in prefix <dir> ])],
    [
        BZIP_LDFLAGS="-L$withval"
    ])

AC_SUBST(BZIP_BASE_PATH)
AC_SUBST(BZIP_INCLUDES)
AC_SUBST(BZIP_LDFLAGS)

# vmevent component

AC_ARG_WITH([vmevent],
    [AC_HELP_STRING([--with-vmevent=<dir>], [use event-server binaries rooted at prefix <dir> ])],
    [
        VMEVENT_INCLUDES="-I$withval/include"
    ])

AC_CHECK_HEADERS(vmevent.h)
AC_SUBST(VMEVENT_INCLUDES)

# Cyrus SASL

AC_ARG_WITH([sasl],
    [AC_HELP_STRING([--with-sasl=<dir>], [use SASL binaries rooted at prefix <dir> ])],
    [
        SASL_BASE_PATH="$withval"
        SASL_INCLUDES="-I$withval/include"
        SASL_LIBPATH="$withval/lib64"
        SASL_LDFLAGS="-L$withval/lib64"
        SASL_PLUGINSPATH="$withval/lib64/sasl2"
    ])

AC_ARG_WITH([sasl-includes],
    [AC_HELP_STRING([--with-sasl-includes=<dir>], [use SASL headers located in prefix <dir> ])],
    [
        SASL_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([sasl-libs],
    [AC_HELP_STRING([--with-sasl-libs=<dir>], [use SASL libraries located in prefix <dir> ])],
    [
        SASL_LIBPATH="$withval/lib64"
        SASL_LDFLAGS="-L$withval"
    ])

AC_ARG_WITH([sasl-plugins],
    [AC_HELP_STRING([--with-sasl-plugins=<dir>], [assume SASL plugins at runtime are located in <dir> ])],
    [
        SASL_PLUGINSPATH="$withval"
    ])

AC_SUBST(SASL_BASE_PATH)
AC_SUBST(SASL_INCLUDES)
AC_SUBST(SASL_LDFLAGS)

AC_DEFINE_UNQUOTED(VMDIR_CONFIG_SASL2_LIB_PATH, "$SASL_PLUGINSPATH", [SASL2 Library path])

# Trident

AC_ARG_WITH([trident],
    [AC_HELP_STRING([--with-trident=<dir>], [use trident-server binaries rooted at prefix <dir> ])],
    [
        TRIDENT_BASE_PATH="$withval"
        TRIDENT_INCLUDES="-I$withval/include"
        TRIDENT_LDFLAGS="-L$withval/lib64"
    ])

TRIDENT_LIBS="-lrestengine"
AC_CHECK_HEADERS(vmrest.h)
AC_SUBST(TRIDENT_BASE_PATH)
AC_SUBST(TRIDENT_INCLUDES)
AC_SUBST(TRIDENT_LDFLAGS)

# Jansson

AC_ARG_WITH([jansson],
    [AC_HELP_STRING([--with-jansson=<dir>], [use jansson binaries rooted at prefix <dir> ])],
    [
        JANSSON_BASE_PATH="$withval"
        JANSSON_INCLUDES="-I$withval/include"
        JANSSON_LDFLAGS="-L$withval/lib"
    ])

JANSSON_LIBS="-ljansson"
AC_CHECK_HEADERS(jansson.h)
AC_SUBST(JANSSON_BASE_PATH)
AC_SUBST(JANSSON_INCLUDES)
AC_SUBST(JANSSON_LDFLAGS)

# Copenapi

AC_ARG_WITH([copenapi],
    [AC_HELP_STRING([--with-copenapi=<dir>], [use copenapi binaries rooted at prefix <dir> ])],
    [
        COPENAPI_BASE_PATH="$withval"
        COPENAPI_INCLUDES="-I$withval/include"
        COPENAPI_LDFLAGS="-L$withval/lib"
    ])

COPENAPI_LIBS="-lcopenapi"
AC_CHECK_HEADERS(copenapi/copenapi.h)
AC_SUBST(COPENAPI_BASE_PATH)
AC_SUBST(COPENAPI_INCLUDES)
AC_SUBST(COPENAPI_LDFLAGS)

# OIDC

AC_ARG_WITH([ssocommon],
    [AC_HELP_STRING([--with-ssocommon=<dir>], [use ssocommon binaries rooted at prefix <dir> ])],
    [
        SSOCOMMON_BASE_PATH="$withval"
        SSOCOMMON_INCLUDES="-I$withval/include"
        SSOCOMMON_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

SSOCOMMON_LIBS="-lssocommon"
AC_SUBST(SSOCOMMON_BASE_PATH)
AC_SUBST(SSOCOMMON_INCLUDES)
AC_SUBST(SSOCOMMON_LDFLAGS)

AC_ARG_WITH([oidc],
    [AC_HELP_STRING([--with-oidc=<dir>], [use oidc binaries rooted at prefix <dir> ])],
    [
        OIDC_BASE_PATH="$withval"
        OIDC_INCLUDES="-I$withval/include"
        OIDC_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

OIDC_LIBS="-lssooidc"
AC_CHECK_HEADERS(oidc.h)
AC_CHECK_HEADERS(oidc_types.h)
AC_CHECK_HEADERS(common_types.h)
AC_SUBST(OIDC_BASE_PATH)
AC_SUBST(OIDC_INCLUDES)
AC_SUBST(OIDC_LDFLAGS)

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
    [AC_HELP_STRING([--with-likewise-rpath=<dir>], [use likewise libraries located at <dir> at runtime])],
    [
        LW_RPATH="$withval"
    ])

AC_ARG_WITH([likewise],
    [AC_HELP_STRING([--with-likewise=<dir>], [use likewise binaries rooted at prefix <dir> ])],
    [
        LW_BASE_PATH="$withval"
        LW_INCLUDES="-I$withval/include"
        LW_LDFLAGS="-L$withval/$PLATFORM_LIB_PREFIX -Wl,-rpath,$LW_RPATH -Wl,-rpath-link,$withval/$PLATFORM_LIB_PREFIX"
    ])

AC_ARG_WITH([likewise-includes],
    [AC_HELP_STRING([--with-likewise-includes=<dir>], [use likewise headers located in prefix <dir> ])],
    [
        LW_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([likewise-libs],
    [AC_HELP_STRING([--with-likewise-libs=<dir>], [use likewise libraries located in prefix <dir> ])],
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

AC_ARG_ENABLE([pac],
     AS_HELP_STRING([--disable-pac], [Disable Kerberos PAC]))
AM_CONDITIONAL([VMDIR_ENABLE_PAC], [test "x$enable_pac" != "xno"]])
if test x"$enable_pac" != x"no"
then
AC_DEFINE_UNQUOTED(VMDIR_ENABLE_PAC, 1, [ Enable PAC ])
fi


saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LW_INCLUDES"
AC_CHECK_HEADERS(lw/base.h lw/types.h uuid/uuid.h limits.h)
AC_CHECK_HEADERS(openssl/crypto.h openssl/ssl.h openssl/err.h)
CPPFLAGS="$saved_CPPFLAGS"

AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h)
AC_CHECK_HEADERS(pwd.h grp.h nss.h nss_common.h nsswitch.h shadow.h crypt.h)
AC_CHECK_HEADERS(termios.h)

AC_C_CONST
AC_TYPE_SIZE_T

AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strerror)

AC_CHECK_LIB([dl], [dlopen], [DL_LIBS="-ldl"])
AC_CHECK_LIB([pthread], [pthread_self], [PTHREAD_LIBS="-lpthread"])
AC_CHECK_LIB([z],[inflate],[ZLIB_LIBS="-lz"],[],[$ZLIB_LDFLAGS])
AC_CHECK_LIB([bz2],[BZ2_bzdopen],[BZIP_LIBS="-lbz2"],[],[$BZIP_LDFLAGS])
AC_CHECK_LIB([uuid],[uuid_copy], [UUID_LIBS="-luuid"], [], [$LW_LDFLAGS -luuid])
AC_CHECK_LIB(
    [crypto],
    [MD5_Init],
    [CRYPTO_LIBS="-lcrypto -lssl"],
    [],
    [$OPENSSL_LDFLAGS])
AC_CHECK_LIB([lber], [ber_scanf], [LBER_LIBS="-llber"], [], [$LW_LDFLAGS -llber])
AC_CHECK_LIB(
    [gssapi_krb5],
    [gss_accept_sec_context],
    [GSSAPI_LIBS="-lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err"],
    [],
    [$LW_LDFLAGS -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err])
AC_CHECK_LIB([sasl2], [sasl_server_init], [SASL_LIBS="-lsasl2"], [], [$SASL_LDFLAGS])
AC_CHECK_LIB([ldap_r], [ldap_initialize], [LDAP_LIBS="-lldap_r -llber"], [], [$LW_LDFLAGS -llber $SASL_LDFLAGS $SASL_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [lwbase],
    [LwRtlMemoryAllocate],
    [LWBASE_LIBS="-llwbase -llwbase_nothr"],
    [],
    [$LW_LDFLAGS -llwbase_nothr])
AC_CHECK_LIB(
    [lwadvapi],
    [LwFreeMemory],
    [LWADVAPI_LIBS="-llwadvapi -llwadvapi_nothr"],
    [],
    [$LW_LDFLAGS -llwadvapi_nothr $LWBASE_LIBS $LDAP_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
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
    [$DCERPC_LDFLAGS $OPENSSL_LDFLAGS $GSSAPI_LIBS $SCHANNEL_LIBS $CRYPTO_LIBS $LWIO_LIBS
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS
    ])

AC_CHECK_LIB([shadow], [getspnam], [SHADOW_LIBS="-lshadow"])

# Use -lxcrypt on SLES11, which contains advanced hash algorithms
AC_CHECK_LIB([xcrypt], [crypt_r], [CRYPT_LIBS="-L/usr/lib64 -lxcrypt $DL_LIBS"], [CRYPT_LIBS="no"], [-L/usr/lib64 $DL_LIBS])
if test x"$CRYPT_LIBS" = x"no"; then
  AC_CHECK_LIB([crypt], [crypt_r], [CRYPT_LIBS="-lcrypt"],[CRYPT_LIBS=""])
fi

AC_SUBST(DL_LIBS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(MDB_LIBS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(BZIP_LIBS)
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(LWBASE_LIBS)
AC_SUBST(LWADVAPI_LIBS)
AC_SUBST(LWMSG_LIBS)
AC_SUBST(LWREG_LIBS)
AC_SUBST(LWRSUTILS_LIBS)
AC_SUBST(CRYPTO_LIBS)
AC_SUBST(SCHANNEL_LIBS)
AC_SUBST(LWIO_LIBS)
AC_SUBST(DCERPC_LIBS)
AC_SUBST(UUID_LIBS)
AC_SUBST(LDAP_LIBS)
AC_SUBST(LBER_LIBS)
AC_SUBST(SHADOW_LIBS)
AC_SUBST(CRYPT_LIBS)
AC_SUBST(SASL_LIBS)
AC_SUBST(TRIDENT_LIBS)
AC_SUBST(JANSSON_LIBS)
AC_SUBST(COPENAPI_LIBS)
AC_SUBST(SSOCOMMON_LIBS)
AC_SUBST(OIDC_LIBS)

AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

if test x"$localstatedir" = x"/var"; then
    vmdirdbdir="$localstatedir/lib/vmware/vmdir"
else
    vmdirdbdir="$localstatedir"
fi
AC_SUBST(vmdirdbdir)
AS_AC_EXPAND(VMDIR_DB_DIR, $vmdirdbdir)
AC_DEFINE_UNQUOTED(VMDIR_DB_DIR, "$VMDIR_DB_DIR", [Database directory])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

AS_AC_EXPAND(VMDIR_SBIN, ["${sbindir}"])
VMDIR_SBIN_DIR=$VMDIR_SBIN
AC_SUBST(VMDIR_SBIN_DIR)

vmdirlibdir=$prefix/lib64
AC_SUBST(vmdirlibdir)
AS_AC_EXPAND(VMDIR_LIB_DIR, $vmdirlibdir)
AC_DEFINE_UNQUOTED(VMDIR_LIB_DIR, "$VMDIR_LIB_DIR", [Lib directory])

vmdirsasldir=$vmdirlibdir/sasl2
AC_SUBST(vmdirsasldir)

vmdirlogdir=/var/log/vmware/vmdir/
AC_ARG_WITH([logdir],
   [AC_HELP_STRING([--with-logdir], [set the logging directory for vmdir])],
   [
        vmdirlogdir="$withval"
   ])
AC_SUBST(vmdirlogdir)
AS_AC_EXPAND(VMDIR_LOG_DIR, $vmdirlogdir)
AC_DEFINE_UNQUOTED(VMDIR_LOG_DIR, "$VMDIR_LOG_DIR", [Log directory])

vmdir_version=1.0
AC_ARG_WITH([version],
    [AC_HELP_STRING([--with-version], [set the version (default: 1.0)])],
    [
        vmdir_version="$withval"
    ])
AC_SUBST(vmdir_version)
AS_AC_EXPAND(VDIR_PSC_VERSION, $vmdir_version)
AC_DEFINE_UNQUOTED(VDIR_PSC_VERSION, "$VDIR_PSC_VERSION", [PSC version])

vmdirconfdir="$datadir/config"
AC_SUBST(vmdirconfdir)
AS_AC_EXPAND(VMDIR_CONFIG_DIR, $vmdirconfdir)
AC_DEFINE_UNQUOTED(VMDIR_CONFIG_DIR, "$VMDIR_CONFIG_DIR", [Config directory])

AS_AC_EXPAND(VMDIR_PREFIX, ["${prefix}"])
VMDIR_PREFIX_DIR=$VMDIR_PREFIX
AC_SUBST(VMDIR_PREFIX_DIR)

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/public/Makefile
                 config/Makefile
                 kdccommon/Makefile
                 common/Makefile
                 server/Makefile
                 server/kdcsrvcommon/Makefile
                 server/kdckrb5/Makefile
                 server/kdctools/Makefile
                 server/vmkdc_mit_tools/Makefile
                 server/vmkdc/Makefile
                 server/common/Makefile
                 server/backend/Makefile
                 server/mdb-store/Makefile
                 server/indexcfg/Makefile
                 server/middle-layer/Makefile
                 server/schema/Makefile
                 server/acl/Makefile
                 server/ldap-head/Makefile
                 server/replication/Makefile
                 server/saslvmdirdb/Makefile
                 server/vmdir/Makefile
                 client/Makefile
                 config/vmdir.reg
                 testing/Makefile
                 testing/query/Makefile
                 testing/kerberos/Makefile
                 testing/test_lib/Makefile
                 testing/test_runner/Makefile
                 testing/integration_tests/Makefile
                 testing/integration_tests/acls/Makefile
                 testing/integration_tests/misc/Makefile
                 testing/integration_tests/multitenancy/Makefile
                 testing/integration_tests/passwordapis/Makefile
                 testing/integration_tests/search/Makefile
                 testing/unittests/Makefile
                 testing/unittests/libcommon/Makefile
                 tools/Makefile
                 tools/vdcaclmgr/Makefile
                 tools/vdcadmintool/Makefile
                 tools/vdcpromo/Makefile
                 tools/vdcpass/Makefile
                 tools/vdcrepadmin/Makefile
                 tools/vdcsetupldu/Makefile
                 tools/vdcbackup/Makefile
                 tools/vmkdc_admin/Makefile
                 tools/vdcsrp/Makefile
                 tools/vdcupgrade/Makefile
                 tools/vdcleavefed/Makefile
                 tools/vdcresetMachineActCred/Makefile
                 tools/vdcmetric/Makefile
                 tools/vdcschema/Makefile
                 tools/test/Makefile
                 tools/test/circularbuffer/Makefile
                 tools/test/dequetest/Makefile
                 tools/test/parseargs/Makefile
                 tools/test/registry/Makefile
                 tools/test/string/Makefile
                 tools/test/vmdirclienttest/Makefile
                 thirdparty/Makefile
                 thirdparty/openldap/Makefile
                 thirdparty/openldap/libraries/Makefile
                 thirdparty/openldap/libraries/mdb/Makefile
                 thirdparty/heimdal/Makefile
                 thirdparty/heimdal/asn1/Makefile
                 thirdparty/heimdal/krb5-crypto/Makefile
                 thirdparty/heimdal/ntlm/Makefile
                 gssapi-plugins/Makefile
                 gssapi-plugins/ntlm/Makefile
                 gssapi-plugins/srp/Makefile
                 gssapi-plugins/unix/Makefile
                 thirdparty/csrp/Makefile
                ])
AC_OUTPUT
