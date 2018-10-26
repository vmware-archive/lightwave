
noinst_LTLIBRARIES = libschema.la

libschema_la_SOURCES = \
    api.c \
    check.c \
    dn.c \
    globalmutex.c \
    globals.c \
    head.c \
    idmap.c \
    instance.c \
    libmain.c \
    matchingrule.c \
    parse.c \
    patch.c \
    replstatus.c \
    syntax.c \
    util.c

libschema_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libschema_la_LDFLAGS = \
    -static

