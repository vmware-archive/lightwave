bin_PROGRAMS = testmutentcaauthz

check_PROGRAMS = testmutentcaauthz
TESTS = testmutentcaauthz

testmutentcaauthz_SOURCES = \
    authz.c \
	lightwave-authz.c \
    main.c

testmutentcaauthz_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

testmutentcaauthz_LDADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/authz/libmutentcaauthz.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CRYPTO_LIBS@ \
    @CMOCKA_LIBS@

testmutentcaauthz_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
