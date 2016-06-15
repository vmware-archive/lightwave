
noinst_LTLIBRARIES = libschema.la

libschema_la_SOURCES = \
    api.c \
    check.c \
    globalmutex.c \
    globals.c \
    head.c \
    idmap.c \
    instance.c \
    libmain.c \
    matchingrule.c \
    parse.c \
    patch.c \
    syntax.c \
    util.c \
    legacy/legacylibmain.c \
    legacy/legacyload.c \
    legacy/legacypatch.c \
    legacy/legacyutil.c

libschema_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libschema_la_LDFLAGS = \
    -static

