noinst_LTLIBRARIES = libvmcasrvplugin.la

libvmcasrvplugin_la_SOURCES = \
    plugin.c

libvmcasrvplugin_la_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmca/service/common \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libvmcasrvplugin_la_LIBADD = \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmca/service/common/libvmcasrvcommon.la \
    @DL_LIBS@

libvmcasrvplugin_la_LDFLAGS = \
    -static
