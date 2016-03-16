bin_PROGRAMS = sl-cli

sl_cli_SOURCES = \
    superlogWrapper.c \
    parseargs.c \
    main.c

sl_cli_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @VMDIR_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

sl_cli_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    @VMDIR_LIBS@ \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LDAP_LIBS@   \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

sl_cli_LDFLAGS = \
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
