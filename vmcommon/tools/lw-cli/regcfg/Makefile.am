noinst_LTLIBRARIES = libregcfgcli.la

libregcfgcli_la_SOURCES = \
    regcfgcli.c

libregcfgcli_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmcommon/include \
    @LW_INCLUDES@

libregcfgcli_la_LIBADD = \
    @top_builddir@/vmcommon/libvmcommon.la

libregcfgcli_la_LDFLAGS = \
    -static \
    @LW_LDFLAGS@
