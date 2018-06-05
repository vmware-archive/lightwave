
noinst_LTLIBRARIES = libreplication.la

libreplication_la_SOURCES = \
    drrestore.c \
    libmain.c \
    firstreplcycle.c \
    metricsinit.c \
    replentry.c \
    thread.c \
    raftldaprpc.c

libreplication_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmmetrics/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libreplication_la_LDFLAGS = \
    -static
