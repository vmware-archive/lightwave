bin_PROGRAMS = vdcmetric

vdcmetric_SOURCES = \
    superlogWrapper.c \
    parseargs.c \
    main.c

vdcmetric_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/tools/include \
    -I$(top_srcdir)/vmdir/client \
    -I$(top_builddir)/vmdir/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcmetric_LDADD = \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/common/libcommon.la \
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
    @DL_LIBS@

vdcmetric_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
