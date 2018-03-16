lib_LTLIBRARIES = libssocommon.la

libssocommon_la_SOURCES = \
    json.c \
    jwt.c \
    jwk.c \
    base64.c \
    cdc.c \
    http_client.c \
    http_client_response.c \
    memory.c \
    signature.c \
    string_builder.c \
    key_value_pair.c \
    strings.c \
    errors.c

libssocommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/ \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    @JANSSON_INCLUDES@ \
    @CURL_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libssocommon_la_LIBADD = \
    @JANSSON_LIBS@ \
    @CURL_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libssocommon_la_LDFLAGS = \
    @JANSSON_LDFLAGS@ \
    @CURL_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
