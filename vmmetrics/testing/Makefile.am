noinst_PROGRAMS = vmmetrics_test

vmmetrics_test_SOURCES = \
    countertest.c \
    gaugetest.c \
    histogramtest.c \
    mixedtest.c \
    main.c

vmmetrics_test_CPPFLAGS = \
    -I$(top_srcdir)/vmmetrics/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@

vmmetrics_test_LDADD = \
    $(top_builddir)/vmmetrics/libvmmetrics.la \
    @LWBASE_LIBS@

vmmetrics_test_LDFLAGS = \
    @LW_LDFLAGS@
