bin_PROGRAMS = testmutentcasrvplugin

check_PROGRAMS = testmutentcasrvplugin
TESTS = testmutentcasrvplugin

SUBDIRS = \
    test-plugins

testmutentcasrvplugin_SOURCES = \
    main.c \
    plugin.c

testmutentcasrvplugin_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    -I$(top_srcdir)/mutentca/server/plugin \
    -I$(top_srcdir)/mutentca/tests/unit-tests/server/plugin/test-plugins \
    -I$(top_srcdir)/vmafd/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

testmutentcasrvplugin_LDADD = \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/plugin/libmutentcasrvplugin.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @DL_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CMOCKA_LIBS@

testmutentcasrvplugin_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
