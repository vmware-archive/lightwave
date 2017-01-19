sbin_PROGRAMS = vmafdd

idl_srcdir=$(top_srcdir)/idl

vmafdd_SOURCES = \
    ad.c          \
    authservice.c \
    authutil.c    \
    certutil.c    \
    circularbuffer.c    \
    config.c      \
    cdcservice.c \
    cdcstatemachine.c \
    cdcthread.c \
    cdcupdate.c \
    dcfinder.c    \
    ddns.c	\
    ddnspackets.c \
    dns.c         \
    globals.c     \
    heartbeat.c     \
    ipcapihandler.c \
    ipclocalapi.c \
    ipcmarshalhelper.c \
    ipcserver.c   \
    init.c        \
    krbconfig.c   \
    ldap.c   \
    main.c        \
    parseargs.c   \
    passrefresh.c \
    regconfig.c   \
    rootfetch.c   \
    rpc.c         \
    rpcserv.c     \
    service.c     \
    signal.c      \
    shutdown.c    \
    sourceip.c	  \
    storehash_util.c \
    superlogging.c \
    utils.c       \
    vecsserviceapi.c \
    vmdir.c       \
    rpcserv60.c   \
    vmafd_sstub.c \
    vmafd60_sstub.c \
    vmafdsuperlog_sstub.c

vmafdd_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    @VMDIR_INCLUDES@ \
    @VMDNS_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmafdd_LDADD = \
    $(top_builddir)/server/db/libvecsdb.la \
    $(top_builddir)/vmafcfg/api/libvmafcfgapi.la \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/vmevent/server/libvmeventserver.la \
    $(top_builddir)/common/libcommon.la \
    @VMDIR_LIBS@ \
    @VMDNS_LIBS@ \
    @DCERPC_LIBS@ \
    @SCHANNEL_LIBS@ \
    @DOMAINJOIN_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWIO_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

vmafdd_LDFLAGS = \
    @VMDIR_LDFLAGS@ \
    @VMDNS_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmafd_h.h \
    vmafd60_h.h \
    vmafdsuperlog_h.h \
    vmafd_cstub.c \
    vmafd60_sstub.c \
    vmafdsuperlog_cstub.c \
    vmafdsuperlog_sstub.c

BUILT_SOURCES = vmafd_h.h vmafd60_h.h vmafdsuperlog_h.h

vmafd_h.h vmafd_sstub.c: $(idl_srcdir)/vmafd.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd_h.h -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/include/public $<

vmafd60_h.h vmafd60_sstub.c: $(idl_srcdir)/vmafd60.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd60_h.h -sstub_pref Srv60_ -I$(idl_srcdir) -I$(top_srcdir)/include/public $<

vmafdsuperlog_h.h vmafdsuperlog_cstub.c: $(idl_srcdir)/vmafdsuperlog.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafdsuperlog_h.h -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/include/public $<
