noinst_LTLIBRARIES = libvmcasrvconfig.la

libvmcasrvconfig_la_SOURCES = \
    config.c

libvmcasrvconfig_la_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmca/service/common \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @JANSSON_INCLUDES@

libvmcasrvconfig_la_LIBADD = \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmca/service/common/libvmcasrvcommon.la \
    @JANSSON_LIBS@

libvmcasrvconfig_la_LDFLAGS = \
    -static
