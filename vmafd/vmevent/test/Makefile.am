noinst_PROGRAMS = vmevent_test

vmevent_test_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/vmevent/include \
    -I$(top_srcdir)/vmafd/vmevent/include/public \
    -I$(top_builddir)/vmafd/vmevent/client \
    @SQLITE_INCLUDES@ \
    @LW_INCLUDES@  \
    @OPENSSL_INCLUDES@ \
    -Wno-unused-result

vmevent_test_SOURCES = \
	testsrv.c

vmevent_test_LDADD = \
    $(top_builddir)/vmafd/vmevent/db/libvmeventdb.la \
    $(top_builddir)/vmafd/vmevent/client/libvmeventclient.la \
    @DCERPC_LIBS@ \
    @DL_LIBS@  \
    @PTHREAD_LIBS@ \
    @GSSAPI_LIBS@ \
    @LWADVAPI_LIBS@ \
    @LWBASE_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @CRYPTO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWIO_LIBS@ \
    @UUID_LIBS@ \
    @SQLITE_LIBS@ \
    @LDAP_LIBS@

vmevent_test_LDFLAGS = \
    -ldl \
    @SQLITE_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
