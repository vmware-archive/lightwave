bin_PROGRAMS = vdcpass

vdcpass_SOURCES = \
    main.c \
    parseargs.c

vdcpass_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcpass_LDADD = \
    $(top_builddir)/client/libvmdirclient.la \
    $(top_builddir)/common/libcommon.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

vdcpass_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

