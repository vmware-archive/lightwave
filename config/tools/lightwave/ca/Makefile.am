noinst_LTLIBRARIES = libca.la

libca_la_SOURCES = \
    getsignedcert.c \
    utils.c

libca_la_CPPFLAGS = \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    @LW_INCLUDES@

libca_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
