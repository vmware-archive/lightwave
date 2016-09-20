AC_PREREQ(2.59)

AC_INIT([ic-config], [1.0.0], [support@vmware.com])
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

# Likewise components

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

AC_SUBST(LW_BASE_PATH)
AC_SUBST(LW_INCLUDES)
AC_SUBST(LW_LDFLAGS)

DCERPC_PATH=$(LW_BASE_PATH)/bin
DCERPC_INCLUDES=$LW_INCLUDES
DCERPC_LDFLAGS=$LW_LDFLAGS

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
AC_CHECK_HEADERS(lwsm/lwsm.h)
AC_CHECK_HEADERS(reg/lwreg.h)
AC_CHECK_HEADERS(reg/regutil.h)
CPPFLAGS="$saved_CPPFLAGS"

# VMDIR component

AC_ARG_WITH([vmdir],
    [AC_HELP_STRING([--with-vmdir=<dir>], [use VMDIR binaries rooted at prefix <dir> ])],
    [
        VMDIR_BASE_PATH="$withval"
        VMDIR_INCLUDES="-I$withval/include"
        VMDIR_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64"
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

AC_SUBST(VMDIR_BASE_PATH)
AC_SUBST(VMDIR_INCLUDES)
AC_SUBST(VMDIR_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMDIR_INCLUDES $DCERPC_INCLUDES"
AC_CHECK_HEADER(vmdirclient.h,
[
    AC_DEFINE([HAVE_VMDIRCLIENT_H],
              [1],
              [Define to 1 if you have vmdirclient.h header file])
],
[
],
[
    #include <lw/base.h>
    #include <stdio.h> 
])
AC_CHECK_HEADERS(ldap.h lber.h)
CPPFLAGS="$saved_CPPFLAGS"

vmdircfgdir="$VMDIR_BASE_PATH/share/config"
AS_AC_EXPAND(VMDIR_CONFIG_PATH, $vmdircfgdir)
AC_DEFINE_UNQUOTED(VMDIR_CONFIG_PATH, "$vmdircfgdir", [VMware Directory Config Path])

# vmafd component

AC_ARG_WITH([afd],
    [AC_HELP_STRING([--with-afd=<dir>], [use afd-server binaries rooted at prefix <dir> ])],
    [
        VMAFD_BASE_PATH="$withval"
        VMAFD_INCLUDES="-I$withval/include"
        VMAFD_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64"
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

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMAFD_INCLUDES $LW_INCLUDES"
AC_CHECK_HEADERS(vmafdclient.h)
CPPFLAGS="$saved_CPPFLAGS"

# vmca component

AC_ARG_WITH([vmca],
    [AC_HELP_STRING([--with-vmca=<dir>], [use vmca binaries rooted at prefix <dir> ])],
    [
        VMCA_BASE_PATH="$withval"
        VMCA_INCLUDES="-I$withval/include"
        VMCA_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64"
    ])

AC_ARG_WITH([vmca-includes],
    [AC_HELP_STRING([--with-vmca-includes=<dir>], [use vmca headers located in prefix <dir> ])],
    [
        VMCA_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([vmca-libs],
    [AC_HELP_STRING([--with-vmca-libs=<dir>], [use vmca libraries located in prefix <dir> ])],
    [
        VMCA_LDFLAGS="-L$withval"
    ])

AC_SUBST(VMCA_INCLUDES)
AC_SUBST(VMCA_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMCA_INCLUDES $LW_INCLUDES"
AC_CHECK_HEADERS(vmca.h)
CPPFLAGS="$saved_CPPFLAGS"

# vmdns component

AC_ARG_WITH([vmdns],
    [AC_HELP_STRING([--with-vmdns=<dir>], [use vmdns binaries rooted at prefix <dir> ])],
    [
        VMDNS_BASE_PATH="$withval"
        VMDNS_INCLUDES="-I$withval/include"
        VMDNS_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64"
    ])

AC_ARG_WITH([vmdns-includes],
    [AC_HELP_STRING([--with-vmdns-includes=<dir>], [use vmdns headers located in prefix <dir> ])],
    [
        VMDNS_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([vmdns-libs],
    [AC_HELP_STRING([--with-vmdns-libs=<dir>], [use vmdns libraries located in prefix <dir> ])],
    [
        VMDNS_LDFLAGS="-L$withval"
    ])

AC_SUBST(VMDNS_INCLUDES)
AC_SUBST(VMDNS_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMDNS_INCLUDES $LW_INCLUDES"
AC_CHECK_HEADERS(vmdns.h)
CPPFLAGS="$saved_CPPFLAGS"

# vmsts component

AC_ARG_WITH([sts],
    [AC_HELP_STRING([--with-sts=<dir>], [use VMware STS Client binaries rooted at prefix <dir> ])],
    [
        VMSTS_HOME="$withval"
        VMSTS_CLASSPATH=$VMSTS_HOME/jars
    ])

AC_SUBST(VMSTS_HOME)
AC_SUBST(VMSTS_CLASSPATH)

AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h sys/stat.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h sys/un.h dirent.h termios.h term.h)
AC_CHECK_HEADERS(arpa/inet.h)

# openssl component

AC_ARG_WITH([ssl],
    [AC_HELP_STRING([--with-ssl=<dir>], [use SSL binaries rooted at prefix <dir> ])],
    [
        OPENSSL_BASE_PATH="$withval"
        OPENSSL_INCLUDES="-I$withval/include"
        OPENSSL_LDFLAGS="-L$withval/lib64"
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
    [VmDirConnectionOpen],
    [VMDIR_LIBS="-lvmdirclient"],
    [],
    [$VMDIR_LDFLAGS -lvmdirclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [vmafdclient],
    [VmAfdGetDomainNameA],
    [VMAFD_LIBS="-lvmafdclient"],
    [],
    [$VMAFD_LDFLAGS -lvmafdclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [vmcaclient],
    [VMCACreateSelfSignedCertificateA],
    [VMCA_LIBS="-lvmcaclient"],
    [],
    [$VMCA_LDFLAGS -lvmcaclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])
AC_CHECK_LIB(
    [vmdnsclient],
    [VmDnsOpenServerA],
    [VMDNS_LIBS="-lvmdnsclient"],
    [],
    [$VMDNS_LDFLAGS -lvmdnsclient $LW_LDFLAGS $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

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
AC_SUBST(LWADVAPI_LIBS)
AC_SUBST(LWSM_LIBS)
AC_SUBST(LDAP_LIBS)
AC_SUBST(VMDIR_LIBS)
AC_SUBST(VMAFD_LIBS)
AC_SUBST(VMCA_LIBS)
AC_SUBST(VMDNS_LIBS)

#AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

#if test x"$IDL" = x"no"; then
#    AC_MSG_ERROR([DCERPC IDL compiler not found])
#fi

pscsetupbindir=$prefix/bin
AC_SUBST(pscsetupbindir)

systemddir=/lib/systemd/system
AC_SUBST(systemddir)

scriptsconfdir="$datadir/config"
AC_SUBST(scriptsconfdir)

pscsetupjarsdir=$prefix/jars
AC_SUBST(pscsetupjarsdir)

AC_CONFIG_FILES([Makefile
                 jdepends/Makefile
                 cfgutils/Makefile
                 pscsetup/Makefile
                 tools/Makefile
                 tools/ic-promote/Makefile
                 tools/ic-join/Makefile
                 scripts/Makefile
                 scripts/configure-lightwave-server
                 scripts/configure-identity-server
                ])
AC_OUTPUT

