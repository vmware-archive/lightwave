noinst_LTLIBRARIES = libdns.la

libdns_la_SOURCES = \
    delete.c

libdns_la_CPPFLAGS = \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

libdns_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
