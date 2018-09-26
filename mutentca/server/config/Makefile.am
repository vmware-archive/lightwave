noinst_LTLIBRARIES = libmutentcasrvconfig.la

libmutentcasrvconfig_la_SOURCES = \
    config.c

libmutentcasrvconfig_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    @LW_INCLUDES@ \
    @JANSSON_INCLUDES@

libmutentcasrvconfig_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    @JANSSON_LIBS@

libmutentcasrvconfig_la_LDFLAGS = \
    -static
