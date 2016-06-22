lib_LTLIBRARIES = libidm.la

libidm_la_SOURCES = \
    globals.c     \
    gssapi-sspi.c \
    ldap.c        \
    libmain.c     \
    sidcache.c

libidm_la_CPPFLAGS = \
    -I$(top_srcdir)/interop/include \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/interop/idm/include \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libidm_la_LIBADD = \
    @top_builddir@/interop/idm/common/libidmcommon.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@

libidm_la_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
