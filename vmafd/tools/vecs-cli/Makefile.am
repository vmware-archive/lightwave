bin_PROGRAMS = vecs-cli

vecs_cli_SOURCES = \
    cli.c  \
    main.c \
    utils.c
    
vecs_cli_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@
    
vecs_cli_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

vecs_cli_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

