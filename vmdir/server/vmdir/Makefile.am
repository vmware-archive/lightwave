noinst_LTLIBRARIES = libvmdird.la

idl_srcdir=$(top_srcdir)/vmdir/idl

VMDIRD_BACKEND_STORE= $(top_builddir)/vmdir/server/mdb-store/libmdb-store.la

VMDIRD_REPLICATION_LIB = $(top_builddir)/vmdir/server/replication2/libreplication.la

libvmdird_la_SOURCES = \
    accountmgmt.c \
    auth.c \
    background.c \
    dbbackup.c \
    dbcrosschk.c \
    globals.c \
    heartbeat.c \
    index.c \
    init.c \
    instance.c \
    integritychk.c \
    integrityrpt.c \
    ipcserver.c \
    ipclocalapi.c \
    ipcapihandler.c \
    krb.c \
    metrics.c \
    parseargs.c \
    promote/dcactmgmt.c \
    regconfig.c \
    rpcserv.c \
    rpcsrpserv.c \
    rpcstring.c \
    rpc.c \
    schema.c \
    service.c \
    shutdown.c \
    signal.c \
    srp_verifier_sstub.c \
    srvthr.c \
    superlogging.c \
    tenantmgmt.c \
    tombstone.c \
    tracklastlogin.c \
    trusts.c \
    utils.c \
    vmdir_sstub.c \
    vmdirftp_sstub.c \
    vmdirdbcp_sstub.c \
    vmdirsuperlog_sstub.c \
    vmevent.c

libvmdird_la_CPPFLAGS = \
    -g \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/include \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/vmdir/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/vmdir/thirdparty \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmevent/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmdird_la_LIBADD = \
    $(top_builddir)/vmdir/server/vmkdc/libvmkdcserv.la \
    $(top_builddir)/vmdir/server/metrics/libmetrics.la \
    $(top_builddir)/vmdir/server/ldap-head/libldap-head.la \
    $(top_builddir)/vmdir/server/rest-head/librest-head.la \
    $(top_builddir)/vmdir/server/indexcfg/libindexcfg.la \
    $(top_builddir)/vmdir/server/dbintegritychk/libdb-integritychk.la \
    $(top_builddir)/vmdir/server/middle-layer/libmiddle-layer.la \
    $(top_builddir)/vmdir/server/schema/libschema.la \
    $(top_builddir)/vmdir/server/acl/libvmacl.la \
    $(top_builddir)/vmdir/server/backend/libbackend.la \
    $(VMDIRD_BACKEND_STORE) \
    $(VMDIRD_REPLICATION_LIB) \
    $(top_builddir)/vmdir/server/clusterstate/libclusterstate.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/server/common/libsrvcommon.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/vmdir/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/vmdir/thirdparty/csrp/libcsrp.la \
    $(top_builddir)/sts/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/sts/ssoclients/oidc/src/libssooidc.la \
    $(top_builddir)/vmcommon/libvmcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @GSSAPI_LIBS@ \
    @SASL_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @COPENAPI_LIBS@ \
    @CRESTENGINE_LIBS@

libvmdird_la_LDFLAGS = \
    -static

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
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirsuperlog_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/vmdir/include/public $<
