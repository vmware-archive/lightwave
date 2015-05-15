
noinst_LTLIBRARIES = libvmeventdb.la

libvmeventdb_la_CPPFLAGS = \
    -I$(top_srcdir)/vmevent/include \
    -I$(top_srcdir)/vmevent/include/public \
    @SQLITE_INCLUDES@ \
    @LW_INCLUDES@  \
    @OPENSSL_INCLUDES@

libvmeventdb_la_SOURCES = \
    database.c     \
    dbcontext.c    \
    errorCodeMap.c \
    globals.c      \
    libmain.c      \
    package.c

libvmeventdb_la_LIBADD = \
    $(top_builddir)/vmevent/common/libvmeventcommon.la \
    @SQLITE_LIBS@ \
    @LWBASE_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWMSG_LIBS@ \
    @PTHREAD_LIBS@

libvmeventdb_la_LDFLAGS = \
    -static \
    @SQLITE_LDFLAGS@ \
    @LW_LDFLAGS@
