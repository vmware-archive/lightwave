bin_PROGRAMS = dequetest

dequetest_SOURCES = \
    main.c

dequetest_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

dequetest_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/liblwraftclient.la \
    $(top_builddir)/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

dequetest_LDFLAGS = \
    @LW_LDFLAGS@
