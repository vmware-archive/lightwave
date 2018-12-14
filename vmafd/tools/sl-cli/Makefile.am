bin_PROGRAMS = sl-cli

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

sl_cli_SOURCES = \
    superlogWrapper.c \
    parseargs.c \
    main.c

sl_cli_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

sl_cli_LDADD = \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmafd/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
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
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
