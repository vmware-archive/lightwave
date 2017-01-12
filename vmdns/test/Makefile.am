bin_PROGRAMS = dnstest

idl_srcdir=$(top_srcdir)/idl
include_srcdir=$(top_srcdir)/include
include_public_srcdir=$(top_srcdir)/include/public

dnstest_SOURCES =       \
    AllTests.c          \
    CuTest.c            \
    LruListTest.c       \
    test-nameentry.c    \
    test-recordlist.c   \
    TestUtils.c

dnstest_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/common/include \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server/common \
    -I$(top_srcdir)/build/server/service \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

dnstest_LDADD = \
    $(top_builddir)/server/common/libsrvcommon.la \
    $(top_builddir)/common/libcommon.la \
    @SCHANNEL_LIBS@ \
    @VMDIR_LIBS@ \
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
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
