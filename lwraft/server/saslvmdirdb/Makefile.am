lwraftsasl_LTLIBRARIES = libsasllwraftdb.la

libsasllwraftdb_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libsasllwraftdb_la_SOURCES = \
    vmdirdb_init.c \
    vmdirdb.c

libsasllwraftdb_la_LIBADD = \
    $(top_builddir)/client/liblwraftclient.la \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libsasllwraftdb_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
