
noinst_LTLIBRARIES = libvmdirserverapi.la

libvmdirserverapi_la_SOURCES = \
    certs.c \
    createcomputer.c \
    joinatomic.c \
    password.c

libvmdirserverapi_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

libvmdirserverapi_la_LDFLAGS = \
    -static
