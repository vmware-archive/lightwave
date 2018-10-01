bin_PROGRAMS = security_test

check_PROGRAMS = security_test
TESTS = security_test

security_test_CPPFLAGS = \
    -I$(top_srcdir)/plugins/unit-tests/security \
    -I$(top_srcdir)/plugins/include/public \
    @LW_INCLUDES@

security_test_SOURCES = \
    main.c \
    security_test_global.c \
    security_test_check_version.c \
    security_test_validate_interface.c \
    security_test_caps.c

security_test_LDADD = \
    @CMOCKA_LIBS@

security_test_LDFLAGS = \
    -ldl

#test library implementation
pkglib_LTLIBRARIES = security_plugin_test.la

security_plugin_test_la_CPPFLAGS = \
    -I$(top_srcdir)/plugins/unit-tests/security \
    -I$(top_srcdir)/plugins/include/public \
    @LW_INCLUDES@

security_plugin_test_la_SOURCES = \
    security_plugin_test_impl.c

security_plugin_test_la_LDFLAGS = \
    -avoid-version \
    -module
