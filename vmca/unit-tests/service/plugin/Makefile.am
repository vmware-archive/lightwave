bin_PROGRAMS = vmcasrvplugin

check_PROGRAMS = vmcasrvplugin
TESTS = vmcasrvplugin

SUBDIRS = \
    test-plugins

vmcasrvplugin_SOURCES = \
    main.c \
    plugin.c

vmcasrvplugin_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmca/service/common \
    -I$(top_srcdir)/vmca/service/plugin \
    -I$(top_srcdir)/vmca/unit-tests/service/plugin/test-plugins \
    -I$(top_srcdir)/vmafd/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

vmcasrvplugin_LDADD = \
    $(top_builddir)/vmca/service/common/libvmcasrvcommon.la \
    $(top_builddir)/vmca/service/plugin/libvmcasrvplugin.la \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @DL_LIBS@ \
    @DCERPC_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CMOCKA_LIBS@

vmcasrvplugin_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
