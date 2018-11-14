bin_PROGRAMS = testmutentcapolicy

check_PROGRAMS = testmutentcapolicy
TESTS = testmutentcapolicy

testmutentcapolicy_SOURCES = \
    main.c \
    parse.c \
    util.c \
    validate.c

testmutentcapolicy_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    @LW_INCLUDES@ \
    @JANSSON_INCLUDES@

testmutentcapolicy_LDADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/policy/libmutentcapolicy.la \
    @CMOCKA_LIBS@

testmutentcapolicy_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
