lib_LTLIBRARIES = libssooidc.la

libssooidc_la_SOURCES = \
    oidc_client.c \
    oidc_server_metadata.c \
    oidc_token.c \
    oidc_id_token.c \
    oidc_access_token.c \
    oidc_token_success_response.c \
    oidc_error_response.c

libssooidc_la_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/oidc/include/public/

libssooidc_la_LIBADD = \
    @top_builddir@/ssoclients/common/src/libssocommon.la
