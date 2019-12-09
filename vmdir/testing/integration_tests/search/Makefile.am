vmdirtestlib_LTLIBRARIES = libsearchtests.la

libsearchtests_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/sts/ssoclients/common/include/public \
    -I$(top_srcdir)/sts/ssoclients/oidc/include \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libsearchtests_la_SOURCES = \
    control.c \
    main.c \
    abnormalsearch.c \
    search.c \
    searchutil.c \
    provision.c \
    restsearch.c

libsearchtests_la_LIBADD = \
    @top_builddir@/vmdir/testing/test_lib/libvmdirtesting.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libsearchtests_la_LDFLAGS = \
    -export-symbols @top_srcdir@/vmdir/testing/integration_tests/search/libsearchtests.exp \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
