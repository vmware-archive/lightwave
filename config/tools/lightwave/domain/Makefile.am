noinst_LTLIBRARIES = libdomain.la

libdomain_la_SOURCES = \
    demote.c \
    join.c \
    leave.c \
    promote.c

libdomain_la_CPPFLAGS = \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

libdomain_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
