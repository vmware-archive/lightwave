noinst_LTLIBRARIES = libmutentcasrvcommon.la

libmutentcasrvcommon_la_SOURCES = \
    api.c \
    jsonutils.c \
    util.c

libmutentcasrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

libmutentcasrvcommon_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    @JANSSON_LIBS@

libmutentcasrvcommon_la_LDFLAGS = \
    -static
