
noinst_LTLIBRARIES = libreplication.la

libreplication_la_SOURCES = \
    deadlockdetectionvector.c \
    drrestore.c \
    firstreplcycle.c \
    globals.c \
    libmain.c \
    metrics.c \
    replentry.c \
    replqueue.c \
    thread.c \
    utdvector.c

libreplication_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libreplication_la_LDFLAGS = \
    -static
