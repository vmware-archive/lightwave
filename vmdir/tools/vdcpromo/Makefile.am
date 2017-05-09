bin_PROGRAMS = vdcvmdirpromo

vdcvmdirpromo_SOURCES = \
    parseargs.c \
    main.c

vdcvmdirpromo_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcvmdirpromo_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmdirclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @SASL_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@

vdcvmdirpromo_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
