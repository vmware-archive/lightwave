noinst_LTLIBRARIES = libmutentcasrvcommon.la

libmutentcasrvcommon_la_SOURCES = \
    api.c \
    jsonutils.c \
    token_util.c \
    util.c

libmutentcasrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

libmutentcasrvcommon_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/pkcs/libmutentcapkcs.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    @JANSSON_LIBS@

libmutentcasrvcommon_la_LDFLAGS = \
    -static
