bin_PROGRAMS = common

check_PROGRAMS = common
TESTS = common

common_SOURCES = \
    queue.c

common_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

common_LDADD = \
    $(top_builddir)/lwraft/server/common/libsrvcommon.la \
    $(top_builddir)/lwraft/common/libcommon.la \
    @PTHREAD_LIBS@ \
    @CMOCKA_LIBS@

common_LDFLAGS = \
    -static
