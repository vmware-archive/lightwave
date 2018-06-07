noinst_PROGRAMS = vmdir_test_runner

vmdir_test_runner_SOURCES = \
    main.c \
    testinfrastructure.c

vmdir_test_runner_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmdir_test_runner_LDADD = \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    ../test_lib/libvmdirtesting.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @DL_LIBS@

vmdir_test_runner_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
