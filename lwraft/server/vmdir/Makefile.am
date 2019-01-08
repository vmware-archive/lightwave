sbin_PROGRAMS = postd

idl_srcdir=$(top_srcdir)/lwraft/idl

postd_SOURCES = \
    accountmgmt.c \
    auth.c \
    background.c \
    dbbackup.c \
    globals.c \
    index.c \
    init.c \
    instance.c \
    ipcapihandler.c \
    ipclocalapi.c \
    ipcserver.c \
    krb.c \
    main.c \
    metrics.c \
    parseargs.c \
    regconfig.c \
    rpc.c \
    rpcserv.c \
    rpcstring.c \
    schema.c \
    service.c \
    shutdown.c \
    signal.c \
    srvthr.c \
    superlogging.c \
    tenantmgmt.c \
    tracklastlogin.c \
    utils.c \
    vmevent.c \
    vmdir_sstub.c \
    vmdirdbcp_sstub.c \
    vmdirsuperlog_sstub.c \
    vmdirraft_sstub.c

VMDIRD_BACKEND_INCLUDES=
VMDIRD_BACKEND_LD_FLAGS=
VMDIRD_BACKEND_STORE= $(top_builddir)/lwraft/server/mdb-store/libmdb-store.la

postd_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/thirdparty \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    -I$(top_srcdir)/vmevent/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    $(VMDIRD_BACKEND_INCLUDES) \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

if BUILD_GRPC
    GRPC_HEAD = $(top_builddir)/lwraft/server/grpc-head/libdomath-grpc.la \
                $(top_builddir)/lwraft/server/grpc-head/postwatch/libpostwatch.la
endif

postd_LDADD = \
    $(top_builddir)/lwraft/server/metrics/libmetrics.la \
    $(top_builddir)/lwraft/server/ldap-head/libldap-head.la \
    $(top_builddir)/lwraft/server/rest-head/librest-head.la \
    $(top_builddir)/lwraft/server/indexcfg/libindexcfg.la \
    $(top_builddir)/lwraft/server/middle-layer/libmiddle-layer.la \
    $(top_builddir)/lwraft/server/schema/libschema.la \
    $(top_builddir)/lwraft/server/acl/libvmacl.la \
    $(top_builddir)/lwraft/server/backend/libbackend.la \
    $(top_builddir)/lwraft/server/watchnotify/libwatchnotify.la \
    $(VMDIRD_BACKEND_STORE) \
    $(top_builddir)/lwraft/server/replication/libreplication.la \
    $(top_builddir)/lwraft/client/libpostclient.la \
    $(top_builddir)/lwraft/server/common/libsrvcommon.la \
    $(top_builddir)/lwraft/common/libcommon.la \
    $(top_builddir)/lwraft/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    $(top_builddir)/lwraft/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/lwraft/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/lwraft/thirdparty/csrp/libcsrp.la \
    $(top_builddir)/vmidentity/ssoclients/common/src/libssocommon.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(GRPC_HEAD) \
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
    @GRPC_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @COPENAPI_LIBS@ \
    @CRESTENGINE_LIBS@

postd_LDFLAGS = \
    -rdynamic -ldl \
    $(VMDIRD_BACKEND_LD_FLAGS) \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    @COPENAPI_LDFLAGS@ \
    @CRESTENGINE_LDFLAGS@ \
    $(GRPC_LIBS)

CLEANFILES = \
    vmdir_h.h \
    vmdir_cstub.c \
    vmdir_sstub.c \
    vmdirdbcp_h.h \
    vmdirdbcp_cstub.c \
    vmdirdbcp_sstub.c \
    vmdirsuperlog_h.h \
    vmdirsuperlog_cstub.c \
    vmdirsuperlog_sstrub.c \
    vmdirraft_h.h \
    vmdirraft_cstub.c \
    vmdirraft_sstub.c

BUILT_SOURCES = vmdir_h.h vmdirdbcp_h.h vmdirsuperlog_h.h vmdirraft_h.h

vmdir_h.h vmdir_sstub.c: $(idl_srcdir)/vmdir.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdir_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirdbcp_h.h vmdirdbcp_sstub.c: $(idl_srcdir)/vmdirdbcp.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirdbcp_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirsuperlog_h.h vmdirsuperlog_sstub.c: $(idl_srcdir)/vmdirsuperlog.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirsuperlog_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/lwraft/include/public $<

vmdirraft_h.h vmdirraft_sstub.c: $(idl_srcdir)/vmdirraft.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirraft_h.h -cstub_pref Cli_ -sstub_pref Srv_ -I$(idl_srcdir) $<
