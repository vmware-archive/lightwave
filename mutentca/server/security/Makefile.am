noinst_LTLIBRARIES = libmutentcasecurity.la

libmutentcasecurity_la_SOURCES = \
    globals.c \
    security.c

libmutentcasecurity_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/plugins/include/public \
    @LW_INCLUDES@

libmutentcasecurity_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/plugin/libmutentcasrvplugin.la \
    @DL_LIBS@
