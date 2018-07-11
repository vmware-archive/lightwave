noinst_LTLIBRARIES = libvmkdctools.la

libvmkdctools_la_SOURCES = \
    fgetsl.c \
    princtok.c \
    parsekt.c

libvmkdctools_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/lwraft/server/kdckrb5 \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/lwraft/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/lwraft/server/vmkdc \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkdctools_la_LDFLAGS = \
    -static
