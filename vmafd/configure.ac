AC_PREREQ(2.59)

AC_INIT([vmafd], [1.0.0], [support@vmware.com])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_CXX

dnl Check supported operating systems
dnl

case "${host_os}:${host_cpu}" in

    linux*:x86_64)
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

# vmdir-provider notify only build

AC_ARG_ENABLE([notify-vmdir-provider],
    [AC_HELP_STRING([--enable-notify-vmdir-provider], [enable vmdir provider notification (default: no)])],
    [
        if test x"$enableval" = x"yes"
        then
            AC_DEFINE_UNQUOTED(NOTIFY_VMDIR_PROVIDER, "1", [Notify VMDir Provider of Join/Leave])
        fi
    ])

# SQLITE components

AC_ARG_WITH([sqlite],
    [AC_HELP_STRING([--with-sqlite=<dir>], [use SQLITE binaries rooted at prefix <dir> ])],
    [
        SQLITE_BASE_PATH="$withval"
        SQLITE_INCLUDES="-I$withval/include"
        SQLITE_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([sqlite-includes],
    [AC_HELP_STRING([--with-sqlite-includes=<dir>], [use SQLITE headers located in prefix <dir> ])],
    [
        SQLITE_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([sqlite-libs],
    [AC_HELP_STRING([--with-sqlite-libs=<dir>], [use SQLITE libraries located in prefix <dir> ])],
    [
        SQLITE_LDFLAGS="-L$withval"
    ])
echo $(SQLITE_BASE_PATH)
AC_SUBST(SQLITE_BASE_PATH)
AC_SUBST(SQLITE_INCLUDES)
AC_SUBST(SQLITE_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $SQLITE_INCLUDES"
AC_CHECK_HEADERS(sqlite3.h)
CPPFLAGS="$saved_CPPFLAGS"

# BOOST components

AC_ARG_WITH([boost],
    [AC_HELP_STRING([--with-boost=<dir>], [use BOOST binaries rooted at prefix <dir> ])],
    [
        BOOST_BASE_PATH="$withval"
        BOOST_INCLUDES="-I$withval/include"
        BOOST_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([boost-includes],
    [AC_HELP_STRING([--with-boost-includes=<dir>], [use BOOST headers located in prefix <dir> ])],
    [
        BOOST_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([boost-libs],
    [AC_HELP_STRING([--with-boost-libs=<dir>], [use BOOST libraries located in prefix <dir> ])],
    [
        BOOST_LDFLAGS="-L$withval"
    ])

AC_ARG_WITH([boost-python-lib],
    [AC_HELP_STRING([--with-boost-python-lib=<dir>], [use BOOST Python library extension ])],
    [
        BOOST_PYTHON_EXT="$withval"
        BOOST_PYTHON_LDFLAGS="$BOOST_BASE_PATH/lib/libboost_python-$BOOST_PYTHON_EXT.a"
    ])


AC_SUBST(BOOST_BASE_PATH)
AC_SUBST(BOOST_INCLUDES)
AC_SUBST(BOOST_LDFLAGS)
AC_SUBST(BOOST_PYTHON_EXT)
AC_SUBST(BOOST_PYTHON_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_INCLUDES"
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS(boost/asio.hpp)
AC_LANG_POP([C++])
CPPFLAGS="$saved_CPPFLAGS"

# Python components

AC_ARG_WITH([python],
    [AC_HELP_STRING([--with-python=<dir>], [use Python binaries rooted at prefix <dir> ])],
    [
        PYTHON_BASE_PATH="$withval"
        PYTHON_INCLUDES="-I$withval/include/python2.7"
        PYTHON_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([python-includes],
    [AC_HELP_STRING([--with-python-includes=<dir>], [use Python headers located in prefix <dir> ])],
    [
        PYTHON_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([python-libs],
    [AC_HELP_STRING([--with-python-libs=<dir>], [use Python libraries located in prefix <dir> ])],
    [
        PYTHON_LDFLAGS="-L$withval"
    ])

AC_SUBST(PYTHON_BASE_PATH)
AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(PYTHON_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_CHECK_HEADERS(python.h)
CPPFLAGS="$saved_CPPFLAGS"





# openssl component

AC_ARG_WITH([ssl],
    [AC_HELP_STRING([--with-ssl=<dir>], [use SSL binaries rooted at prefix <dir> ])],
    [
        OPENSSL_BASE_PATH="$withval"
        OPENSSL_INCLUDES="-I$withval/include"
        OPENSSL_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64"
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

# Maven component

AC_ARG_WITH([maven],
    [AC_HELP_STRING([--with-maven=<dir>], [use Apache Maven  binaries rooted at prefix <dir> ])],
    [
        MAVEN_HOME="$withval"
    ])

AC_SUBST(MAVEN_HOME)

# Java

AC_ARG_WITH([jdk],
    [AC_HELP_STRING([--with-jdk=<dir>], [use jdk binaries rooted at prefix <dir> ])],
    [
        JAVA_HOME="$withval"
        JDK_INCLUDES="-I$withval/include -I$withval/include/linux"
        JDK_LDFLAGS="-L$withval/lib"
        TOOLS_CLASSPATH=$JAVA_HOME/lib/tools.jar
    ])

AC_ARG_WITH([jdk-includes],
    [AC_HELP_STRING([--with-jdk-includes=<dir>], [use java headers located in prefix <dir> ])],
    [
        JDK_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([jdk-libs],
    [AC_HELP_STRING([--with-jdk-libs=<dir>], [use java libraries located in prefix <dir> ])],
    [
        JDK_LDFLAGS="-L$withval"
    ])

AC_SUBST(JAVA_HOME)
AC_SUBST(JDK_INCLUDES)
AC_SUBST(JDK_LDFLAGS)
AC_SUBST(TOOLS_CLASSPATH)

JDK_PATH=$JAVA_HOME/bin
AC_SUBST(JDK_PATH)

JAVA=$JAVA_HOME/bin/java
AC_SUBST(JAVA)

JRE_EXT=$JAVA_HOME/jre/lib/ext
AC_SUBST(JRE_EXT)

AC_PATH_PROG([JAVAH], [javah], [no], [$PATH:$JDK_PATH])

if test x"$JAVAH" = x"no"; then
    AC_MSG_ERROR([JAVAH compiler not found])
fi

# ESX Platform
 
DCERPC_DEPENDENCY="lsass dcerpc"
AC_SUBST(DCERPC_DEPENDENCY)
AC_ARG_ENABLE([esx],
    [AC_HELP_STRING([--enable-esx], [enable acl check on modify (default: enabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AC_DEFINE_UNQUOTED(PLATFORM_VMWARE_ESX, "1", [ Built to run on VMware ESXi ])
            DCERPC_DEPENDENCY="lsass"
        else
            DCERPC_DEPENDENCY="lsass dcerpc"
        fi
    ])

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

# Likewise components

KRB5_DEFAULT_ENABLED="no"
AC_ARG_ENABLE([krb5-default],
    [AC_HELP_STRING([--enable-krb5-default], [enable default krb5.conf/krb5.keytab (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AC_DEFINE_UNQUOTED(USE_DEFAULT_KRB5_PATHS, "1", [ Use /etc/krb5.conf /etc/krb5.keytab ])
            KRB5_DEFAULT_ENABLED="yes"
        fi
    ])

LIKEWISE_LIB_PATH="/opt/likewise/lib64"
AC_SUBST(LIKEWISE_LIB_PATH)
LIKEWISE_LIBDIR="/opt/likewise/lib64"
AC_SUBST(LIKEWISE_LIBDIR)
AC_ARG_WITH([likewise-libdir],
    [AC_HELP_STRING([--with-likewise-libdir=<dir>], [use LIKEWISE libraries (at install) in this location])],
    [
        LIKEWISE_LIBDIR="$withval"
    ])

AC_ARG_WITH([likewise],
    [AC_HELP_STRING([--with-likewise=<dir>], [use likewise binaries rooted at prefix <dir> ])],
    [
        LW_BASE_PATH="$withval"
        LW_INCLUDES="-I$withval/include"
        LW_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/likewise/lib64 -Wl,-rpath-link,/opt/likewise/lib64"
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

AC_ARG_WITH([likewise-rpath],
    [AC_HELP_STRING([--with-likewise-rpath=<dir>], [use likewise libraries located at <dir> at runtime])],
    [
        LW_LDFLAGS="-L$LW_BASE_PATH/lib64 -Wl,-rpath,$withval -Wl,-rpath-link,$LW_BASE_PATH/lib64"
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
CPPFLAGS="$CPPFLAGS $LW_INCLUDES $OPENSSL_INCLUDES"
AC_CHECK_HEADERS(lw/base.h lw/types.h uuid/uuid.h lwrpcrt/lwrpcrt.h dce/rpc.h)
AC_CHECK_HEADERS(openssl/crypto.h openssl/ssl.h openssl/err.h openssl/x509.h)
CPPFLAGS="$saved_CPPFLAGS"

# VMDIR component

AC_ARG_WITH([vmdir],
    [AC_HELP_STRING([--with-vmdir=<dir>], [use VMDIR binaries rooted at prefix <dir> ])],
    [
        VMDIR_BASE_PATH="$withval"
        VMDIR_INCLUDES="-I$withval/include"
        VMDIR_LDFLAGS="-L$VMDIR_BASE_PATH/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,$VMDIR_BASE_PATH/lib64"
    ])

AC_ARG_WITH([vmdir-includes],
    [AC_HELP_STRING([--with-vmdir-includes=<dir>], [use VMDIR headers located in prefix <dir> ])],
    [
        VMDIR_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([vmdir-libs],
    [AC_HELP_STRING([--with-vmdir-libs=<dir>], [use VMDIR libraries located in prefix <dir> ])],
    [
        VMDIR_LDFLAGS="-L$withval"
    ])

AC_ARG_WITH([vmdir-rpath],
    [AC_HELP_STRING([--with-vmdir-rpath=<dir>], [use VMDIR runtime libraries located at <dir> ])],
    [
        VMDIR_LDFLAGS="-L$VMDIR_BASE_PATH/lib64 -Wl,-rpath,$withval -Wl,-rpath-link,$VMDIR_BASE_PATH/lib64"
    ])

AC_SUBST(VMDIR_BASE_PATH)
AC_SUBST(VMDIR_INCLUDES)
AC_SUBST(VMDIR_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMDIR_INCLUDES $DCERPC_INCLUDES"
AC_CHECK_HEADERS(vmdirclient.h)
CPPFLAGS="$saved_CPPFLAGS"

# VMDNS component

AC_ARG_WITH([vmdns],
    [AC_HELP_STRING([--with-vmdns=<dir>], [use VMDNS binaries rooted at prefix <dir> ])],
    [
        VMDNS_BASE_PATH="$withval"
        VMDNS_INCLUDES="-I$withval/include"
        VMDNS_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/usr/lib/vmware-vmdns/lib64 -Wl,-rpath-link,/usr/lib/vmware-vmdns/lib64"
    ])

AC_ARG_WITH([vmdns-includes],
    [AC_HELP_STRING([--with-vmdns-includes=<dir>], [use VMDNS headers located in prefix <dir> ])],
    [
        VMDNS_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([vmdns-libs],
    [AC_HELP_STRING([--with-vmdns-libs=<dir>], [use VMDNS libraries located in prefix <dir> ])],
    [
        VMDNS_LDFLAGS="-L$withval"
    ])

AC_SUBST(VMDNS_BASE_PATH)
AC_SUBST(VMDNS_INCLUDES)
AC_SUBST(VMDNS_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMDNS_INCLUDES $DCERPC_INCLUDES"
AC_CHECK_HEADERS(vmdns.h)
CPPFLAGS="$saved_CPPFLAGS"


AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h sys/stat.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h sys/un.h dirent.h termios.h term.h)
AC_CHECK_HEADERS(pwd.h grp.h nss.h nss_common.h nsswitch.h shadow.h crypt.h)
AC_CHECK_HEADERS(limits.h)

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
    [ldap_r],
    [ldap_initialize],
    [LDAP_LIBS="-lldap_r -llber -lsasl2"],
    [],
    [$LW_LDFLAGS -llber -lsasl2 $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
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
    [lwadvapi],
    [LwAllocateMemory],
    [LWADVAPI_LIBS="-llwadvapi -llwadvapi_nothr"],
    [],
    [$LW_LDFLAGS $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS $GSSAPI_LIBS
    $UUID_LIBS $LDAP_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS -llwadvapi_nothr])
AC_CHECK_LIB(
    [lwsm],
    [LwSmStartService],
    [LWSM_LIBS="-llwsm -llwsmcommon"],
    [],
    [$LW_LDFLAGS $LWMSG_LIBS $LWADVAPI_LIBS $LWBASE_LIBS $GSSAPI_LIBS $UUID_LIBS $LDAP_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS -llwsmcommon]
    )

AC_CHECK_LIB(
    [domainjoin],
    [DJJoinDomain],
    [DOMAINJOIN_LIBS="-ldomainjoin -lcentutils -leventlog -leventlogutils -llsaclient -llsacommon -llwnetclientapi -llwnetcommon -llwsm -llwsmcommon"],
    [],
    [$LW_LDFLAGS -ldomainjoin -lcentutils -leventlog -leventlogutils -llsaclient -llsacommon -llwnetclientapi -llwnetcommon -llwsm -llwsmcommon $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS -llwadvapi_nothr])
AC_CHECK_LIB(
    [vmdirclient],
    [VmDirSetupHostInstance],
    [VMDIR_LIBS="-lvmdirclient"],
    [],
    [$VMDIR_LDFLAGS -lvmdirclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [vmdnsclient],
    [VmDnsOpenServerA],
    [VMDNS_LIBS="-lvmdnsclient"],
    [],
    [$VMDNS_LDFLAGS -lvmdnsclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])


AC_CHECK_LIB([shadow], [getspnam], [SHADOW_LIBS="-lshadow"])
AC_CHECK_LIB([crypt], [crypt_r], [CRYPT_LIBS="-lcrypt"])
AC_LANG_PUSH([C++])
AC_CHECK_LIB(
        [boost_unit_test_framework],
        [main],
        [BOOST_LIBS="-lboost_thread -lboost_system -lboost_filesystem -lboost_program_options"],
        [],
        [$BOOST_LDFLAGS])
AC_LANG_POP([C++])

AC_CHECK_LIB([sqlite3], [sqlite3_open], [SQLITE_LIBS="-lsqlite3"], [], [$SQLITE_LDFLAGS $DL_LIBS $PTHREAD_LIBS])
AC_CHECK_LIB([python2.7 ],  [PyArg_Parse], [PYTHON_LIBS="-lpython2.7"], [], [$PYTHON_LDFLAGS])

AC_SUBST(DL_LIBS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(LWBASE_LIBS)
AC_SUBST(DOMAINJOIN_LIBS)
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
AC_SUBST(BOOST_LIBS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(SQLITE_LIBS)
AC_SUBST(LWADVAPI_LIBS)
AC_SUBST(LWSM_LIBS)
AC_SUBST(LDAP_LIBS)
AC_SUBST(VMDIR_LIBS)
AC_SUBST(VMDNS_LIBS)

AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

AS_AC_EXPAND(VMAFD_BIN_DIR, $prefix)
AC_SUBST(VMAFD_BIN_DIR)

AS_AC_EXPAND(VMAFD_SBIN_DIR, ["${sbindir}"])
AC_SUBST(VMAFD_SBIN_DIR)

if test x"$localstatedir" = x"/var"; then
    vmafddbdir="$localstatedir/lib/afd"
else
    vmafddbdir="$localstatedir"
fi
AC_SUBST(vmafddbdir)
AS_AC_EXPAND(VMAFD_DB_DIR, $vmafddbdir)
AC_DEFINE_UNQUOTED(VMAFD_DB_DIR, "$VMAFD_DB_DIR", [Database directory])

vmafdconfdir="$datadir/config"
AC_SUBST(vmafdconfdir)
AS_AC_EXPAND(VMAFD_CONFIG_DIR, $vmafdconfdir)
AC_SUBST(VMAFD_CONFIG_DIR)
AC_DEFINE_UNQUOTED(VMAFD_CONFIG_DIR, "$VMAFD_CONFIG_DIR", [Config afd])

if test x"$KRB5_DEFAULT_ENABLED" = x"yes"; then
VMAFD_KEYTAB_CONFIG_DIR="/etc"
VMAFD_KRB5CONF_FILENAME="krb5.conf"
VMAFD_KRB5CONF_FILEPATH="/etc/${VMAFD_KRB5CONF_FILENAME}"
else
VMAFD_KEYTAB_CONFIG_DIR="$VMAFD_CONFIG_DIR"
VMAFD_KRB5CONF_FILENAME="krb5.lotus.conf"
VMAFD_KRB5CONF_FILEPATH="/etc/${VMAFD_KRB5CONF_FILENAME}"
fi
AC_DEFINE_UNQUOTED(VMAFD_KEYTAB_CONFIG_DIR, "$VMAFD_KEYTAB_CONFIG_DIR", [Config afd])
AC_SUBST(VMAFD_KEYTAB_CONFIG_DIR)
AC_SUBST(VMAFD_KRB5CONF_FILENAME)
AC_SUBST(VMAFD_KRB5CONF_FILEPATH)

databasedir=$localstatedir
AC_SUBST(databasedir)
AS_AC_EXPAND(VMAFD_DB_DIR, $databasedir)
AC_DEFINE_UNQUOTED(VMAFD_DB_DIR, "$VMAFD_DB_DIR", [DB path for vmafd])

initddir=$sysconfdir/init.d
AC_SUBST(initddir)

pymodulesdir=$prefix/site-packages/identity
AC_SUBST(pymodulesdir)

javaetcdir=/etc/vmware/java
AC_SUBST(javaetcdir)

vmafdjarsdir="$prefix/jars"
AC_SUBST(vmafdjarsdir)

jreextdir=$JRE_EXT
AC_SUBST(jreextdir)

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/public/Makefile
                 config/Makefile
                 config/vmafd.reg
                 common/Makefile
                 vmafcfg/Makefile
                 vmafcfg/api/Makefile
                 vmafcfg/posix/Makefile
                 vmevent/Makefile
                 vmevent/common/Makefile
                 vmevent/db/Makefile
                 vmevent/server/Makefile
                 vmevent/client/Makefile
                 vmevent/test/Makefile
                 server/Makefile
                 server/db/Makefile
                 server/vmafd/Makefile
                 server/vmafd/vmafd-server-defines.h
                 client/Makefile
                 jdepends/Makefile
                 tools/Makefile
                 tools/dir-cli/Makefile
                 tools/cdc-cli/Makefile
                 tools/cli/Makefile
                 tools/domainjoin/Makefile
                 tools/vdcpromo/Makefile
                 tools/vecs-cli/Makefile
                 tools/sl-cli/Makefile
                 interop/Makefile
                 interop/cdcjni/Makefile
                 interop/heartbeatjni/Makefile
                 interop/java/Makefile
                 interop/jni/Makefile
                 interop/python/Makefile
		         test/Makefile
                ])
AC_OUTPUT

