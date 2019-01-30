bin_PROGRAMS = post-cli

post_cli_SOURCES = \
    clinode.c \
    clibackup.c \
    clidr.c \
    cliprocess.c \
    main.c

post_cli_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

post_cli_LDADD = \
    $(top_builddir)/lwraft/common/libcommon.la \
    $(top_builddir)/lwraft/client/libpostclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@ \
    @DL_LIBS@

post_cli_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
