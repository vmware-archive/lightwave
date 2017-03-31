bin_PROGRAMS = dir-cli

dir_cli_SOURCES = \
    cert.c \
    cli.c  \
    ldap.c \
    main.c

dir_cli_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @VMDIR_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

dir_cli_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/vmafcfg/api/libvmafcfgapi.la \
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

dir_cli_LDFLAGS = \
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
 
