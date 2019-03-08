noinst_LTLIBRARIES = libgssapi_common.la

libgssapi_common_la_CPPFLAGS = \
   -I$(top_srcdir)/vmdir/gssapi-plugins/include/public

libgssapi_common_la_SOURCES = \
    creds_plugin.c

libgssapi_common_la_LIBADD = \
    -ldl

libgssapi_common_la_LDFLAGS = \
    -static
