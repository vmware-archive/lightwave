bin_PROGRAMS = vdcrepadmin

vdcrepadmin_SOURCES = \
    parseargs.c \
    redundancy.c \
    main.c

vdcrepadmin_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/tools/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcrepadmin_LDADD = \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/common/libcommon.la \
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
    @LBER_LIBS@ \
    @DL_LIBS@

vdcrepadmin_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
