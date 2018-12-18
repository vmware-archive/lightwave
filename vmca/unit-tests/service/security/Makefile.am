bin_PROGRAMS = testvmcasecurity

check_PROGRAMS = testvmcasecurity
TESTS = testvmcasecurity

SUBDIRS = \
    test-vmcasecurity-config \
    test-vmcasecurity-plugin

testvmcasecurity_SOURCES = \
    security_global.c \
    security.c \
    main.c

testvmcasecurity_CPPFLAGS = \
    -I$(top_srcdir)/plugins/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

testvmcasecurity_LDADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/vmca/service/plugin/libvmcasrvplugin.la \
    $(top_builddir)/vmca/service/security/libvmcasecurity.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CRYPTO_LIBS@ \
    @CMOCKA_LIBS@

testvmcasecurity_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
