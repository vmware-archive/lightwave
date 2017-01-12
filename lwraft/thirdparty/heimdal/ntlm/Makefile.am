noinst_LTLIBRARIES = libheimntlm.la

libheimntlm_la_SOURCES = \
    ntlm.c

libheimntlm_la_CPPFLAGS = \
    @OPENSSL_INCLUDES@ \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/public \
    -DHAVE_OPENSSL \
    -I$(top_srcdir)/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/thirdparty/heimdal/krb5-crypto

libheimntlm_la_LDFLAGS = \
    -static
