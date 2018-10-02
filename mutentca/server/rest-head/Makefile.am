noinst_LTLIBRARIES = libmutentcaresthead.la

libmutentcaresthead_la_SOURCES = \
    libmain.c

if REST_ENABLED

libmutentcaresthead_la_SOURCES += \
    globals.c \
    handler.c \
    httperror.c \
    openssl.c \
    operation.c \
    param.c \
    resource.c \
    result.c

endif

libmutentcaresthead_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

libmutentcaresthead_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la

libmutentcaresthead_la_LDFLAGS = \
    -static
