bin_PROGRAMS = dnstest

idl_srcdir=$(top_srcdir)/vmdns/idl
include_public_srcdir=$(top_srcdir)/vmdns/include/public

dnstest_SOURCES =       \
    AllTests.c          \
    CuTest.c            \
    LruListTest.c       \
    test-nameentry.c    \
    test-recordlist.c   \
    TestUtils.c         \
    TestPtr.c

dnstest_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdns/common/include \
    -I$(top_srcdir)/vmdns/server/include \
    -I$(top_srcdir)/vmdns/server/common \
    -I$(top_srcdir)/vmdns/build/server/service \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmmetrics/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

dnstest_LDADD = \
    $(top_builddir)/vmdns/server/common/libsrvcommon.la \
    $(top_builddir)/vmdns/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmmetrics/libvmmetrics.la \
    @SCHANNEL_LIBS@ \
    @DCERPC_LIBS@ \
    @LDAP_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

dnstest_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
