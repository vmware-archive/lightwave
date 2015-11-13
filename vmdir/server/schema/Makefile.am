
noinst_LTLIBRARIES = libschema.la

libschema_la_SOURCES = \
    adcompatibleschema.c \
    api.c           \
    attr2idmap.c    \
    check.c         \
    globals.c       \
    init.c          \
    libmain.c       \
    load.c          \
    matchingrule.c  \
    parse.c         \
    syntax.c        \
    verify.c

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

