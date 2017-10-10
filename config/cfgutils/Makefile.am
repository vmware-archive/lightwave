lib_LTLIBRARIES = libcfgutils.la

libcfgutils_la_SOURCES = \
    certificate.c \
    cfgutils.c    \
    dns.c         \
    globals.c     \
    libmain.c     \
    logging.c     \
    memory.c      \
    service.c     \
    strings.c     \
    sysutils.c    \
    validate.c

libcfgutils_la_CPPFLAGS = \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

libcfgutils_la_LIBADD = \
    @top_builddir@/vmafd/client/libvmafdclient.la \
    @top_builddir@/vmca/client/libvmcaclient.la \
    @top_builddir@/vmdns/client/libvmdnsclient.la \
    @top_builddir@/vmdir/client/libvmdirclient.la \
    @LWSM_LIBS@ \
    @LWADVAPI_LIBS@ \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @CRYPTO_LIBS@

libcfgutils_la_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
