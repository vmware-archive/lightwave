
noinst_LTLIBRARIES = librest-head.la

librest_head_la_SOURCES = \
    libmain.c

if REST_ENABLED

librest_head_la_SOURCES += \
    auth.c \
    authtoken.c \
    cache.c \
    decode.c \
    encode.c \
    globals.c \
    handler.c \
    httperror.c \
    ldapapi.c \
    metrics.c \
    metricsapi.c \
    operation.c \
    param.c \
    resource.c \
    certsapi.c \
    passwordapi.c \
    result.c \
    utils.c

endif

librest_head_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

librest_head_la_LIBADD = \
    $(top_builddir)/vmdir/server/api/libvmdirserverapi.la

librest_head_la_LDFLAGS = \
    -static
