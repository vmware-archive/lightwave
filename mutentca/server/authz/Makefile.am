noinst_LTLIBRARIES = libmutentcaauthz.la

libmutentcaauthz_la_SOURCES = \
    authz.c \
    globals.c \
    lightwave-authz.c

libmutentcaauthz_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@

libmutentcaauthz_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/plugin/libmutentcasrvplugin.la \
    @DL_LIBS@

libmutentcaauthz_la_LDFLAGS = \
    -static
