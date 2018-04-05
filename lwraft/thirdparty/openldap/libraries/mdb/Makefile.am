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

bin_PROGRAMS = mdb_stat mdb_verify_checksum

mdb_stat_SOURCES = \
    mdb_stat.c

mdb_stat_LDADD = \
    $(top_builddir)/lwraft/thirdparty/openldap/libraries/mdb/liblwraftmdb.la \
    @PTHREAD_LIBS@

mdb_verify_checksum_SOURCES = \
    mdb_verify_checksum.c

mdb_verify_checksum_LDADD = \
    $(top_builddir)/lwraft/thirdparty/openldap/libraries/mdb/liblwraftmdb.la \
    @PTHREAD_LIBS@ \
    @CRYPTO_LIBS@

mdb_verify_checksum_LDFLAGS = \
    @OPENSSL_LDFLAGS@
