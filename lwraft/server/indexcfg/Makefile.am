
noinst_LTLIBRARIES = libindexcfg.la

libindexcfg_la_SOURCES = \
    api.c \
    globals.c \
    indexcfg.c \
    indexingtask.c \
    indexingthr.c \
    indexupd.c \
    libmain.c \
    progress.c

libindexcfg_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libindexcfg_la_LDFLAGS = \
    -static
