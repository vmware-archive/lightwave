SUBDIRS = \
    include \
    . \
    config \
    testing

lib_LTLIBRARIES = libvmcommon.la

libvmcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

libvmcommon_la_SOURCES = \
    memory.c \
    vmmetrics.c

libvmcommon_la_LIBADD = \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmcommon_la_LDFLAGS = \
    @LW_LDFLAGS@
