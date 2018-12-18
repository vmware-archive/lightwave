noinst_LTLIBRARIES = libvmcasecurity.la

libvmcasecurity_la_SOURCES = \
    globals.c \
    security.c

libvmcasecurity_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/plugins/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libvmcasecurity_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/vmca/service/plugin/libvmcasrvplugin.la \
    @LWBASE_LIBS@ \
    @DCERPC_LIBS@ \
    @DL_LIBS@

libvmcasecurity_la_LDFLAGS = \
    @LW_LDFLAGS@
