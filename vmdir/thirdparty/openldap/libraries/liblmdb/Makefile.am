lib_LTLIBRARIES = libvmdirmdb.la

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
    -version-number 0:9:22

bin_PROGRAMS = mdb_stat mdb_verify_checksum

mdb_stat_SOURCES = \
    mdb_stat.c

mdb_stat_LDADD = \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/liblmdb/.libs/libvmdirmdb.so \
    @PTHREAD_LIBS@

mdb_stat_LDFLAGS = \
    @OPENSSL_LDFLAGS@

mdb_verify_checksum_SOURCES = \
    mdb_verify_checksum.c

mdb_verify_checksum_LDADD = \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/liblmdb/.libs/libvmdirmdb.so \
    @PTHREAD_LIBS@ \
    @CRYPTO_LIBS@

mdb_verify_checksum_LDFLAGS = \
    @OPENSSL_LDFLAGS@
