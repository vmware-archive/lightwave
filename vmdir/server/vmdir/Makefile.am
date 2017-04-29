sbin_PROGRAMS = vmdird

idl_srcdir=$(top_srcdir)/idl

vmdird_SOURCES = \
    accountmgmt.c \
    auth.c \
    globals.c \
    heartbeat.c \
    index.c \
    init.c \
    instance.c \
    integritychk.c \
    ipcserver.c \
    ipclocalapi.c \
    ipcapihandler.c \
    parseargs.c \
    main.c \
    regconfig.c \
    rpcserv.c \
    rpcsrpserv.c \
    rpcstring.c \
    rpc.c \
    schema.c \
    service.c \
    shutdown.c \
    signal.c \
    srvthr.c \
    superlogging.c \
    utils.c \
    krb.c \
    vmdir_sstub.c \
    vmdirftp_sstub.c \
    vmdirdbcp_sstub.c \
    srp_verifier_sstub.c \
    vmdirsuperlog_sstub.c \
    vmevent.c \
    tenantmgmt.c \
    tombstone.c \
    tracklastlogin.c

VMDIRD_BACKEND_INCLUDES=
VMDIRD_BACKEND_LD_FLAGS=
VMDIRD_BACKEND_STORE= $(top_builddir)/server/mdb-store/libmdb-store.la

vmdird_CPPFLAGS = \
    -g \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/thirdparty \
    -I$(top_srcdir)/server/include \
    $(VMDIRD_BACKEND_INCLUDES) \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @VMEVENT_INCLUDES@

vmdird_LDADD = \
    $(top_builddir)/server/vmkdc/libvmkdcserv.la \
    $(top_builddir)/server/ldap-head/libldap-head.la \
    $(top_builddir)/server/indexcfg/libindexcfg.la \
    $(top_builddir)/server/middle-layer/libmiddle-layer.la \
    $(top_builddir)/server/schema/libschema.la \
    $(top_builddir)/server/acl/libvmacl.la \
    $(top_builddir)/server/backend/libbackend.la \
    $(VMDIRD_BACKEND_STORE) \
    $(top_builddir)/server/replication/libreplication.la \
    $(top_builddir)/client/libvmdirclient.la \
    $(top_builddir)/server/common/libsrvcommon.la \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/thirdparty/csrp/libcsrp.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@

vmdird_LDFLAGS = \
    -rdynamic -ldl \
    $(VMDIRD_BACKEND_LD_FLAGS) \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmdir_h.h \
    vmdir_cstub.c \
    vmdir_sstub.c \
    vmdirftp_h.h \
    vmdirdbcp_h.h \
    vmdirftp_cstub.c \
    vmdirdbcp_cstub.c \
    vmdirftp_sstub.c \
    vmdirdbcp_sstub.c \
    srp_verifier_h.h \
    srp_verifier_cstub.c \
    srp_verifier_sstub.c \
    vmdirsuperlog_h.h \
    vmdirsuperlog_cstub.c \
    vmdirsuperlog_sstrub.c

BUILT_SOURCES = vmdir_h.h vmdirftp_h.h vmdirdbcp_h.h srp_verifier_h.h vmdirsuperlog_h.h

vmdir_h.h vmdir_sstub.c: $(idl_srcdir)/vmdir.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdir_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirftp_h.h vmdirftp_sstub.c: $(idl_srcdir)/vmdirftp.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirftp_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirdbcp_h.h vmdirdbcp_sstub.c: $(idl_srcdir)/vmdirdbcp.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirdbcp_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) $<

srp_verifier_h.h srp_verifier_sstub.c: $(idl_srcdir)/srp_verifier.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header srp_verifier_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirsuperlog_h.h vmdirsuperlog_sstub.c: $(idl_srcdir)/vmdirsuperlog.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirsuperlog_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/include/public $<
