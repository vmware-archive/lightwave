lib_LTLIBRARIES = libssocoreclient.la

libssocoreclient_la_SOURCES = \
    access_token.c \
    boolean_data.c \
    certificate_array_data.c \
    certificate_data.c \
    credentials_data.c \
    debug.c \
    integer_data.c \
    json_utils.c \
    long_data.c \
    requests.c \
    rest_client.c \
    rest_server_error.c \
    signature_utils.c \
    string_data.c \
    utils.c

libssocoreclient_la_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/

libssocoreclient_la_LIBADD = \
    @top_builddir@/ssoclients/common/src/libssocommon.la
