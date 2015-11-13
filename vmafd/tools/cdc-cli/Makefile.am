bin_PROGRAMS = cdc-cli

cdc_cli_SOURCES = \
    cli.c  \
    main.c

cdc_cli_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

cdc_cli_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @CRYPTO_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@

cdc_cli_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

