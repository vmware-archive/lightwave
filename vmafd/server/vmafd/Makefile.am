sbin_PROGRAMS = vmafdd

idl_srcdir=$(top_srcdir)/vmafd/idl

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
    dns.c         \
    globals.c     \
    heartbeat.c     \
    ha_upgrade.c     \
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
    rest.c        \
    restcerts-lwca.c   \
    restcerts-vmca.c   \
    restcerts.c   \
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
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmafd/server/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmcommon/include \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmafdd_LDADD = \
    $(top_builddir)/vmafd/server/db/libvecsdb.la \
    $(top_builddir)/vmafd/vmafcfg/api/libvmafcfgapi.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmafd/vmevent/server/libvmeventserver.la \
    $(top_builddir)/vmafd/common/libcommon.la \
    $(top_builddir)/vmafd/vmnetevent/libvmnetevent.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdns/client/libvmdnsclient.la \
    $(top_builddir)/vmcommon/libvmcommon.la \
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
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd_h.h -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/vmafd/include/public $<

vmafd60_h.h vmafd60_sstub.c: $(idl_srcdir)/vmafd60.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd60_h.h -sstub_pref Srv60_ -I$(idl_srcdir) -I$(top_srcdir)/vmafd/include/public $<

vmafdsuperlog_h.h vmafdsuperlog_cstub.c: $(idl_srcdir)/vmafdsuperlog.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafdsuperlog_h.h -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/vmafd/include/public $<
