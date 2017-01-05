sbin_PROGRAMS = vmdnsd

idl_srcdir=$(top_srcdir)/idl
include_srcdir=$(top_srcdir)/include
include_public_srcdir=$(top_srcdir)/include/public

vmdnsd_SOURCES = \
    auth.c        \
    globals.c     \
    init.c        \
    main.c        \
    parseargs.c   \
    rpc.c         \
    rpcserv.c     \
    service.c     \
    signal.c      \
    shutdown.c    \
    utils.c       \
    vmdns_sstub.c

vmdnsd_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/common \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server/service \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

vmdnsd_LDADD = \
    $(top_builddir)/server/common/libsrvcommon.la \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/vmsock/api/libvmsock.la \
    @VMDIR_LIBS@ \
    @DCERPC_LIBS@ \
    @LDAP_LIBS@ \
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

vmdnsd_LDFLAGS = \
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmdns_h.h \
    vmdns_cstub.c \
    vmdns_sstub.c

BUILT_SOURCES = vmdns_h.h

vmdns_h.h vmdns_sstub.c: $(idl_srcdir)/vmdns.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdns_h.h -I$(idl_srcdir) -I$(include_public_srcdir) $<
