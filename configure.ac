AC_PREREQ([2.69])
AC_INIT([lightwave], [1.3.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
LT_INIT
AC_PROG_CC
AC_PROG_CXX(g++)
AC_PROG_LIBTOOL
AC_PREFIX_DEFAULT(/opt/vmware)

AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

dnl Check supported operating systems
dnl

case "${host_os}:${host_cpu}" in
    linux*:x86_64)
        PLATFORM_LIB_PREFIX=lib64
        ;;
    linux*:aarch64)
        PLATFORM_LIB_PREFIX=lib
        ;;
    darwin*:x86_64)
        PLATFORM_LIB_PREFIX=lib
        ;;
    *)
        AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
        ;;
esac

CPPFLAGS="$CPPFLAGS -D_REENTRANT -D_GNU_SOURCE -fPIC"
CXXFLAGS="$CXXFLAGS -frtti"

AM_CPPFLAGS="$AM_CPPFLAGS -I${top_srcdir}/include"
AM_CFLAGS="$AM_CFLAGS -Wall -Werror -fno-strict-aliasing"

AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)

AC_HEADER_STDC
AC_CHECK_HEADERS(pthread.h errno.h sys/types.h sys/stat.h stdio.h string.h strings.h)
AC_CHECK_HEADERS(unistd.h time.h inttypes.h sys/socket.h netdb.h syslog.h)
AC_CHECK_HEADERS(stdlib.h locale.h stddef.h stdarg.h assert.h signal.h)
AC_CHECK_HEADERS(ctype.h netinet/in.h arpa/inet.h fcntl.h sys/un.h dirent.h termios.h term.h)
AC_CHECK_HEADERS(pwd.h grp.h nss.h nss_common.h nsswitch.h shadow.h crypt.h)
AC_CHECK_HEADERS(limits.h)

AC_C_CONST
AC_TYPE_SIZE_T

AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strerror)

#--------------------------------------------------------------------

