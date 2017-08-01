SUBDIRS = \
    include \
    . \
    testing

lib_LTLIBRARIES = libvmmetrics.la

libvmmetrics_la_CPPFLAGS = \
    -I$(top_srcdir)/vmmetrics/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@

libvmmetrics_la_SOURCES = \
    memory.c \
    vmmetrics.c

libvmmetrics_la_LIBADD = \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmmetrics_la_LDFLAGS = \
    @LW_LDFLAGS@
