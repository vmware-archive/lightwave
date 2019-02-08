noinst_LTLIBRARIES = libvmdirmdb.la

libvmdirmdb_la_SOURCES = \
    mdb.c \
    midl.c

libvmdirmdb_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -D MDB_USE_PWRITEV \
    -D MDB_PARANOID \
    -Wno-unused-but-set-variable \
    -Wno-unused-result

libvmdirmdb_la_LDFLAGS = \
    -static
