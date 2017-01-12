
noinst_LTLIBRARIES = libindexcfg.la

libindexcfg_la_SOURCES = \
    api.c \
    globals.c \
    indexcfg.c \
    indexingtask.c \
    indexingthr.c \
    indexupd.c \
    libmain.c \
    progress.c \
    vmit.c

libindexcfg_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libindexcfg_la_LDFLAGS = \
    -static

