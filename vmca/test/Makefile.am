noinst_PROGRAMS = vmca_test vmca_dlopen

vmca_dlopen_SOURCES = \
	testdl.c
vmca_dlopen_LDFLAGS = \
	-ldl

vmca_test_SOURCES = \
	VmcaUnitTest.cpp \
  	testcases.cpp \
    vmcaOpenSsl.cpp \
    posix_test.cpp \
    API_test.cpp

vmca_test_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/idl \
    -I$(top_srcdir)/vmca/common \
    -I$(top_srcdir)/vmca/test \
    @BOOST_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @LW_INCLUDES@

vmca_test_LDADD = \
    ${GOBUILD_BOOST1470_LIN64_GCC412_ROOT}/lib/libboost_unit_test_framework-gcc41-mt-1_47.a \
    ${GOBUILD_BOOST1470_LIN64_GCC412_ROOT}/lib/libboost_chrono-gcc41-mt-1_47.a \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmca/client/libvmcaclient.la \
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
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@

vmca_test_LDFLAGS = \
    @BOOST_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    -ggdb 
