sbin_PROGRAMS = vmdnsd

idl_srcdir=$(top_srcdir)/vmdns/idl
include_public_srcdir=$(top_srcdir)/vmdns/include/public

vmdnsd_SOURCES = \
    auth.c        \
    globals.c     \
    init.c        \
    main.c        \
    metrics.c     \
    parseargs.c   \
    rpc.c         \
    rpcserv.c     \
    service.c     \
    signal.c      \
    shutdown.c    \
    utils.c       \
    vmdns_sstub.c

vmdnsd_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdns/server/common \
    -I$(top_srcdir)/vmdns/server/include \
    -I$(top_srcdir)/vmdns/server/service \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmmetrics/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

vmdnsd_LDADD = \
    $(top_builddir)/vmdns/server/common/libsrvcommon.la \
    $(top_builddir)/vmdns/server/rest-head/librest-head.la \
    $(top_builddir)/vmdns/common/libcommon.la \
    $(top_builddir)/vmdns/vmsock/api/libvmsock.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmidentity/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
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
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @COPENAPI_LIBS@ \
    @CRESTENGINE_LIBS@

vmdnsd_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    @COPENAPI_LDFLAGS@ \
    @CRESTENGINE_LDFLAGS@

CLEANFILES = \
    vmdns_h.h \
    vmdns_cstub.c \
    vmdns_sstub.c

BUILT_SOURCES = vmdns_h.h

vmdns_h.h vmdns_sstub.c: $(idl_srcdir)/vmdns.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdns_h.h -I$(idl_srcdir) -I$(include_public_srcdir) $<
