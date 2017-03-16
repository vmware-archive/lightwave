AC_PREREQ(2.59)

AC_INIT([vmca], [1.0.0], [support@vmware.com])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR

#AC_PROG_CC
AC_PROG_CXX(g++)
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

CXXFLAGS="$CXXFLAGS -frtti"
CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC "

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -Wall -Werror -fno-strict-aliasing"

AC_SUBST(AM_CXXFLAGS)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)

AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--enable-debug], [enable debugging (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AM_CFLAGS="$AM_CFLAGS -g -O0"
            AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -DVMCA_DEBUG"
        fi
    ])


AC_ARG_ENABLE([gcov],
    [AC_HELP_STRING([--enable-gcov], [enable code coverage (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AM_CFLAGS="$AM_CFLAGS -g -O0 -fprofile-arcs -ftestcoverage"
            AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -DVMCA_DEBUG"
        fi
    ])

ENABLE_PYTHON=false
AC_ARG_ENABLE([python],
    [AC_HELP_STRING([--enable-python], [enable python module (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            ENABLE_PYTHON=true
        fi
    ])
AM_CONDITIONAL(ENABLE_PYTHON, [$ENABLE_PYTHON])

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

# vmafd component

AC_ARG_WITH([afd],
    [AC_HELP_STRING([--with-afd=<dir>], [use afd-server binaries rooted at prefix <dir> ])],
    [
        VMAFD_BASE_PATH="$withval"
        VMAFD_INCLUDES="-I$withval/include"
        VMAFD_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,$withval/lib64"
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

AC_ARG_WITH([afd-rpath],
    [AC_HELP_STRING([--with-afd-rpath=<dir>], [use afd-server libraries located at <dir> at runtime])],
    [
        VMAFD_LDFLAGS="-L$VMAFD_BASE_PATH/lib64 -Wl,-rpath,$withval -Wl,-rpath-link,$VMAFD_BASE_PATH/lib64"
    ])

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $VMAFD_INCLUDES $DCERPC_INCLUDES"
AC_CHECK_HEADERS(vmafdclient.h)
CPPFLAGS="$saved_CPPFLAGS"

VMAFD_LIBS="-lvmafdclient"
AC_SUBST(VMAFD_BASE_PATH)
AC_SUBST(VMAFD_INCLUDES)
AC_SUBST(VMAFD_LDFLAGS)

# OIDC

OIDC_DEFAULT_PATH=/opt/vmware/lib64
SSOCOMMON_LDFLAGS=$OIDC_DEFAULT_PATH
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

OIDC_LDFLAGS=$OIDC_DEFAULT_PATH
AC_ARG_WITH([oidc],
    [AC_HELP_STRING([--with-oidc=<dir>], [use oidc binaries rooted at prefix <dir> ])],
    [
        OIDC_BASE_PATH="$withval"
        OIDC_INCLUDES="-I$withval/include"
        OIDC_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

OIDC_LIBS="-loidc"
AC_CHECK_HEADERS(oidc.h)
AC_CHECK_HEADERS(oidc_types.h)
AC_CHECK_HEADERS(common_types.h)
AC_SUBST(OIDC_BASE_PATH)
AC_SUBST(OIDC_INCLUDES)
AC_SUBST(OIDC_LDFLAGS)


# Trident

AC_ARG_WITH([trident],
    [AC_HELP_STRING([--with-trident=<dir>], [use trident-server binaries rooted at prefix <dir> ])],
    [
        TRIDENT_BASE_PATH="$withval"
        TRIDENT_INCLUDES="-I$withval/include/vmware-rest"
        TRIDENT_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
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
        JANSSON_LDFLAGS="-L$withval/lib64 -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"
    ])

JANSSON_LIBS="-ljansson"
AC_CHECK_HEADERS(jansson.h)
AC_SUBST(JANSSON_BASE_PATH)
AC_SUBST(JANSSON_INCLUDES)
AC_SUBST(JANSSON_LDFLAGS)



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
AC_CHECK_HEADERS(lw/base.h lw/types.h uuid/uuid.h)
AC_CHECK_HEADERS(openssl/crypto.h openssl/ssl.h openssl/err.h)
CPPFLAGS="$saved_CPPFLAGS"

# POPT components

AC_ARG_WITH([popt],
    [AC_HELP_STRING([--with-popt=<dir>], [use POPT binaries rooted at prefix <dir> ])],
    [
        POPT_BASE_PATH="$withval"
        POPT_INCLUDES="-I$withval/include"
        POPT_LDFLAGS="-L$withval/lib"
    ])

AC_ARG_WITH([popt-includes],
    [AC_HELP_STRING([--with-popt-includes=<dir>], [use POPT headers located in prefix <dir> ])],
    [
        POPT_INCLUDES="-I$withval"
    ])

AC_ARG_WITH([popt-libs],
    [AC_HELP_STRING([--with-popt-libs=<dir>], [use POPT libraries located in prefix <dir> ])],
    [
        POPT_LDFLAGS="-L$withval"
    ])

AC_SUBST(POPT_BASE_PATH)
AC_SUBST(POPT_INCLUDES)
AC_SUBST(POPT_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $POPT_INCLUDES"
AC_CHECK_HEADERS(popt.h)
CPPFLAGS="$saved_CPPFLAGS"

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
        BOOST_LIB_PATH="$withval/lib"
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
        BOOST_LIB_PATH="$withval"
        BOOST_LDFLAGS="-L$withval"
    ])

AC_SUBST(BOOST_BASE_PATH)
AC_SUBST(BOOST_INCLUDES)
AC_SUBST(BOOST_LIB_PATH)
AC_SUBST(BOOST_LDFLAGS)

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

AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h termios.h term.h)

AC_C_CONST
AC_TYPE_SIZE_T

AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strerror)

AC_CHECK_LIB([dl], [dlopen], [DL_LIBS="-ldl"])
AC_CHECK_LIB([pthread], [pthread_self], [PTHREAD_LIBS="-lpthread"])
AC_CHECK_LIB([popt], [poptGetArg], [POPT_LIBS="-lpopt"], [], [$POPT_LDFLAGS])

AC_LANG_PUSH([C++])
echo $BOOST_BASE_PATH
echo $BOOST_LDFLAGS

AC_CHECK_LIB(
        [boost_unit_test_framework-gcc41-mt-1_55],
        [main],
        [BOOST_LIBS="-lboost_system-gcc41-mt-s-1_55 -lboost_filesystem-gcc41-mt-s-1_55 -lboost_program_options-gcc41-mt-s-1_55"],
        [],
        [$BOOST_LDFLAGS])

AC_CHECK_LIB(
        [boost_unit_test_framework-xgcc42-mt-1_55],
        [main],
        [BOOST_LIBS="-lboost_system-xgcc42-mt-s-1_55 -lboost_filesystem-xgcc42-mt-s-1_55 -lboost_program_options-xgcc42-mt-s-1_55"],
        [],
        [$BOOST_LDFLAGS])

AC_CHECK_LIB(
        [boost_unit_test_framework],
        [main],
        [BOOST_LIBS="-lboost_thread -lboost_system -lboost_filesystem -lboost_program_options"],
        [],
        [$BOOST_LDFLAGS])
AC_LANG_POP([C++])

AC_CHECK_LIB([sqlite3], [sqlite3_open], [SQLITE_LIBS="-lsqlite3"], [], [$SQLITE_LDFLAGS $DL_LIBS $PTHREAD_LIBS])
AC_CHECK_LIB([python2.7 ],  [PyArg_Parse], [PYTHON_LIBS="-lpython2.7"], [], [$PYTHON_LDFLAGS])

AC_LANG_PUSH([C++])
saved_LIBS="$LIBS"
LOG4CPP_LIBS="-llog4cpp"
AC_LANG_PUSH([C++])
saved_LIBS="$LIBS"
LOG4CPP_LIBS="-static -llog4cpp"



AC_CHECK_LIB(
    [gssapi_krb5],
    [gss_accept_sec_context],
    [GSSAPI_LIBS="-lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err"],
    [],
    [$LW_LDFLAGS -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err])
AC_CHECK_LIB(
    [uuid],
    [uuid_generate],
    [UUID_LIBS="-luuid"],
    [],
    [$LW_LDFLAGS])
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
    [ssl],
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
    [$DCERPC_LDFLAGS $OPENSSL_LDFLAGS $GSSAPI_LIBS $SCHANNEL_LIBS $CRYPTO_LIBS $LWIO_LIBS
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS
    ])
AC_CHECK_LIB(
              [lber],
              [ber_scanf],
              [LBER_LIBS="-llber"],
              [],
              [$LW_LDFLAGS -llber])

#AC_CHECK_LIB(
#              [gssapi_krb5],
#              [gss_accept_sec_context],
#              [GSSAPI_LIBS="-lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err"],
#              [],
#              [$LW_LDFLAGS -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err])

AC_CHECK_LIB(
              [ldap_r],
              [ldap_add_ext],
              [LDAP_LIBS="-lldap_r -llber -lsasl2"],
              [],
              [$LW_LDFLAGS -llber -lsasl2 $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])



AC_SUBST(BOOST_LIBS)
AC_SUBST(DL_LIBS)
AC_SUBST(PTHREAD_LIBS)
AC_SUBST(POPT_LIBS)
AC_SUBST(SQLITE_LIBS)
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(UUID_LIBS)
AC_SUBST(LWBASE_LIBS)
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
AC_SUBST(PYTHON_LIBS)
AC_SUBST(VMAFD_LIBS)
AC_SUBST(TRIDENT_LIBS)
AC_SUBST(JANSSON_LIBS)
AC_SUBST(OIDC_LIBS)
AC_SUBST(SSOCOMMON_LIBS)


AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

AS_AC_EXPAND(VMCA_BIN_DIR, $prefix)
AC_SUBST(VMCA_BIN_DIR)

AS_AC_EXPAND(VMCA_SBIN_DIR, ["${sbindir}"])
AC_SUBST(VMCA_SBIN_DIR)

if test x"$localstatedir" = x"/var"; then
    vmcadbdir="$localstatedir/lib/vmca"
else
    vmcadbdir="$localstatedir"
fi
AC_SUBST(vmcadbdir)
AS_AC_EXPAND(VMCA_DB_DIR, $vmcadbdir)
AC_SUBST(VMCA_DB_DIR)
AC_DEFINE_UNQUOTED(VMCA_DB_DIR, "$VMCA_DB_DIR", [Database directory])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

AS_AC_EXPAND(VMCA_INSTALL_DIR, $prefix)
AC_DEFINE_UNQUOTED(VMCA_INSTALL_DIR, "$VMCA_INSTALL_DIR", [Install folder])

vmcaconfdir="$datadir/config"
AC_SUBST(vmcaconfdir)

AS_AC_EXPAND(VMCA_CONFIG_DIR, $vmcaconfdir)
AC_SUBST(VMCA_CONFIG_DIR)

AC_DEFINE_UNQUOTED(VMCA_CONFIG_DIR, "$VMCA_CONFIG_DIR", [Config directory])

vmcatoolsdir=$prefix/bin
AC_SUBST(vmcatoolsdir)

vmcapyutilsdir=$prefix/site-packages/cis
AC_SUBST(vmcapyutilsdir)

vmcajarsdir="$prefix/jars"
AC_SUBST(vmcajarsdir)

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 include/public/Makefile
                 config/Makefile
                 config/vmca.reg
                 common/Makefile
                 service/Makefile
                 client/Makefile
                 vmcadb/Makefile
                 certool/Makefile
                 test/Makefile
                 python/Makefile
                 jdepends/Makefile
                 java/Makefile
                ])
AC_OUTPUT

