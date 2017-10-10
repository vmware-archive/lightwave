postsasl_LTLIBRARIES = libsaslpostdb.la

libsaslpostdb_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libsaslpostdb_la_SOURCES = \
    vmdirdb_init.c \
    vmdirdb.c

libsaslpostdb_la_LIBADD = \
    $(top_builddir)/lwraft/client/libpostclient.la \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

libsaslpostdb_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
