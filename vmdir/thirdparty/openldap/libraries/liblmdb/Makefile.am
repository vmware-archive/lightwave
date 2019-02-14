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

if MDB_0_9_22
bin_PROGRAMS = mdb_verify_checksum

mdb_verify_checksum_SOURCES = \
    mdb_verify_checksum.c

mdb_verify_checksum_LDADD = \
    $(top_builddir)/vmdir/thirdparty/openldap/libraries/liblmdb/libvmdirmdb.la \
    @PTHREAD_LIBS@ \
    @CRYPTO_LIBS@

mdb_verify_checksum_LDFLAGS = \
    @OPENSSL_LDFLAGS@
endif
