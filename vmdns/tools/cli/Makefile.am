bin_PROGRAMS = vmdns-cli

vmdns_cli_SOURCES = \
    cli.c  \
    main.c \
    utils.c

vmdns_cli_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

vmdns_cli_LDADD = \
    $(top_builddir)/vmdns/client/libvmdnsclient.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    @top_builddir@/vmdns/common/libcommon.la \
    @LDAP_LIBS@ \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWREG_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

vmdns_cli_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
