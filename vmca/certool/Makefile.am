bin_PROGRAMS = certool

certool_SOURCES = \
    certool.cpp  \
    Handler.cpp  \
    utility.cpp

certool_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/idl \
    -I$(top_srcdir)/vmca/common \
    -I$(top_srcdir)/vmca/certool \
    -I$(top_srcdir)/vmafd/include/public \
    @BOOST_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

certool_LDADD = \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmca/client/libvmcaclient.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @DL_LIBS@  \
    @BOOST_LIBS@ \
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

certool_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @BOOST_LDFLAGS@ \
    @LW_LDFLAGS@
