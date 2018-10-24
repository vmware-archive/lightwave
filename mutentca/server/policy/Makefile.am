noinst_LTLIBRARIES = libmutentcapolicy.la

libmutentcapolicy_la_SOURCES = \
    parse.c \
    policy.c \
    util.c \
    validate.c

libmutentcapolicy_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@ \
    @JANSSON_INCLUDES@

libmutentcapolicy_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la
