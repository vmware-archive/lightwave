noinst_PROGRAMS = vecs_test vecs_dlopen

vecs_dlopen_SOURCES = \
	testdl.c

vecs_dlopen_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/test \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vecs_dlopen_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmafdclient.la \
    @DL_LIBS@  \
    @PTHREAD_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LWBASE_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @CRYPTO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWIO_LIBS@ \
    @DCERPC_LIBS@ \
    @UUID_LIBS@

vecs_dlopen_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -ggdb

vecs_test_SOURCES = \
    vecstest.cpp

vecs_test_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/test \
    @BOOST_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

vecs_test_LDADD = \
    ${GOBUILD_BOOST1470_LIN64_GCC412_ROOT}/lib/libboost_unit_test_framework-gcc41-mt-1_47.a \
    ${GOBUILD_BOOST1470_LIN64_GCC412_ROOT}/lib/libboost_chrono-gcc41-mt-1_47.a \
    ${GOBUILD_BOOST1470_LIN64_GCC412_ROOT}/lib/libboost_thread-gcc41-mt-1_47.a  \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmafdclient.la \
    @BOOST_LIBS@  \
    @DL_LIBS@  \
    @PTHREAD_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LWBASE_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @CRYPTO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWIO_LIBS@ \
    @DCERPC_LIBS@ \
    @LDAP_LIBS@ \
    @UUID_LIBS@

vecs_test_LDFLAGS = \
    @BOOST_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -ggdb

