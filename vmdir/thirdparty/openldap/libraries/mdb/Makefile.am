noinst_LTLIBRARIES = libvmdirmdb.la

libvmdirmdb_la_SOURCES = \
    mdb.c \
    midl.c

libvmdirmdb_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -D MDB_PARANOID

libvmdirmdb_la_LDFLAGS = \
    -static
