bin_PROGRAMS = vdcrepadmin

vdcrepadmin_SOURCES = \
    parseargs.c \
    main.c

vdcrepadmin_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/lwraft/tools/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcrepadmin_LDADD = \
    $(top_builddir)/lwraft/client/libpostclient.la \
    $(top_builddir)/lwraft/common/libcommon.la \
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
    @LDAP_LIBS@

vdcrepadmin_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
