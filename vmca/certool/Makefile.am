bin_PROGRAMS = certool

certool_SOURCES = \
    certool.cpp  \
    Handler.cpp  \
    utility.cpp

certool_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/idl \
    -I$(top_srcdir)/common \
    -I$(top_srcdir)/certool \
    @BOOST_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @VMAFD_INCLUDES@


certool_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmcaclient.la \
    @VMAFD_LIBS@ \
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
    @VMAFD_LDFLAGS@ \
    @BOOST_LDFLAGS@ \
    @LW_LDFLAGS@





