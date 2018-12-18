bin_PROGRAMS = vmafd-cli

vmafd_cli_SOURCES = \
    cli.c  \
    main.c \
    utils.c

vmafd_cli_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmafd_cli_LDADD = \
    $(top_builddir)/vmafd/vmafcfg/api/libvmafcfgapi.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmafd/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
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

vmafd_cli_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
