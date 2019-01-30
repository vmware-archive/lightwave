sbin_PROGRAMS = postmd

postmd_SOURCES = \
    signal.c \
    process.c \
    processtable.c \
    ipcapihandler.c \
    ipclocalapi.c \
    ipcserver.c \
    globals.c \
    main.c

postmd_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

postmd_LDADD = \
    $(top_builddir)/lwraft/common/libcommon.la \
    @PTHREAD_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LDAP_LIBS@

postmd_LDFLAGS = \
    -rdynamic -ldl \
    @LW_LDFLAGS@