AC_ARG_ENABLE([debug],
    [AC_HELP_STRING([--enable-debug],
                    [enable debugging (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AM_CFLAGS="$AM_CFLAGS -g -O0"
            AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -DLDAP_DEBUG -DVMCA_DEBUG"
        fi
    ])

#--------------------------------------------------------------------

ADDACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([addacl],
    [AC_HELP_STRING([--enable-addacl],
                    [enable acl check on add (default: enabled)])],
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

#--------------------------------------------------------------------

MODACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([modacl],
    [AC_HELP_STRING([--enable-modacl],
                    [enable acl check on modify (default: enabled)])],
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

#--------------------------------------------------------------------

DELACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([delacl],
    [AC_HELP_STRING([--enable-delacl],
                    [enable acl check on delete (default: enabled)])],
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

#--------------------------------------------------------------------

SEAACL_CHECK_ENABLED="yes"
AC_ARG_ENABLE([seaacl],
    [AC_HELP_STRING([--enable-seaacl],
                    [enable acl check on search (default: enabled)])],
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

#--------------------------------------------------------------------
# rest head flag

REST_ENABLED=true
AC_ARG_ENABLE([rest],
    [AC_HELP_STRING([--enable-rest],
                    [enable rest heads (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            REST_ENABLED=false
        fi
    ])

if $REST_ENABLED
then
AC_DEFINE_UNQUOTED(REST_ENABLED, 1, [ Enable rest heads ])
fi

AM_CONDITIONAL(REST_ENABLED, [$REST_ENABLED])

#--------------------------------------------------------------------
# datastore flag

AC_ARG_WITH([datastore],
    [AC_HELP_STRING([--with-datastore=<datastore>],
                    [use backend datastore <datastore>])],
    [
        DATASTORE="$withval"
    ],
    [
        DATASTORE="mdb"
    ])

#--------------------------------------------------------------------
# lightwave build flag

ENABLE_LIGHTWAVE_BUILD=true
AC_ARG_ENABLE([lightwave-build],
    [AC_HELP_STRING([--enable-lightwave-build],
                    [enable lightwave build (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            ENABLE_LIGHTWAVE_BUILD=false
        fi
    ])

if $ENABLE_LIGHTWAVE_BUILD
then
AC_DEFINE_UNQUOTED(LIGHTWAVE_BUILD, 1, [ enable lightwave specific build ])
fi

AM_CONDITIONAL(LIGHTWAVE_BUILD, [$LIGHTWAVE_BUILD])

#--------------------------------------------------------------------
# client only build

ENABLE_SERVER=true
AC_ARG_ENABLE([server],
    [AC_HELP_STRING([--enable-server],
                    [enable full build (default: yes)])],
    [
        if test x"$enableval" = x"yes"
        then
            ENABLE_SERVER=true
        else
            ENABLE_SERVER=false
        fi
    ])

AM_CONDITIONAL(ENABLE_SERVER, [$ENABLE_SERVER])

#--------------------------------------------------------------------
# go dependency flag

GO_ENABLED=false
AC_ARG_ENABLE([go],
    [AC_HELP_STRING([--enable-go],
                    [enable go dependencies (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            GO_ENABLED=true
        fi
    ])

if $GO_ENABLED
then
AC_DEFINE_UNQUOTED(GO_ENABLED, 1, [ Enable go dependencies ])
fi

AM_CONDITIONAL(GO_ENABLED, [$GO_ENABLED])

#--------------------------------------------------------------------
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
    [AC_HELP_STRING([--with-ssl=<dir>],
                    [use SSL binaries rooted at prefix <dir>])],
    [
        OPENSSL_BASE_PATH="$withval"
    ],
    [
        OPENSSL_BASE_PATH="/usr"
    ])

AC_ARG_WITH([ssl-includes],
    [AC_HELP_STRING([--with-ssl-includes=<dir>],
                    [use SSL headers located in prefix <dir>])],
    [
        OPENSSL_INCLUDES="-DOPENSSL_NO_KRB5 -I$withval"
    ],
    [
        OPENSSL_INCLUDES="-DOPENSSL_NO_KRB5 -I$OPENSSL_BASE_PATH/include"
    ])

AC_ARG_WITH([ssl-libs],
    [AC_HELP_STRING([--with-ssl-libs=<dir>],
                    [use SSL libraries located in prefix <dir>])],
    [
        OPENSSL_LDFLAGS="-L$withval"
    ],
    [
        OPENSSL_LDFLAGS="-L$OPENSSL_BASE_PATH/$PLATFORM_LIB_PREFIX -Wl,-rpath,$OPEN_SSL_DEFAULT_PATH"
    ])

AC_SUBST(OPENSSL_BASE_PATH)
AC_SUBST(OPENSSL_INCLUDES)
AC_SUBST(OPENSSL_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $LW_INCLUDES"
AC_CHECK_HEADERS(openssl/crypto.h openssl/ssl.h openssl/err.h)
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------
# zlib component

AC_ARG_WITH([zlib],
    [AC_HELP_STRING([--with-zlib=<dir>],
                    [use zlib binaries rooted at prefix <dir>])],
    [
        ZLIB_BASE_PATH="$withval"
    ],
    [
        ZLIB_BASE_PATH="/usr"
    ])

ZLIB_INCLUDES="-I$ZLIB_BASE_PATH/include"
ZLIB_LDFLAGS="-L$ZLIB_BASE_PATH/lib -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"

AC_SUBST(ZLIB_BASE_PATH)
AC_SUBST(ZLIB_INCLUDES)
AC_SUBST(ZLIB_LDFLAGS)

#--------------------------------------------------------------------
# bzip component

AC_ARG_WITH([bzip],
    [AC_HELP_STRING([--with-bzip=<dir>],
                    [use BZIP binaries rooted at prefix <dir>])],
    [
        BZIP_BASE_PATH="$withval"
    ],
    [
        BZIP_BASE_PATH="/usr"
    ])

BZIP_INCLUDES="-I$BZIP_BASE_PATH/include"
BZIP_LDFLAGS="-L$BZIP_BASE_PATH/lib -Wl,-rpath,/opt/vmware/lib64 -Wl,-rpath-link,/opt/vmware/lib64"

AC_SUBST(BZIP_BASE_PATH)
AC_SUBST(BZIP_INCLUDES)
AC_SUBST(BZIP_LDFLAGS)

#--------------------------------------------------------------------
# Cyrus SASL

AC_ARG_WITH([sasl],
    [AC_HELP_STRING([--with-sasl=<dir>],
                    [use SASL binaries rooted at prefix <dir>])],
    [
        SASL_BASE_PATH="$withval"
    ],
    [
        SASL_BASE_PATH="/usr"
    ])

AC_ARG_WITH([sasl-includes],
    [AC_HELP_STRING([--with-sasl-includes=<dir>],
                    [use SASL headers located in prefix <dir>])],
    [
        SASL_INCLUDES="-I$withval"
    ],
    [
        SASL_INCLUDES="-I$SASL_BASE_PATH/include"
    ])

AC_ARG_WITH([sasl-libs],
    [AC_HELP_STRING([--with-sasl-libs=<dir>],
                    [use SASL libraries located in prefix <dir>])],
    [
        SASL_LIBPATH="$withval"
        SASL_LDFLAGS="-L$withval"
    ],
    [
        SASL_LIBPATH="$SASL_BASE_PATH/lib64"
        SASL_LDFLAGS="-L$SASL_BASE_PATH/lib64"
    ])

AC_ARG_WITH([sasl-plugins],
    [AC_HELP_STRING([--with-sasl-plugins=<dir>],
                    [assume SASL plugins at runtime are located in <dir>])],
    [
        SASL_PLUGINSPATH="$withval"
    ],
    [
        SASL_PLUGINSPATH="$SASL_BASE_PATH/lib64/sasl2"
    ])

AC_SUBST(SASL_BASE_PATH)
AC_SUBST(SASL_INCLUDES)
AC_SUBST(SASL_LDFLAGS)

AC_DEFINE_UNQUOTED(VMDIR_CONFIG_SASL2_LIB_PATH, "$SASL_PLUGINSPATH", [SASL2 Library path])

#--------------------------------------------------------------------
# Jansson

AC_ARG_WITH([jansson],
    [AC_HELP_STRING([--with-jansson=<dir>],
                    [use jansson binaries rooted at prefix <dir>])],
    [
        JANSSON_BASE_PATH="$withval"
    ],
    [
        JANSSON_BASE_PATH="/usr"
    ])

JANSSON_INCLUDES="-I$JANSSON_BASE_PATH/include"
JANSSON_LDFLAGS="-L$JANSSON_BASE_PATH/lib"

AC_CHECK_HEADERS(jansson.h)
AC_SUBST(JANSSON_BASE_PATH)
AC_SUBST(JANSSON_INCLUDES)
AC_SUBST(JANSSON_LDFLAGS)

#--------------------------------------------------------------------
# Copenapi

AC_ARG_WITH([copenapi],
    [AC_HELP_STRING([--with-copenapi=<dir>],
                    [use copenapi binaries rooted at prefix <dir>])],
    [
        COPENAPI_BASE_PATH="$withval"
    ],
    [
        COPENAPI_BASE_PATH="/usr"
    ])

COPENAPI_INCLUDES="-I$COPENAPI_BASE_PATH/include"
COPENAPI_LDFLAGS="-L$COPENAPI_BASE_PATH/lib"

AC_CHECK_HEADERS(copenapi/copenapi.h)
AC_SUBST(COPENAPI_BASE_PATH)
AC_SUBST(COPENAPI_INCLUDES)
AC_SUBST(COPENAPI_LDFLAGS)

#--------------------------------------------------------------------
# C rest engine

AC_ARG_WITH([c-rest-engine],
    [AC_HELP_STRING([--with-c-rest-engine=<dir>],
                    [use c-rest-engine server binaries rooted at prefix <dir>])],
    [
        CRESTENGINE_BASE_PATH="$withval"
    ],
    [
        CRESTENGINE_BASE_PATH="/usr"
    ])

CRESTENGINE_INCLUDES="-I$CRESTENGINE_BASE_PATH/include"
CRESTENGINE_LDFLAGS="-L$CRESTENGINE_BASE_PATH/lib"

AC_CHECK_HEADERS(vmrest.h)
AC_SUBST(CRESTENGINE_BASE_PATH)
AC_SUBST(CRESTENGINE_INCLUDES)
AC_SUBST(CRESTENGINE_LDFLAGS)

#--------------------------------------------------------------------
# Likewise components

AC_ARG_WITH([likewise],
    [AC_HELP_STRING([--with-likewise=<dir>],
                    [use likewise binaries rooted at prefix <dir>])],
    [
        LW_BASE_PATH="$withval"
    ],
    [
        LW_BASE_PATH="/opt/likewise"
    ])

case "${host_os}:${host_cpu}" in
    linux*:x86_64)
    LIKEWISE_DEFAULT_PATH=$LW_BASE_PATH/lib64
    ;;
    darwin*:x86_64)
    LIKEWISE_DEFAULT_PATH=$LW_BASE_PATH/lib
    ;;
    linux*:aarch64)
    LIKEWISE_DEFAULT_PATH=$LW_BASE_PATH/lib
    ;;
    *)
    AC_ERROR("Unsupported operating system - ${host_os}:${host_cpu}")
    ;;
esac

AC_ARG_WITH([likewise-rpath],
    [AC_HELP_STRING([--with-likewise-rpath=<dir>],
                    [use likewise libraries located at <dir> at runtime])],
    [
        LW_RPATH="$withval"
    ],
    [
        LW_RPATH=$LIKEWISE_DEFAULT_PATH
    ])

AC_ARG_WITH([likewise-includes],
    [AC_HELP_STRING([--with-likewise-includes=<dir>],
                    [use likewise headers located in prefix <dir>])],
    [
        LW_INCLUDES="-I$withval"
    ],
    [
        LW_INCLUDES="-I$LW_BASE_PATH/include"
    ])

AC_ARG_WITH([likewise-libs],
    [AC_HELP_STRING([--with-likewise-libs=<dir>],
                    [use likewise libraries located in prefix <dir>])],
    [
        LW_LDFLAGS="-L$withval -Wl,-rpath,$LW_RPATH -Wl,-rpath-link,$withval"
    ],
    [
        LW_LDFLAGS="-L$LW_BASE_PATH/$PLATFORM_LIB_PREFIX -Wl,-rpath,$LW_RPATH -Wl,-rpath-link,$LW_BASE_PATH/$PLATFORM_LIB_PREFIX"
    ])

AC_SUBST(LW_BASE_PATH)
AC_SUBST(LW_INCLUDES)
AC_SUBST(LW_LDFLAGS)

#--------------------------------------------------------------------

DCERPC_PATH=$LW_BASE_PATH/bin
DCERPC_INCLUDES=$LW_INCLUDES
DCERPC_LDFLAGS=$LW_LDFLAGS

AC_ARG_WITH([dcerpc],
    [AC_HELP_STRING([--with-dcerpc=<dir>],
                    [use DCERPC binaries rooted at prefix <dir>])],
    [
        DCERPC_BASE_PATH="$withval/bin"
    ],
    [
        DCERPC_BASE_PATH="$LW_BASE_PATH"
    ])

DCERPC_PATH="$DCERPC_BASE_PATH/bin"

AC_ARG_WITH([dcerpc-includes],
    [AC_HELP_STRING([--with-dcerpc-includes=<dir>],
                    [use DCERPC headers located in prefix <dir>])],
    [
        DCERPC_INCLUDES="-I$withval"
    ],
    [
        DCERPC_INCLUDES="-I$DCERPC_BASE_PATH/include"
    ])

AC_ARG_WITH([dcerpc-libs],
    [AC_HELP_STRING([--with-dcerpc-libs=<dir>],
                    [use DCERPC libraries located in prefix <dir>])],
    [
        DCERPC_LDFLAGS="-L$withval"
    ],
    [
        DCERPC_LDFLAGS="-L$DCERPC_BASE_PATH/lib64"
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
AC_CHECK_HEADERS(lwsm/lwsm.h)
AC_CHECK_HEADERS(reg/lwreg.h)
AC_CHECK_HEADERS(reg/regutil.h)
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------

AC_ARG_ENABLE([pac],
     AS_HELP_STRING([--disable-pac], [Disable Kerberos PAC]))
AM_CONDITIONAL([VMDIR_ENABLE_PAC], [test "x$enable_pac" != "xno"]])
if test x"$enable_pac" != x"no"
then
AC_DEFINE_UNQUOTED(VMDIR_ENABLE_PAC, 1, [ Enable PAC ])
fi

#--------------------------------------------------------------------
# POPT components

AC_ARG_WITH([popt],
    [AC_HELP_STRING([--with-popt=<dir>],
                    [use POPT binaries rooted at prefix <dir>])],
    [
        POPT_BASE_PATH="$withval"
    ],
    [
        POPT_BASE_PATH="/usr"
    ])

POPT_INCLUDES="-I$POPT_BASE_PATH/include"
POPT_LDFLAGS="-L$POPT_BASE_PATH/lib"

AC_SUBST(POPT_BASE_PATH)
AC_SUBST(POPT_INCLUDES)
AC_SUBST(POPT_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $POPT_INCLUDES"
AC_CHECK_HEADERS(popt.h)
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------
# SQLITE components

AC_ARG_WITH([sqlite],
    [AC_HELP_STRING([--with-sqlite=<dir>],
                    [use SQLITE binaries rooted at prefix <dir>])],
    [
        SQLITE_BASE_PATH="$withval"
    ],
    [
        SQLITE_BASE_PATH="/usr"
    ])

SQLITE_INCLUDES="-I$SQLITE_BASE_PATH/include"
SQLITE_LDFLAGS="-L$SQLITE_BASE_PATH/lib"

AC_SUBST(SQLITE_BASE_PATH)
AC_SUBST(SQLITE_INCLUDES)
AC_SUBST(SQLITE_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $SQLITE_INCLUDES"
AC_CHECK_HEADERS(sqlite3.h)
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------
# BOOST components

AC_ARG_WITH([boost],
    [AC_HELP_STRING([--with-boost=<dir>],
                    [use BOOST binaries rooted at prefix <dir>])],
    [
        BOOST_BASE_PATH="$withval"
    ],
    [
        BOOST_BASE_PATH="/usr"
    ])

AC_ARG_WITH([boost-includes],
    [AC_HELP_STRING([--with-boost-includes=<dir>],
                    [use BOOST headers located in prefix <dir>])],
    [
        BOOST_INCLUDES="-I$withval"
    ],
    [
        BOOST_INCLUDES="-I$BOOST_BASE_PATH/include"
    ])

AC_ARG_WITH([boost-libs],
    [AC_HELP_STRING([--with-boost-libs=<dir>],
                    [use BOOST libraries located in prefix <dir>])],
    [
        BOOST_LDFLAGS="-L$withval"
    ],
    [
        BOOST_LDFLAGS="-L$BOOST_BASE_PATH/lib"
    ])

AC_ARG_WITH([boost-python-lib],
    [AC_HELP_STRING([--with-boost-python-lib=<dir>],
                    [use BOOST Python library extension])],
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

#--------------------------------------------------------------------
# Python components

AC_ARG_WITH([python],
    [AC_HELP_STRING([--with-python=<dir>],
                   [use Python binaries rooted at prefix <dir>])],
    [
        PYTHON_BASE_PATH="$withval"
    ],
    [
        PYTHON_BASE_PATH="/usr"
    ])

AC_ARG_WITH([python-includes],
    [AC_HELP_STRING([--with-python-includes=<dir>],
                    [use Python headers located in prefix <dir>])],
    [
        PYTHON_INCLUDES="-I$withval"
    ],
    [
        PYTHON_INCLUDES="-I$PYTHON_BASE_PATH/include/python2.7"
    ])

AC_ARG_WITH([python-libs],
    [AC_HELP_STRING([--with-python-libs=<dir>],
                    [use Python libraries located in prefix <dir>])],
    [
        PYTHON_LDFLAGS="-L$withval"
    ],
    [
        PYTHON_LDFLAGS="-L$PYTHON_BASE_PATH/lib"
    ])

AC_SUBST(PYTHON_BASE_PATH)
AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(PYTHON_LDFLAGS)

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_CHECK_HEADERS(python.h)
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------
# Maven component

AC_ARG_WITH([maven],
    [AC_HELP_STRING([--with-maven=<dir>],
                    [use Apache Maven  binaries rooted at prefix <dir>])],
    [
        MAVEN_HOME="$withval"
    ],
    [
        MAVEN_HOME="/var/opt/apache-maven"
    ])

AC_SUBST(MAVEN_HOME)

#--------------------------------------------------------------------
# Java

AC_ARG_WITH([java],
    [AC_HELP_STRING([--with-java=<dir>],
                    [use java binaries rooted at prefix <dir>])],
    [
        JAVA_HOME="$withval"
    ],
    [
        JAVA_HOME="/etc/alternatives/jre/../"
    ])

AC_ARG_WITH([java-includes],
    [AC_HELP_STRING([--with-java-includes=<dir>],
                    [use java headers located in prefix <dir>])],
    [
        JDK_INCLUDES="-I$withval"
    ],
    [
        JDK_INCLUDES="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"
    ])

AC_ARG_WITH([java-libs],
    [AC_HELP_STRING([--with-java-libs=<dir>],
                    [use java libraries located in prefix <dir>])],
    [
        JDK_LDFLAGS="-L$withval"
    ],
    [
        JDK_LDFLAGS="-L$JAVA_HOME/lib"
    ])

TOOLS_CLASSPATH=$JAVA_HOME/lib/tools.jar

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

#--------------------------------------------------------------------

AC_CHECK_LIB([dl],
    [dlopen],
    [DL_LIBS="-ldl"])

AC_CHECK_LIB([pthread],
    [pthread_self],
    [PTHREAD_LIBS="-lpthread"])

AC_CHECK_LIB([z],
    [inflate],
    [ZLIB_LIBS="-lz"],
    [],
    [$ZLIB_LDFLAGS])

AC_CHECK_LIB([bz2],
    [BZ2_bzdopen],
    [BZIP_LIBS="-lbz2"],
    [],
    [$BZIP_LDFLAGS])

AC_CHECK_LIB([uuid],
    [uuid_copy],
    [UUID_LIBS="-luuid"],
    [],
    [$LW_LDFLAGS -luuid])

AC_CHECK_LIB([crypto],
    [MD5_Init],
    [CRYPTO_LIBS="-lcrypto -lssl"],
    [],
    [$OPENSSL_LDFLAGS])

AC_CHECK_LIB([lber],
    [ber_scanf],
    [LBER_LIBS="-llber"],
    [],
    [$LW_LDFLAGS -llber])

AC_CHECK_LIB([gssapi_krb5],
    [gss_accept_sec_context],
    [GSSAPI_LIBS="-lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err"],
    [],
    [$LW_LDFLAGS -lgssapi_krb5 -lkrb5 -lk5crypto -lkrb5support -lcom_err])

AC_CHECK_LIB([sasl2],
    [sasl_server_init],
    [SASL_LIBS="-lsasl2"],
    [],
    [$SASL_LDFLAGS])

AC_CHECK_LIB([ldap_r],
    [ldap_initialize],
    [LDAP_LIBS="-lldap_r -llber"],
    [],
    [$LW_LDFLAGS -llber $SASL_LDFLAGS $SASL_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

AC_CHECK_LIB([lwbase],
    [LwRtlMemoryAllocate],
    [LWBASE_LIBS="-llwbase -llwbase_nothr"],
    [],
    [$LW_LDFLAGS -llwbase_nothr])

AC_CHECK_LIB([lwadvapi],
    [LwFreeMemory],
    [LWADVAPI_LIBS="-llwadvapi -llwadvapi_nothr"],
    [],
    [$LW_LDFLAGS -llwadvapi_nothr $LWBASE_LIBS $LDAP_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

AC_CHECK_LIB([lwmsg],
    [lwmsg_connection_new],
    [LWMSG_LIBS="-llwmsg -llwmsg_nothr"],
    [],
    [$LW_LDFLAGS -llwmsg_nothr $LWBASE_LIBS])

AC_CHECK_LIB([regclient],
    [LwRegOpenKeyExW],
    [LWREG_LIBS="-lregclient -lregcommon"],
    [],
    [$LW_LDFLAGS -lregcommon $LWMSG_LIBS $LWBASE_LIBS])

AC_CHECK_LIB([rsutils],
    [RegUtilSetValue],
    [LWRSUTILS_LIBS="-lrsutils"],
    [],
    [$LW_LDFLAGS -lrsutils $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS])

AC_CHECK_LIB([schannel],
    [schn_init_creds],
    [SCHANNEL_LIBS="-lschannel"],
    [],
    [$LW_LDFLAGS $LWBASE_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS])

AC_CHECK_LIB([lwioclient],
    [LwNtCreateFile],
    [LWIO_LIBS="-llwioclient -llwioshareinfo -llwiocommon"],
    [],
    [$LW_LDFLAGS -llwioshareinfo -llwiocommon
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS $GSSAPI_LIBS])

AC_CHECK_LIB([dcerpc],
    [rpc__init],
    [DCERPC_LIBS="-ldcerpc"],
    [],
    [$DCERPC_LDFLAGS $OPENSSL_LDFLAGS $GSSAPI_LIBS $SCHANNEL_LIBS $CRYPTO_LIBS $LWIO_LIBS
     $LWREG_LIBS $LWMSG_LIBS $LWBASE_LIBS])

AC_CHECK_LIB([jansson],
    [json_object],
    [JANSSON_LIBS="-ljansson"],
    [],
    [$JANSSON_LDFLAGS])

AC_CHECK_LIB([curl],
    [main],
    [CURL_LIBS="-lcurl"],
    [],
    [$CURL_LDFLAGS])

AC_CHECK_LIB([copenapi],
    [coapi_load_from_file],
    [COPENAPI_LIBS="-lcopenapi"]
    [],
    [$COPENAPI_LDFLAGS])

AC_CHECK_LIB([restengine],
    [VmRESTInit],
    [CRESTENGINE_LIBS="-lrestengine"],
    [],
    [$CRESTENGINE_LDFLAGS])

AC_CHECK_LIB([shadow],
    [getspnam],
    [SHADOW_LIBS="-lshadow"])

AC_CHECK_LIB([popt],
    [poptGetArg],
    [POPT_LIBS="-lpopt"],
    [],
    [$POPT_LDFLAGS])

#
# Use -lxcrypt on SLES11, which contains advanced hash algorithms
#
AC_CHECK_LIB([xcrypt],
    [crypt_r],
    [CRYPT_LIBS="-L/usr/lib64 -lxcrypt $DL_LIBS"],
    [CRYPT_LIBS="no"],
    [-L/usr/lib64 $DL_LIBS])
if test x"$CRYPT_LIBS" = x"no"; then
    AC_CHECK_LIB([crypt],
        [crypt_r],
        [CRYPT_LIBS="-lcrypt"],
        [CRYPT_LIBS=""])
fi

AC_CHECK_LIB([lwsm],
    [LwSmStartService],
    [LWSM_LIBS="-llwsm -llwsmcommon"],
    [],
    [$LW_LDFLAGS $LWMSG_LIBS $LWADVAPI_LIBS $LWBASE_LIBS $GSSAPI_LIBS
     $UUID_LIBS $LDAP_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS -llwsmcommon])

AC_CHECK_LIB([domainjoin],
    [DJJoinDomain],
    [DOMAINJOIN_LIBS="-ldomainjoin -lcentutils -leventlog -leventlogutils -llsaclient -llsacommon -llwnetclientapi -llwnetcommon -llwsm -llwsmcommon"],
    [],
    [$LW_LDFLAGS -ldomainjoin -lcentutils -leventlog -leventlogutils -llsaclient -llsacommon -llwnetclientapi -llwnetcommon -llwsm -llwsmcommon $DCERPC_LIBS $SCHANNEL_LIBS $LWMSG_LIBS $LWRSUTILS_LIBS $LWREG_LIBS $LWIO_LIBS $LWBASE_LIBS $LDAP_LIBS $PTHREAD_LIBS $UUID_LIBS $LWBASE_LIBS $GSSAPI_LIBS $OPENSSL_LDFLAGS $CRYPTO_LIBS -llwadvapi_nothr])

AC_LANG_PUSH([C++])

AC_CHECK_LIB([boost_unit_test_framework-gcc41-mt-1_55],
    [main],
    [BOOST_LIBS="-lboost_system-gcc41-mt-s-1_55 -lboost_filesystem-gcc41-mt-s-1_55 -lboost_program_options-gcc41-mt-s-1_55"],
    [],
    [$BOOST_LDFLAGS])

AC_CHECK_LIB([boost_unit_test_framework-xgcc42-mt-1_55],
    [main],
    [BOOST_LIBS="-lboost_system-xgcc42-mt-s-1_55 -lboost_filesystem-xgcc42-mt-s-1_55 -lboost_program_options-xgcc42-mt-s-1_55"],
    [],
    [$BOOST_LDFLAGS])

AC_CHECK_LIB([boost_unit_test_framework],
    [main],
    [BOOST_LIBS="-lboost_thread -lboost_system -lboost_filesystem -lboost_program_options"],
    [],
    [$BOOST_LDFLAGS])

AC_LANG_POP([C++])

AC_CHECK_LIB([sqlite3],
    [sqlite3_open],
    [SQLITE_LIBS="-lsqlite3"],
    [],
    [$SQLITE_LDFLAGS $DL_LIBS $PTHREAD_LIBS])

AC_CHECK_LIB([python2.7],
    [PyArg_Parse],
    [PYTHON_LIBS="-lpython2.7"],
    [],
    [$PYTHON_LDFLAGS])

#--------------------------------------------------------------------

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
AC_SUBST(JANSSON_LIBS)
AC_SUBST(COPENAPI_LIBS)
AC_SUBST(CRESTENGINE_LIBS)
AC_SUBST(DOMAINJOIN_LIBS)
AC_SUBST(BOOST_LIBS)
AC_SUBST(PYTHON_LIBS)
AC_SUBST(SQLITE_LIBS)
AC_SUBST(LWSM_LIBS)
AC_SUBST(POPT_LIBS)
AC_SUBST(DOMAINJOIN_LIBS)
AC_SUBST(LWSM_LIBS)
AC_SUBST(CARES_LIBS)
AC_SUBST(CURL_LIBS)

#--------------------------------------------------------------------

AC_PATH_PROG([IDL], [dceidl], [no], [$PATH:$DCERPC_PATH])

if test x"$IDL" = x"no"; then
    AC_MSG_ERROR([DCERPC IDL compiler not found])
fi

#--------------------------------------------------------------------

if test x"$localstatedir" = x"/var"; then
    vmdirdbdir="$localstatedir/lib/vmware/vmdir"
else
    vmdirdbdir="$localstatedir/vmdir"
fi
AC_SUBST(vmdirdbdir)
AS_AC_EXPAND(VMDIR_DB_DIR, ["${vmdirdbdir}"])
AC_DEFINE_UNQUOTED(VMDIR_DB_DIR, "$VMDIR_DB_DIR", [Database directory])

#--------------------------------------------------------------------

if test x"$localstatedir" = x"/var"; then
    lwtmpdir="$localstatedir/lib/vmware/lightwave_tmp"
else
    lwtmpdir="$localstatedir/lightwave_tmp"
fi
AC_SUBST(lwtmpdir)
AS_AC_EXPAND(LIGHTWAVE_TMP_DIR, ["${lwtmpdir}"])
AC_DEFINE_UNQUOTED(LIGHTWAVE_TMP_DIR, "$LIGHTWAVE_TMP_DIR", [Temporary directory])

#--------------------------------------------------------------------

AS_AC_EXPAND(VMDIR_SBIN, ["${sbindir}"])
VMDIR_SBIN_DIR=$VMDIR_SBIN
AC_SUBST(VMDIR_SBIN_DIR)

vmdirlibdir=$prefix/lib64
AC_SUBST(vmdirlibdir)
AS_AC_EXPAND(VMDIR_LIB_DIR, $vmdirlibdir)
AC_DEFINE_UNQUOTED(VMDIR_LIB_DIR, "$VMDIR_LIB_DIR", [Lib directory])

vmdirsasldir=$vmdirlibdir/sasl2
AC_SUBST(vmdirsasldir)

vmcalibdir=$prefix/lib64
AC_SUBST(vmcalibdir)
AS_AC_EXPAND(VMCA_LIB_DIR, $vmcalibdir)
AC_DEFINE_UNQUOTED(VMCA_LIB_DIR, "$VMCA_LIB_DIR", [Lib directory])

#--------------------------------------------------------------------

AC_ARG_WITH([logdir],
   [AC_HELP_STRING([--with-logdir], [set the logging directory])],
   [
       LOG_BASE_PATH="$withval"
   ],
   [
       LOG_BASE_PATH="/var/log/lightwave"
   ])

vmdirlogdir=$LOG_BASE_PATH
AC_SUBST(vmdirlogdir)
AS_AC_EXPAND(VMDIR_LOG_DIR, $vmdirlogdir)
AC_DEFINE_UNQUOTED(VMDIR_LOG_DIR, "$VMDIR_LOG_DIR", [Log directory])

lwraftlogdir=$LOG_BASE_PATH
AC_SUBST(lwraftlogdir)
AS_AC_EXPAND(LWRAFT_LOG_DIR, $lwraftlogdir)
AC_DEFINE_UNQUOTED(LWRAFT_LOG_DIR, "$LWRAFT_LOG_DIR", [Log directory])

vmcalogdir=$LOG_BASE_PATH
AC_SUBST(vmcalogdir)
AS_AC_EXPAND(VMCA_LOG_DIR, $vmcalogdir)
AC_DEFINE_UNQUOTED(VMCA_LOG_DIR, "$VMCA_LOG_DIR", [Log directory])
#--------------------------------------------------------------------

AC_ARG_WITH([psc-version],
    [AC_HELP_STRING([--with-psc-version], [set the PSC version (default: 1.0)])],
    [
        psc_version="$withval"
    ],
    [
        psc_version="1.0"
    ])
AC_SUBST(psc_version)
AS_AC_EXPAND(VDIR_PSC_VERSION, $psc_version)
AC_DEFINE_UNQUOTED(VDIR_PSC_VERSION, "$VDIR_PSC_VERSION", [PSC version])

#--------------------------------------------------------------------

vmdirconfdir="$datadir/config"
AC_SUBST(vmdirconfdir)
AS_AC_EXPAND(VMDIR_CONFIG_DIR, $vmdirconfdir)
AC_DEFINE_UNQUOTED(VMDIR_CONFIG_DIR, "$VMDIR_CONFIG_DIR", [Config directory])

AS_AC_EXPAND(VMDIR_PREFIX, ["${prefix}"])
VMDIR_PREFIX_DIR=$VMDIR_PREFIX
AC_SUBST(VMDIR_PREFIX_DIR)

if test x"$localstatedir" = x"/var"; then
    lwraftdbdir="$localstatedir/lib/vmware/post"
else
    lwraftdbdir="$localstatedir/post"
fi
AC_SUBST(lwraftdbdir)
AS_AC_EXPAND(LWRAFT_DB_DIR, $lwraftdbdir)
AC_DEFINE_UNQUOTED(LWRAFT_DB_DIR, "$LWRAFT_DB_DIR", [Database directory])

AS_AC_EXPAND(LWRAFT_SBIN, ["${sbindir}"])
LWRAFT_SBIN_DIR=$LWRAFT_SBIN
AC_SUBST(LWRAFT_SBIN_DIR)

lwraftlibdir=$prefix/lib64
AC_SUBST(lwraftlibdir)
AS_AC_EXPAND(LWRAFT_LIB_DIR, $lwraftlibdir)
AC_DEFINE_UNQUOTED(LWRAFT_LIB_DIR, "$LWRAFT_LIB_DIR", [Lib directory])

postsasldir=$lwraftlibdir/sasl2
AC_SUBST(postsasldir)

lwraftconfdir="$datadir/config"
AC_SUBST(lwraftconfdir)
AS_AC_EXPAND(LWRAFT_CONFIG_DIR, $lwraftconfdir)
AC_DEFINE_UNQUOTED(LWRAFT_CONFIG_DIR, "$LWRAFT_CONFIG_DIR", [Config directory])

AS_AC_EXPAND(LWRAFT_PREFIX, ["${prefix}"])
LWRAFT_PREFIX_DIR=$LWRAFT_PREFIX
AC_SUBST(LWRAFT_PREFIX_DIR)

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

#--------------------------------------------------------------------
# vmdir-provider notify only build

VMDIR_PROVIDER_ENABLED=true
AC_ARG_ENABLE([notify-vmdir-provider],
    [AC_HELP_STRING([--enable-notify-vmdir-provider],
                    [enable vmdir provider notification (default: enabled)])],
    [
        if test x"$enableval" = x"no"
        then
            VMDIR_PROVIDER_ENABLED=false
        fi
    ])

if $VMDIR_PROVIDER_ENABLED
then
AC_DEFINE_UNQUOTED(NOTIFY_VMDIR_PROVIDER, 1, [Notify VMDir Provider of Join/Leave])
fi

AM_CONDITIONAL(NOTIFY_VMDIR_PROVIDER, [$NOTIFY_VMDIR_PROVIDER])

#--------------------------------------------------------------------
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

#--------------------------------------------------------------------
# Ant component

AC_ARG_WITH([ant],
    [AC_HELP_STRING([--with-ant=<dir>], [use Ant binaries rooted at prefix <dir> ])],
    [
        ANT_HOME="$withval"
    ],
    [
        ANT_HOME="/var/opt/apache-ant"
    ])

ANT_CLASSPATH=$ANT_HOME/lib/ant.jar:$ANT_HOME/lib/ant-launcher.jar

AC_PATH_PROG([ANT], [ant], [no], [$PATH:$JAVA_HOME/bin:$ANT_HOME/bin])

if test x"$ANT" = x"no"; then
    AC_MSG_ERROR([ANT compiler not found])
fi

AC_SUBST(ANT_HOME)
AC_SUBST(ANT_CLASSPATH)

#--------------------------------------------------------------------

KRB5_DEFAULT_ENABLED="yes"
AC_ARG_ENABLE([krb5-default],
    [AC_HELP_STRING([--enable-krb5-default], [enable default krb5.conf/krb5.keytab (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AC_DEFINE_UNQUOTED(USE_DEFAULT_KRB5_PATHS, "1", [ Use /etc/krb5.conf /etc/krb5.keytab ])
            KRB5_DEFAULT_ENABLED="yes"
        else
            KRB5_DEFAULT_ENABLED="no"
        fi
    ])

AS_AC_EXPAND(VMAFD_BIN_DIR, $prefix)
AC_SUBST(VMAFD_BIN_DIR)

AS_AC_EXPAND(VMAFD_SBIN_DIR, ["${sbindir}"])
AC_SUBST(VMAFD_SBIN_DIR)

#--------------------------------------------------------------------

if test x"$localstatedir" = x"/var"; then
    vmafddbdir="$localstatedir/lib/vmware/vmafd"
else
    vmafddbdir="$localstatedir/vmafd"
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

#--------------------------------------------------------------------

initddir=$sysconfdir/init.d
AC_SUBST(initddir)

pymodulesdir=$prefix/site-packages/identity
AC_SUBST(pymodulesdir)

javaetcdir=/etc/vmware/java
AC_SUBST(javaetcdir)

vmafdjarsdir="$prefix/jars"
AC_SUBST(vmafdjarsdir)

jreextdir="/usr/java/packages/lib/ext"
AC_SUBST(jreextdir)

#--------------------------------------------------------------------

AC_ARG_ENABLE([gcov],
    [AC_HELP_STRING([--enable-gcov], [enable code coverage (default: disabled)])],
    [
        if test x"$enableval" = x"yes"
        then
            AM_CFLAGS="$AM_CFLAGS -g -O0 -fprofile-arcs -ftestcoverage"
            AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -DVMCA_DEBUG"
        fi
    ])

#--------------------------------------------------------------------

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

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $BOOST_INCLUDES"
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS(boost/asio.hpp)
AC_LANG_POP([C++])
CPPFLAGS="$saved_CPPFLAGS"

#--------------------------------------------------------------------

AC_LANG_PUSH([C++])
saved_LIBS="$LIBS"
LOG4CPP_LIBS="-llog4cpp"
AC_LANG_PUSH([C++])
saved_LIBS="$LIBS"
LOG4CPP_LIBS="-static -llog4cpp"

AS_AC_EXPAND(VMCA_BIN_DIR, $prefix)
AC_SUBST(VMCA_BIN_DIR)

AS_AC_EXPAND(VMCA_SBIN_DIR, ["${sbindir}"])
AC_SUBST(VMCA_SBIN_DIR)

if test x"$localstatedir" = x"/var"; then
    vmcadbdir="$localstatedir/lib/vmware/vmca"
else
    vmcadbdir="$localstatedir/vmca"
fi
AC_SUBST(vmcadbdir)
AS_AC_EXPAND(VMCA_DB_DIR, $vmcadbdir)
AC_SUBST(VMCA_DB_DIR)
AC_DEFINE_UNQUOTED(VMCA_DB_DIR, "$VMCA_DB_DIR", [Database directory])

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

lwcertooldir=$prefix/bin
AC_SUBST(lwcertooldir)

#--------------------------------------------------------------------
# CURL components

AC_ARG_WITH([curl],
    [AC_HELP_STRING([--with-curl=<dir>], [use CURL binaries rooted at prefix <dir> ])],
    [
        CURL_BASE_PATH="$withval"
    ],
    [
        CURL_BASE_PATH="/usr"
    ])

CURL_INCLUDES="-I$CURL_BASE_PATH/include"
CURL_LDFLAGS="-L$CURL_BASE_PATH/lib"

AC_SUBST(CURL_BASE_PATH)
AC_SUBST(CURL_INCLUDES)
AC_SUBST(CURL_LDFLAGS)

#--------------------------------------------------------------------
# Commons Daemon component

AC_ARG_WITH([commons-daemon],
    [AC_HELP_STRING([--with-commons-daemon=<dir>], [use Commons daemon binaries rooted at prefix <dir> ])],
    [
        COMMONS_DAEMON_HOME="$withval"
    ],
    [
        COMMONS_DAEMON_HOME="/usr"
    ])

JSVC="$COMMONS_DAEMON_HOME/bin/jsvc"

AC_SUBST(COMMONS_DAEMON_HOME)
AC_SUBST(JSVC)

#--------------------------------------------------------------------
# JAX-WS component

AC_ARG_WITH([jax-ws],
    [AC_HELP_STRING([--with-jax-ws=<dir>], [use JAX WS binaries rooted at prefix <dir> ])],
    [
        JAX_WS_HOME="$withval"
    ],
    [
        JAX_WS_HOME="/opt/jaxws-ri-2.2.10"
    ])

AC_SUBST(JAX_WS_HOME)

#--------------------------------------------------------------------
# Tomcat component

AC_ARG_WITH([tomcat],
    [AC_HELP_STRING([--with-tomcat=<dir>], [use Apache Tomcat binaries rooted at prefix <dir> ])],
    [
        TOMCAT_HOME="$withval"
    ],
    [
        TOMCAT_HOME="/var/opt/apache-tomcat"
    ])

TOMCAT_CLASSPATH=$TOMCAT_HOME/lib/servlet-api.jar

AC_SUBST(TOMCAT_HOME)
AC_SUBST(TOMCAT_CLASSPATH)

#--------------------------------------------------------------------

if test x"$localstatedir" = x"/var"; then
    vmstsdbdir="$localstatedir/lib/vmware/vmsts"
else
    vmstsdbdir="$localstatedir/vmsts"
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

#--------------------------------------------------------------------

vmdircfgdir="$prefix/share/config"
AS_AC_EXPAND(VMDIR_CONFIG_PATH, $vmdircfgdir)
AC_DEFINE_UNQUOTED(VMDIR_CONFIG_PATH, "$vmdircfgdir", [VMware Directory Config Path])

# vmsts component

pscsetupbindir=$prefix/bin
AC_SUBST(pscsetupbindir)

systemddir=/lib/systemd/system
AC_SUBST(systemddir)

scriptsconfdir="$datadir/config"
AC_SUBST(scriptsconfdir)

pscsetupjarsdir=$prefix/jars
AC_SUBST(pscsetupjarsdir)

#--------------------------------------------------------------------

vmmetricsconfdir="$datadir/config"
AC_SUBST(vmmetricsconfdir)

#--------------------------------------------------------------------

gitcommithash=`echo $(git rev-parse --short HEAD)`
AC_SUBST(gitcommithash)
AC_DEFINE_UNQUOTED(GIT_COMMIT_HASH, "$gitcommithash", [Git Commit Hash])

#--------------------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 vmcommon/Makefile
                 vmcommon/include/Makefile
                 vmcommon/include/public/Makefile
                 vmcommon/config/Makefile
                 vmcommon/testing/Makefile
                 vmevent/Makefile
                 vmevent/include/Makefile
                 vmevent/include/public/Makefile
                 lwraft/Makefile
                 lwraft/include/Makefile
                 lwraft/include/public/Makefile
                 lwraft/config/Makefile
                 lwraft/kdccommon/Makefile
                 lwraft/common/Makefile
                 lwraft/server/Makefile
                 lwraft/server/kdcsrvcommon/Makefile
                 lwraft/server/kdckrb5/Makefile
                 lwraft/server/kdctools/Makefile
                 lwraft/server/common/Makefile
                 lwraft/server/backend/Makefile
                 lwraft/server/mdb-store/Makefile
                 lwraft/server/metrics/Makefile
                 lwraft/server/indexcfg/Makefile
                 lwraft/server/middle-layer/Makefile
                 lwraft/server/schema/Makefile
                 lwraft/server/acl/Makefile
                 lwraft/server/ldap-head/Makefile
                 lwraft/server/rest-head/Makefile
                 lwraft/server/replication/Makefile
                 lwraft/server/saslvmdirdb/Makefile
                 lwraft/server/vmdir/Makefile
                 lwraft/client/Makefile
                 lwraft/config/post.reg
                 lwraft/testing/Makefile
                 lwraft/testing/auth/Makefile
                 lwraft/testing/query/Makefile
                 lwraft/testing/kerberos/Makefile
                 lwraft/testing/test_lib/Makefile
                 lwraft/testing/test_runner/Makefile
                 lwraft/testing/integration_tests/Makefile
                 lwraft/testing/integration_tests/acls/Makefile
                 lwraft/testing/unittests/Makefile
                 lwraft/testing/unittests/libcommon/Makefile
                 lwraft/tools/Makefile
                 lwraft/tools/lwraft-cli/Makefile
                 lwraft/tools/vdcaclmgr/Makefile
                 lwraft/tools/vdcadmintool/Makefile
                 lwraft/tools/vdcschema/Makefile
                 lwraft/tools/mdb_walflush/Makefile
                 lwraft/thirdparty/Makefile
                 lwraft/thirdparty/openldap/Makefile
                 lwraft/thirdparty/openldap/libraries/Makefile
                 lwraft/thirdparty/openldap/libraries/mdb/Makefile
                 lwraft/thirdparty/heimdal/Makefile
                 lwraft/thirdparty/heimdal/asn1/Makefile
                 lwraft/thirdparty/heimdal/krb5-crypto/Makefile
                 lwraft/thirdparty/heimdal/ntlm/Makefile
                 lwraft/thirdparty/csrp/Makefile
                 vmdir/Makefile
                 vmdir/include/Makefile
                 vmdir/include/public/Makefile
                 vmdir/config/Makefile
                 vmdir/kdccommon/Makefile
                 vmdir/common/Makefile
                 vmdir/server/Makefile
                 vmdir/server/kdcsrvcommon/Makefile
                 vmdir/server/kdckrb5/Makefile
                 vmdir/server/kdctools/Makefile
                 vmdir/server/vmkdc_mit_tools/Makefile
                 vmdir/server/vmkdc/Makefile
                 vmdir/server/common/Makefile
                 vmdir/server/backend/Makefile
                 vmdir/server/mdb-store/Makefile
                 vmdir/server/metrics/Makefile
                 vmdir/server/indexcfg/Makefile
                 vmdir/server/middle-layer/Makefile
                 vmdir/server/schema/Makefile
                 vmdir/server/acl/Makefile
                 vmdir/server/ldap-head/Makefile
                 vmdir/server/api/Makefile
                 vmdir/server/rest-head/Makefile
                 vmdir/server/replication/Makefile
                 vmdir/server/clusterstate/Makefile
                 vmdir/server/saslvmdirdb/Makefile
                 vmdir/server/vmdir/Makefile
                 vmdir/client/Makefile
                 vmdir/config/vmdir.reg
                 vmdir/testing/Makefile
                 vmdir/testing/query/Makefile
                 vmdir/testing/kerberos/Makefile
                 vmdir/testing/test_lib/Makefile
                 vmdir/testing/test_runner/Makefile
                 vmdir/testing/integration_tests/Makefile
                 vmdir/testing/integration_tests/acls/Makefile
                 vmdir/testing/integration_tests/misc/Makefile
                 vmdir/testing/integration_tests/multitenancy/Makefile
                 vmdir/testing/integration_tests/passwordapis/Makefile
                 vmdir/testing/integration_tests/search/Makefile
                 vmdir/testing/unittests/Makefile
                 vmdir/testing/unittests/libcommon/Makefile
                 vmdir/tools/Makefile
                 vmdir/tools/vdcaclmgr/Makefile
                 vmdir/tools/vdcadmintool/Makefile
                 vmdir/tools/vdcpromo/Makefile
                 vmdir/tools/vdcpass/Makefile
                 vmdir/tools/vdcrepadmin/Makefile
                 vmdir/tools/vdcsetupldu/Makefile
                 vmdir/tools/vdcbackup/Makefile
                 vmdir/tools/vmkdc_admin/Makefile
                 vmdir/tools/vdcsrp/Makefile
                 vmdir/tools/vdcupgrade/Makefile
                 vmdir/tools/vdcleavefed/Makefile
                 vmdir/tools/vdcresetMachineActCred/Makefile
                 vmdir/tools/vdcmetric/Makefile
                 vmdir/tools/vdcschema/Makefile
                 vmdir/tools/test/Makefile
                 vmdir/tools/test/vmdirclienttest/Makefile
                 vmdir/tools/mdb_walflush/Makefile
                 vmdir/thirdparty/Makefile
                 vmdir/thirdparty/openldap/Makefile
                 vmdir/thirdparty/openldap/libraries/Makefile
                 vmdir/thirdparty/openldap/libraries/mdb/Makefile
                 vmdir/thirdparty/heimdal/Makefile
                 vmdir/thirdparty/heimdal/asn1/Makefile
                 vmdir/thirdparty/heimdal/krb5-crypto/Makefile
                 vmdir/thirdparty/heimdal/ntlm/Makefile
                 vmdir/thirdparty/csrp/Makefile
                 vmdir/gssapi-plugins/Makefile
                 vmdir/gssapi-plugins/ntlm/Makefile
                 vmdir/gssapi-plugins/srp/Makefile
                 vmdir/gssapi-plugins/unix/Makefile
                 vmdns/Makefile
                 vmdns/include/Makefile
                 vmdns/include/public/Makefile
                 vmdns/config/Makefile
                 vmdns/config/vmdns.reg
                 vmdns/common/Makefile
                 vmdns/vmsock/Makefile
                 vmdns/vmsock/posix/Makefile
                 vmdns/vmsock/api/Makefile
                 vmdns/server/Makefile
                 vmdns/server/common/Makefile
                 vmdns/server/rest-head/Makefile
                 vmdns/server/vmdns/Makefile
                 vmdns/client/Makefile
                 vmdns/tools/Makefile
                 vmdns/tools/cli/Makefile
                 vmdns/test/Makefile
                 vmafd/Makefile
                 vmafd/include/Makefile
                 vmafd/include/public/Makefile
                 vmafd/config/Makefile
                 vmafd/config/vmafd.reg
                 vmafd/common/Makefile
                 vmafd/vmnetevent/Makefile
                 vmafd/vmafcfg/Makefile
                 vmafd/vmafcfg/api/Makefile
                 vmafd/vmafcfg/posix/Makefile
                 vmafd/vmevent/Makefile
                 vmafd/vmevent/common/Makefile
                 vmafd/vmevent/db/Makefile
                 vmafd/vmevent/server/Makefile
                 vmafd/vmevent/client/Makefile
                 vmafd/vmevent/test/Makefile
                 vmafd/server/Makefile
                 vmafd/server/db/Makefile
                 vmafd/server/vmafd/Makefile
                 vmafd/server/vmafd/vmafd-server-defines.h
                 vmafd/client/Makefile
                 vmafd/jdepends/Makefile
                 vmafd/tools/Makefile
                 vmafd/tools/dir-cli/Makefile
                 vmafd/tools/cdc-cli/Makefile
                 vmafd/tools/cli/Makefile
                 vmafd/tools/domainjoin/Makefile
                 vmafd/tools/vdcpromo/Makefile
                 vmafd/tools/vecs-cli/Makefile
                 vmafd/tools/sl-cli/Makefile
                 vmafd/interop/Makefile
                 vmafd/interop/cdcjni/Makefile
                 vmafd/interop/heartbeatjni/Makefile
                 vmafd/interop/java/Makefile
                 vmafd/interop/jni/Makefile
                 vmafd/interop/python/Makefile
                 vmafd/test/Makefile
                 vmca/Makefile
                 vmca/include/Makefile
                 vmca/include/public/Makefile
                 vmca/config/Makefile
                 vmca/config/vmca.reg
                 vmca/common/Makefile
                 vmca/service/Makefile
                 vmca/client/Makefile
                 vmca/vmcadb/Makefile
                 vmca/certool/Makefile
                 vmca/test/Makefile
                 vmca/python/Makefile
                 vmca/jdepends/Makefile
                 vmca/java/Makefile
                 vmca/tools/Makefile
                 vmca/tools/lw-certool/Makefile
                 vmidentity/Makefile
                 vmidentity/config/setenv.sh
                 vmidentity/config/vmware-stsd.sh
                 vmidentity/config/vmware-stsd.service
                 vmidentity/config/configure-build.sh
                 vmidentity/interop/Makefile
                 vmidentity/interop/idm/Makefile
                 vmidentity/interop/idm/ad/Makefile
                 vmidentity/interop/idm/ad/server/Makefile
                 vmidentity/interop/idm/common/Makefile
                 vmidentity/interop/idm/localos/Makefile
                 vmidentity/ssoclients/Makefile
                 vmidentity/ssoclients/common/Makefile
                 vmidentity/ssoclients/common/include/Makefile
                 vmidentity/ssoclients/common/include/public/Makefile
                 vmidentity/ssoclients/common/src/Makefile
                 vmidentity/ssoclients/common/test/Makefile
                 vmidentity/ssoclients/oidc/Makefile
                 vmidentity/ssoclients/oidc/include/Makefile
                 vmidentity/ssoclients/oidc/include/public/Makefile
                 vmidentity/ssoclients/oidc/src/Makefile
                 vmidentity/ssoclients/oidc/test/Makefile
                 vmidentity/ssoclients/restclient/Makefile
                 vmidentity/ssoclients/restclient/coreclient/Makefile
                 vmidentity/ssoclients/restclient/coreclient/include/Makefile
                 vmidentity/ssoclients/restclient/coreclient/include/public/Makefile
                 vmidentity/ssoclients/restclient/coreclient/src/Makefile
                 vmidentity/ssoclients/restclient/idmclient/Makefile
                 vmidentity/ssoclients/restclient/idmclient/include/Makefile
                 vmidentity/ssoclients/restclient/idmclient/include/public/Makefile
                 vmidentity/ssoclients/restclient/idmclient/src/Makefile
                 vmidentity/ssoclients/restclient/afdclient/Makefile
                 vmidentity/ssoclients/restclient/afdclient/include/Makefile
                 vmidentity/ssoclients/restclient/afdclient/include/public/Makefile
                 vmidentity/ssoclients/restclient/afdclient/src/Makefile
                 vmidentity/ssoclients/restclient/vmdirclient/Makefile
                 vmidentity/ssoclients/restclient/vmdirclient/include/Makefile
                 vmidentity/ssoclients/restclient/vmdirclient/include/public/Makefile
                 vmidentity/ssoclients/restclient/vmdirclient/src/Makefile
                 vmidentity/ssoclients/restclient/test/Makefile
                 vmidentity/ssoclients/restclient/test/src/Makefile
                 vmidentity/build-maven/Makefile
                 vmidentity/make-target/Makefile
                 config/Makefile
                 config/jdepends/Makefile
                 config/cfgutils/Makefile
                 config/tools/Makefile
                 config/tools/ic-promote/Makefile
                 config/tools/ic-join/Makefile
                 config/tools/lightwave/Makefile
                 config/tools/lightwave/ca/Makefile
                 config/tools/lightwave/domain/Makefile
                 config/tools/lightwave/dns/Makefile
                 config/scripts/Makefile
                 config/scripts/configure-lightwave-server
                 config/scripts/configure-sts
                ])

AC_OUTPUT
