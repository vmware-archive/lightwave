sbin_PROGRAMS = vmafdd

idl_srcdir=$(top_srcdir)/idl

vmafdd_SOURCES = \
    ad.c          \
		authservice.c \
		authutil.c    \
    certutil.c    \
    config.c      \
    globals.c     \
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
    serviceapi.c  \
    signal.c      \
    shutdown.c    \
		storehash_util.c \
    utils.c       \
    vecsserviceapi.c \
    vmdir.c       \
    vmafd_sstub.c

vmafdd_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    @VMDIR_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmafdd_LDADD = \
    $(top_builddir)/server/db/libvecsdb.la \
    $(top_builddir)/vmafcfg/api/libvmafcfgapi.la \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/vmevent/server/libvmeventserver.la \
    $(top_builddir)/vmauthsvc/server/libvmauthsvcserver.la \
    @VMDIR_LIBS@ \
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
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmafd_h.h \
    vmafd_cstub.c \
    vmafd_sstub.c

BUILT_SOURCES = vmafd_h.h

vmafd_h.h vmafd_sstub.c: $(idl_srcdir)/vmafd.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd_h.h -I$(idl_srcdir) -I$(top_srcdir)/include/public $<
