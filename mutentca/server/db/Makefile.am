noinst_LTLIBRARIES = libmutentcadb.la

libmutentcadb_la_SOURCES = \
    globals.c \
    db.c \
    utils.c

libmutentcadb_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@

libmutentcadb_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/plugin/libmutentcasrvplugin.la \
    @DL_LIBS@

libmutentcadb_la_LDFLAGS =
