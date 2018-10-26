bin_PROGRAMS = testmutentcadb

check_PROGRAMS = testmutentcadb
TESTS = testmutentcadb

SUBDIRS = \
    test-mutentcadb-plugin \
    test-mutentcadb-config

testmutentcadb_SOURCES = \
    db.c \
    main.c

testmutentcadb_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@

testmutentcadb_LDADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/db/libmutentcadb.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CRYPTO_LIBS@ \
    @CMOCKA_LIBS@

testmutentcadb_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
