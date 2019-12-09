bin_PROGRAMS = testmutentcasrvcommon

check_PROGRAMS = testmutentcasrvcommon
TESTS = testmutentcasrvcommon

testmutentcasrvcommon_SOURCES = \
    init.c \
    jsonutils.c \
    main.c \
    mock.c \
    token_util.c \
    utils.c

testmutentcasrvcommon_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    -I$(top_srcdir)/mutentca/server/config \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/sts/ssoclients/oidc/include/public \
    -I$(top_srcdir)/sts/ssoclients/common/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

testmutentcasrvcommon_LDADD = \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/config/libmutentcasrvconfig.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CMOCKA_LIBS@

testmutentcasrvcommon_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    -Wl,--wrap,LwCAGetVecsMutentCACert \
    -Wl,--wrap,OidcClientBuild \
    -Wl,--wrap,OidcClientAcquireTokensBySolutionUserCredentials
