bin_PROGRAMS = vdcschema

vdcschema_SOURCES = \
    conn.c \
    main.c \
    operations.c \
    parseargs.c \
    syntax.c \
    util.c

vdcschema_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    -I$(top_srcdir)/client \
    -I$(top_builddir)/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcschema_LDADD = \
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

vdcschema_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
