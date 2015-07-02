bin_PROGRAMS = vmdns-cli

vmdns_cli_SOURCES = \
    cli.c  \
    main.c \
    utils.c

vmdns_cli_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@

vmdns_cli_LDADD = \
    $(top_builddir)/client/libvmdnsclient.la \
    @top_builddir@/common/libcommon.la \
    @LDAP_LIBS@ \
    @VMDIR_LIBS@ \
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
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
