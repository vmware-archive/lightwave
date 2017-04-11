AC_PREREQ(2.59)

AC_INIT([vmsts], [6.0.0], [support@vmware.com])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
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

# jansson components

AC_ARG_WITH([jansson],
    [AC_HELP_STRING([--with-jansson=<dir>], [use jansson binaries rooted at prefix <dir> ])],
    [
        JANSSON_BASE_PATH="$withval"
        JANSSON_INCLUDES="-I$withval/include"
        JANSSON_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([jansson-includes],
    [AC_HELP_STRING([--with-jansson-includes=<dir>], [use jansson headers located in prefix <dir> ])],
    [
        JANSSON_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([jansson-libs],
    [AC_HELP_STRING([--with-jansson-libs=<dir>], [use jansson libraries located in prefix <dir> ])],
    [
        JANSSON_LDFLAGS="-L$withval"
    ])

AC_SUBST(JANSSON_BASE_PATH)
AC_SUBST(JANSSON_INCLUDES)
AC_SUBST(JANSSON_LDFLAGS)

# CURL components

AC_ARG_WITH([curl],
    [AC_HELP_STRING([--with-curl=<dir>], [use CURL binaries rooted at prefix <dir> ])],
    [
        CURL_BASE_PATH="$withval"
        CURL_INCLUDES="-I$withval/include"
        CURL_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([curl-includes],
    [AC_HELP_STRING([--with-curl-includes=<dir>], [use CURL headers located in prefix <dir> ])],
    [
        CURL_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([curl-libs],
    [AC_HELP_STRING([--with-curl-libs=<dir>], [use CURL libraries located in prefix <dir> ])],
    [
        CURL_LDFLAGS="-L$withval"
    ])

AC_SUBST(CURL_BASE_PATH)
AC_SUBST(CURL_INCLUDES)
AC_SUBST(CURL_LDFLAGS)

# vmafd component

AC_ARG_WITH([afd],
    [AC_HELP_STRING([--with-afd=<dir>], [use afd-server binaries rooted at prefix <dir> ])],
    [
        VMAFD_BASE_PATH="$withval"
        VMAFD_INCLUDES="-I$withval/include"
        VMAFD_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/usr/lib/vmware-vmafd/lib64"
    ])

AC_ARG_WITH([afd-includes],
    [AC_HELP_STRING([--with-afd-includes=<dir>], [use afd-server headers located in prefix <dir> ])],
    [
        VMAFD_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([afd-libs],
    [AC_HELP_STRING([--with-afd-libs=<dir>], [use afd-server libraries located in prefix <dir> ])],
    [
        VMAFD_LDFLAGS="-L$withval"
    ])

AC_SUBST(VMAFD_BASE_PATH)
AC_SUBST(VMAFD_INCLUDES)
AC_SUBST(VMAFD_LDFLAGS)

# openssl component

AC_ARG_WITH([ssl],
    [AC_HELP_STRING([--with-ssl=<dir>], [use SSL binaries rooted at prefix <dir> ])],
    [
        OPENSSL_BASE_PATH="$withval"
        OPENSSL_INCLUDES="-I$withval/include"
        OPENSSL_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
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

# Java component

AC_ARG_WITH([java],
    [AC_HELP_STRING([--with-java=<dir>], [use Java binaries rooted at prefix <dir> ])],
    [
        JAVA_HOME="$withval"
        JAVA=$JAVA_HOME/bin/java
        TOOLS_CLASSPATH=$JAVA_HOME/lib/tools.jar
    ])

AC_PATH_PROG([JAVAC], [javac], [no], [$PATH:$JAVA_HOME/bin])

if test x"$JAVAC" = x"no"; then
    AC_MSG_ERROR([JAVAC compiler not found])
fi

AC_SUBST(JAVA_HOME)
AC_SUBST(JAVA)
AC_SUBST(TOOLS_CLASSPATH)

# Ant component

AC_ARG_WITH([ant],
    [AC_HELP_STRING([--with-ant=<dir>], [use Ant binaries rooted at prefix <dir> ])],
    [
        ANT_HOME="$withval"
        ANT_CLASSPATH=$ANT_HOME/lib/ant.jar:$ANT_HOME/lib/ant-launcher.jar
    ])

AC_PATH_PROG([ANT], [ant], [no], [$PATH:$JAVA_HOME/bin:$ANT_HOME/bin])

if test x"$ANT" = x"no"; then
    AC_MSG_ERROR([ANT compiler not found])
fi

AC_SUBST(ANT_HOME)
AC_SUBST(ANT_CLASSPATH)

# Commons Daemon component

AC_ARG_WITH([commons-daemon],
    [AC_HELP_STRING([--with-commons-daemon=<dir>], [use Commons daemon binaries rooted at prefix <dir> ])],
    [
        COMMONS_DAEMON_HOME="$withval"
        JSVC=$COMMONS_DAEMON_HOME/bin/jsvc
    ])

AC_SUBST(COMMONS_DAEMON_HOME)
AC_SUBST(JSVC)

# JAX-WS component

AC_ARG_WITH([jax-ws],
    [AC_HELP_STRING([--with-jax-ws=<dir>], [use JAX WS binaries rooted at prefix <dir> ])],
    [
        JAX_WS_HOME="$withval"
    ])

AC_SUBST(JAX_WS_HOME)

# Maven component

AC_ARG_WITH([maven],
    [AC_HELP_STRING([--with-maven=<dir>], [use Apache Maven  binaries rooted at prefix <dir> ])],
    [
        MAVEN_HOME="$withval"
    ])

AC_SUBST(MAVEN_HOME)


# Tomcat component

AC_ARG_WITH([tomcat],
    [AC_HELP_STRING([--with-tomcat=<dir>], [use Apache Tomcat binaries rooted at prefix <dir> ])],
    [
        TOMCAT_HOME="$withval"
        TOMCAT_CLASSPATH=$TOMCAT_HOME/lib/servlet-api.jar
    ])

AC_SUBST(TOMCAT_HOME)
AC_SUBST(TOMCAT_CLASSPATH)

# Tomcat component

AC_ARG_WITH([afd],
    [AC_HELP_STRING([--with-afd=<dir>], [use VMware Authentication Framework binaries rooted at prefix <dir> ])],
    [
        VMAFD_HOME="$withval"
        VMAFD_CLASSPATH=$VMAFD_HOME/jars/authentication-framework.jar
    ])

AC_SUBST(VMAFD_HOME)
AC_SUBST(VMAFD_CLASSPATH)

# Likewise components

case "${host_os}:${host_cpu}" in
    linux*:x86_64)
    LIKEWISE_DEFAULT_PATH=/opt/likewise/lib64
    ;;
    darwin*:x86_64)
    LIKEWISE_DEFAULT_PATH=/opt/likewise/lib
    ;;
    *)
    AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
    ;;
esac

AC_ARG_WITH([likewise],
    [AC_HELP_STRING([--with-likewise=<dir>], [use likewise binaries rooted at prefix <dir> ])],
    [
        LW_BASE_PATH="$withval"
        LW_INCLUDES="-I$withval/include"
        LW_LDFLAGS="-L$withval/$PLATFORM_LIB_PREFIX -Wl,-rpath,$LIKEWISE_DEFAULT_PATH -Wl,-rpath-link,$withval/$PLATFORM_LIB_PREFIX"
    ])

AC_ARG_WITH([likewise-includes],
    [AC_HELP_STRING([--with-likewise-includes=<dir>], [use likewise headers located in prefix <dir> ])],
    [
        LW_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([likewise-libs],
    [AC_HELP_STRING([--with-likewise-libs=<dir>], [use likewise libraries located in prefix <dir> ])],
    [
        LW_LDFLAGS="-L$withval"
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
AC_CHECK_HEADERS(lw/base.h lwmem.h lw/types.h uuid/uuid.h limits.h)
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
AC_CHECK_LIB(
    [jansson],
    [json_loads],
    [JANSSON_LIBS="-ljansson"],
    [],
    [$JANSSON_LDFLAGS])
AC_CHECK_LIB(
    [curl],
    [main],
    [CURL_LIBS="-lcurl"],
    [],
    [$CURL_LDFLAGS])

AC_CHECK_LIB([shadow], [getspnam], [SHADOW_LIBS="-lshadow"])
AC_CHECK_LIB([crypt], [crypt_r], [CRYPT_LIBS="-lcrypt"])

AC_SUBST(DL_LIBS)
AC_SUBST(PTHREAD_LIBS)
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
AC_SUBST(JANSSON_LIBS)
AC_SUBST(ZLIB_LIBS)
AC_SUBST(CARES_LIBS)
AC_SUBST(CURL_LIBS)

AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

if test x"$localstatedir" = x"/var"; then
    vmstsdbdir="$localstatedir/lib/vmware/vmsts"
else
    vmstsdbdir="$localstatedir"
fi
AC_SUBST(vmstsdbdir)
AS_AC_EXPAND(VMSTS_DB_DIR, $vmstsdbdir)
AC_SUBST(VMSTS_DB_DIR)
AC_DEFINE_UNQUOTED(VMSTS_DB_DIR, "$VMSTS_DB_DIR", [Database directory])

VMSTS_PREFIX_DIR="$prefix"
AC_SUBST(VMSTS_PREFIX_DIR)

VMSTS_SBIN_DIR="$prefix/sbin"
AC_SUBST(VMSTS_SBIN_DIR)

vmstsjarsdir="$prefix/jars"
AC_SUBST(vmstsjarsdir)

vmstswebappsdir="$prefix/vmware-sts/webapps"
AC_SUBST(vmstswebappsdir)

vmstsconfdir="$prefix/vmware-sts/conf"
AC_SUBST(vmstsconfdir)

vmstsbindir="$prefix/vmware-sts/bin"
AC_SUBST(vmstsbindir)

vmidmconfdir="$datadir/config/idm"
AC_SUBST(vmidmconfdir)

systemddir="/lib/systemd/system"
AC_SUBST(systemddir)

AC_CONFIG_FILES([Makefile
                 config/setenv.sh
                 config/vmware-stsd.sh
                 config/vmware-stsd.service
                 config/configure-build.sh
                 interop/Makefile
                 interop/idm/Makefile
                 interop/idm/ad/Makefile
                 interop/idm/ad/server/Makefile
                 interop/idm/common/Makefile
                 interop/idm/localos/Makefile
                 ssoclients/Makefile
                 ssoclients/common/Makefile
                 ssoclients/common/include/Makefile
                 ssoclients/common/include/public/Makefile
                 ssoclients/common/src/Makefile
                 ssoclients/common/test/Makefile
                 ssoclients/oidc/Makefile
                 ssoclients/oidc/include/Makefile
                 ssoclients/oidc/include/public/Makefile
                 ssoclients/oidc/src/Makefile
                 ssoclients/oidc/test/Makefile
                 ssoclients/restclient/Makefile
                 ssoclients/restclient/coreclient/Makefile
                 ssoclients/restclient/coreclient/include/Makefile
                 ssoclients/restclient/coreclient/include/public/Makefile
                 ssoclients/restclient/coreclient/src/Makefile
                 ssoclients/restclient/idmclient/Makefile
                 ssoclients/restclient/idmclient/include/Makefile
                 ssoclients/restclient/idmclient/include/public/Makefile
                 ssoclients/restclient/idmclient/src/Makefile
                 ssoclients/restclient/afdclient/Makefile
                 ssoclients/restclient/afdclient/include/Makefile
                 ssoclients/restclient/afdclient/include/public/Makefile
                 ssoclients/restclient/afdclient/src/Makefile
                 ssoclients/restclient/vmdirclient/Makefile
                 ssoclients/restclient/vmdirclient/include/Makefile
                 ssoclients/restclient/vmdirclient/include/public/Makefile
                 ssoclients/restclient/vmdirclient/src/Makefile
                 ssoclients/restclient/test/Makefile
                 ssoclients/restclient/test/src/Makefile
                 build-maven/Makefile
                 make-target/Makefile
                ])
AC_OUTPUT

