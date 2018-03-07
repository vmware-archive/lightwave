noinst_LTLIBRARIES = liblwraftmdb.la

liblwraftmdb_la_SOURCES = \
    mdb.c \
    midl.c

liblwraftmdb_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -D MDB_USE_PWRITEV \
    -D MDB_PARANOID

liblwraftmdb_la_LDFLAGS = \
    -static
