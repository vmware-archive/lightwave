noinst_LTLIBRARIES = libmutentcasrvplugin.la

libmutentcasrvplugin_la_SOURCES = \
    plugin.c

libmutentcasrvplugin_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    @LW_INCLUDES@

libmutentcasrvplugin_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    @DL_LIBS@

libmutentcasrvplugin_la_LDFLAGS = \
    -static
