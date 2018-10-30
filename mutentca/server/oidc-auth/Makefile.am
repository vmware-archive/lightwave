noinst_LTLIBRARIES = libmutentcaoidc.la

libmutentcaoidc_la_SOURCES = \
    oidc-auth.c \
    oidcutils.c

libmutentcaoidc_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libmutentcaoidc_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    @OPENSSL_LIBS@ \
    @DL_LIBS@

libmutentcaoidc_la_LDFLAGS = \
    -static
