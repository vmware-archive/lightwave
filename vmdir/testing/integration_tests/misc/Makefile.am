lib_LTLIBRARIES = libmisctests.la

libmisctests_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libmisctests_la_SOURCES = \
    main.c \
    dcaccount.c \
    groupmembership.c \
    tombstone.c

libmisctests_la_LIBADD = \
    @top_builddir@/testing/test_lib/libvmdirtesting.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libmisctests_la_LDFLAGS = \
    -export-symbols @top_srcdir@/testing/integration_tests/misc/libmisctests.exp \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
