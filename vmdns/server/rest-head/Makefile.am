
noinst_LTLIBRARIES = librest-head.la

librest_head_la_SOURCES = \
    libmain.c

if REST_ENABLED

librest_head_la_SOURCES += \
    globals.c \
    handler.c \
    httperror.c \
    metricsapi.c \
    operation.c \
    resource.c \
    result.c

endif

librest_head_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdns/server/include \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

librest_head_la_LDFLAGS = \
    -static
