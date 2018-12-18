
noinst_LTLIBRARIES = libvmcadb.la

libvmcadb_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    @SQLITE_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmcadb_la_SOURCES = \
    certificate.c  \
    database.c     \
    dbcontext.c    \
    errorCodeMap.c \
    globals.c      \
    libmain.c

libvmcadb_la_LIBADD = \
    @top_builddir@/vmca/common/libcommon.la \
    @SQLITE_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ 


libvmcadb_la_LDFLAGS = \
    -DLDAP_DEPRECATED \
    @SQLITE_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -static \
    @LDAP_LDFLAGAS@ \
    @OPENSSL_LDFLAGS@
