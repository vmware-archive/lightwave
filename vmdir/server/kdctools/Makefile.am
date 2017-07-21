noinst_LTLIBRARIES = libvmkdctools.la

libvmkdctools_la_SOURCES = \
    fgetsl.c \
    princtok.c \
    parsekt.c

libvmkdctools_la_CPPFLAGS = \
    -DOPENSSL_NO_KRB5 \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/server/kdckrb5 \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/vmdir/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/vmdir/server/vmkdc \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkdctools_la_LDFLAGS = \
    -static
