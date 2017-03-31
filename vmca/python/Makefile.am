lib_LTLIBRARIES = vmca.la

vmca_la_SOURCES = \
    vmca-python.cpp  \
    certclient.cpp

    

vmca_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/common \
    -I$(top_srcdir)/python \
    @BOOST_INCLUDES@ \
    @LW_INCLUDES@ \
    @PYTHON_INCLUDES@    

vmca_la_LIBADD = \
    @BOOST_LIB_PATH@/libboost_python-gcc41-mt-1_55.a\
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmcaclient.la \
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
    
 
vmca_la_LDFLAGS = \
    @VMAFD_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@  \
    @PYTHON_LDFLAGS@ \
    -module 
    
    



