
noinst_LTLIBRARIES = librest-head.la

librest_head_la_SOURCES = \
    libmain.c

if REST_ENABLED

librest_head_la_SOURCES += \
    auth.c \
    authtoken.c \
    cache.c \
    curlhandlecache.c \
    decode.c \
    encode.c \
    etcdapi.c \
    globals.c \
    handler.c \
    httperror.c \
    idpapi.c \
    ldapapi.c \
    ldapcontrol.c \
    lightwave.c \
    metrics.c \
    metricsapi.c \
    objectapi.c \
    operation.c \
    param.c \
    proxy.c \
    resource.c \
    result.c \
    vmafd.c \
    utils.c

endif

librest_head_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @COPENAPI_INCLUDES@ \
    @CRESTENGINE_INCLUDES@

librest_head_la_LDFLAGS = \
    -static
