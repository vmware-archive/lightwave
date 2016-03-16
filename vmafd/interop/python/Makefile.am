pymodules_LTLIBRARIES = vmafd.la

pymodules_SCRIPTS = \
    __init__.py \
    cdcclient.py \
    vmkeystore.py

vmafd_la_SOURCES = \
    vmafd-python.cpp  \
    afdclient.cpp

vmafd_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/common \
    -I$(top_srcdir)/interop/python \
    @BOOST_INCLUDES@ \
    @LW_INCLUDES@ \
    @VMDIR_INCLUDES@ \
    @PYTHON_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmafd_la_LIBADD = \
    @BOOST_PYTHON_LDFLAGS@ \
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

vmafd_la_LDFLAGS = \
    @BOOST_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@  \
    @PYTHON_LDFLAGS@ \
    -module
