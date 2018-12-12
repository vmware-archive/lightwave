noinst_LTLIBRARIES = libvmcaresthead.la

libvmcaresthead_la_SOURCES = \
    libmain.c

if REST_V2_ENABLED

libvmcaresthead_la_SOURCES += \
    globals.c \
    handler.c \
    httperror.c \
    operation.c \
    resource.c \
    result.c

endif

libvmcaresthead_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

libvmcaresthead_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la

libvmcaresthead_la_LDFLAGS = \
    -static
