noinst_LTLIBRARIES = liblwraftmdb.la

liblwraftmdb_la_SOURCES = \
    mdb.c \
    midl.c

liblwraftmdb_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include

liblwraftmdb_la_LDFLAGS = \
    -static
