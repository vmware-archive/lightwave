noinst_LTLIBRARIES = libvmregconfig.la

libvmregconfig_la_SOURCES = \
    globals.c \
    vmregconfig.c \
    vmregcfgwrapper.c \
    readinternal.c \
    writeinternal.c \
    util.c

libvmregconfig_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@

libvmregconfig_la_LDFLAGS = \
    -export-symbols @top_srcdir@/vmcommon/vmregconfig/libvmregconfig.exp
