noinst_PROGRAMS = \
    krb5keys-test

krb5keys_test_SOURCES = \
    krb5keys-test.c

krb5keys_test_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server \
    -I$(top_srcdir)/vmdir/server/tools \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

krb5keys_test_LDADD = \
    $(top_builddir)/vmdir/server/common/libsrvcommon.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1.la \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

krb5keys_test_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
