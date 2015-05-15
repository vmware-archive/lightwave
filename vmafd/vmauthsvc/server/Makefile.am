noinst_LTLIBRARIES = libvmauthsvcserver.la

idl_srcdir=$(top_srcdir)/vmauthsvc/idl

libvmauthsvcserver_la_SOURCES = \
    globals.c     \
    init.c        \
    parseargs.c   \
    regconfig.c   \
    rpc.c         \
    rpcmemory.c   \
    rpcserv.c     \
    rpcstring.c   \
    service.c     \
    shutdown.c    \
    signal.c      \
    utils.c       \
    vmauthsvc_sstub.c

libvmauthsvcserver_la_CPPFLAGS = \
    -I$(top_srcdir)/vmauthsvc/include \
    -I$(top_srcdir)/vmauthsvc/include/public \
    -I$(top_srcdir)/vmauthsvc/server/include \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmauthsvcserver_la_LIBADD = \
    $(top_builddir)/vmauthsvc/common/libvmauthsvccommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

libvmauthsvcserver_la_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmauthsvc_h.h \
    vmauthsvc_cstub.c \
    vmauthsvc_sstub.c

BUILT_SOURCES = vmauthsvc_h.h

vmauthsvc_h.h vmauthsvc_sstub.c: $(idl_srcdir)/vmauthsvc.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmauthsvc_h.h -I$(idl_srcdir) -I$(top_srcdir)/vmauthsvc/include/public $<
