bin_PROGRAMS = testmutentcasecurity

check_PROGRAMS = testmutentcasecurity
TESTS = testmutentcasecurity

SUBDIRS = \
    test-mutentcasecurity-config \
    test-mutentcasecurity-plugin

testmutentcasecurity_SOURCES = \
    security_global.c \
    security.c \
    main.c

testmutentcasecurity_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/plugins/include/public \
    @LW_INCLUDES@

testmutentcasecurity_LDADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/security/libmutentcasecurity.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CRYPTO_LIBS@ \
    @CMOCKA_LIBS@

testmutentcasecurity_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
