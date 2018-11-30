bin_PROGRAMS = vecs-cli

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

vecs_cli_SOURCES = \
    cli.c  \
    main.c \
    utils.c
    
vecs_cli_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@
    
vecs_cli_LDADD = \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmafd/common/libcommon.la \
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
