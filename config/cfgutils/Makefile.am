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
    -I$(top_srcdir)/include \
    @VMCA_INCLUDES@ \
    @VMAFD_INCLUDES@ \
    @VMDIR_INCLUDES@ \
    @VMDNS_INCLUDES@ \
    @LW_INCLUDES@

libcfgutils_la_LIBADD = \
    @VMDNS_LIBS@ \
    @VMCA_LIBS@ \
    @VMAFD_LIBS@ \
    @VMDIR_LIBS@ \
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
    @VMDNS_LDFLAGS@ \
    @VMCA_LDFLAGS@ \
    @VMAFD_LDFLAGS@ \
    @VMDIR_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@

