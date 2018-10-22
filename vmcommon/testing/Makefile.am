noinst_PROGRAMS = vmcommon_test

check_PROGRAMS = vmcommon_test
TESTS = $(check_PROGRAMS)

vmcommon_test_SOURCES = \
    countertest.c \
    gaugetest.c \
    histogramtest.c \
    mixedtest.c \
    vmhttpclienttest.c \
    vmsignaturetest.c \
    vmjsonresulttest.c \
    vmjsonresultobjectmaptest.c \
    main.c

vmcommon_test_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

vmcommon_test_LDADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    @LWBASE_LIBS@

vmcommon_test_LDFLAGS = \
    @LW_LDFLAGS@
