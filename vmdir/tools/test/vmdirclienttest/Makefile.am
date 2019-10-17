bin_PROGRAMS = vmdirclienttest

vmdirclienttest_SOURCES = \
    main.c \
    saslclient.c

vmdirclienttest_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/tools/include \
    -I$(top_srcdir)/vmdir/client \
    -I$(top_builddir)/vmdir/client \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmdirclienttest_LDADD = \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @LDAP_LIBS@

vmdirclienttest_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@
