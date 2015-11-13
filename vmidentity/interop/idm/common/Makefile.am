
noinst_LTLIBRARIES = libidmcommon.la

libidmcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -D_XOPEN_SOURCE=1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libidmcommon_la_SOURCES = \
    memory.c

libidmcommon_la_LIBADD = \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libidmcommon_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@

