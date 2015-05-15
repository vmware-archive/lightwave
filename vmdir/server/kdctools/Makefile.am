noinst_LTLIBRARIES = libvmkdctools.la

libvmkdctools_la_SOURCES = \
    fgetsl.c \
    princtok.c \
    parsekt.c

libvmkdctools_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server/kdckrb5 \
    -I$(top_srcdir)/thirdparty/heimdal \
    -I$(top_srcdir)/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/server/vmkdc \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkdctools_la_LDFLAGS = \
    -static
