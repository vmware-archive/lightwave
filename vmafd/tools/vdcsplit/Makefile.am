bin_PROGRAMS = vdcsplit

vdcsplit_SOURCES = \
    main.c \
    parseargs.c

vdcsplit_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcsplit_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

vdcsplit_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
