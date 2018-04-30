bin_PROGRAMS = lwraftpromo

lwraftpromo_SOURCES = \
    parseargs.c \
    main.c

lwraftpromo_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

lwraftpromo_LDADD = \
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

lwraftpromo_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
