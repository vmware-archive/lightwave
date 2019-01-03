noinst_LTLIBRARIES = libmutentcametrics.la

libmutentcametrics_la_SOURCES = \
    globals.c \
    libmain.c \
    operation.c \
    util.c

libmutentcametrics_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @JANSSON_INCLUDES@

libmutentcametrics_la_LDFLAGS = \
    -static
