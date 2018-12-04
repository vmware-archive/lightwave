
noinst_LTLIBRARIES = libwatchnotify.la

libwatchnotify_la_SOURCES = \
    event.c \
    eventrepo.c \
    notifythread.c \
    sessionmanagerthread.c \
    watchsession.c \
    watchsessionmanager.c

libwatchnotify_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libwatchnotify_la_LDFLAGS = \
    -static
