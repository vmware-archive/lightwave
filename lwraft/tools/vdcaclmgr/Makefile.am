bin_PROGRAMS = postaclmgr

postaclmgr_SOURCES = \
    acl.c           \
    ldap.c          \
    main.c

postaclmgr_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/tools/include \
    -I$(top_srcdir)/lwraft/client \
    -I$(top_builddir)/lwraft/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

postaclmgr_LDADD = \
    $(top_builddir)/lwraft/client/libpostclient.la \
    $(top_builddir)/lwraft/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@ \
    @UUID_LIBS@ \
    @LBER_LIBS@

postaclmgr_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
