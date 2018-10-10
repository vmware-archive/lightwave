noinst_LTLIBRARIES = libmutentcapkcs.la

libmutentcapkcs_la_SOURCES = \
    pkcs.c

libmutentcapkcs_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

libmutentcapkcs_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    @OPENSSL_LIBS@ \
    @DL_LIBS@

libmutentcapkcs_la_LDFLAGS = \
    @OPENSSL_LDFLAGS@
