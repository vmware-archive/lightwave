lib_LTLIBRARIES = liblwraftclient.la

idl_srcdir=$(top_srcdir)/idl

liblwraftclient_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @OPENSSL_INCLUDES@ \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

liblwraftclient_la_SOURCES = \
    binding.c \
    client.c \
    globals.c \
    groups.c \
    krb5login.c \
    ldaputil.c \
    libmain.c \
    registry.c \
    repadmin.c \
    replication.c \
    rpc.c \
    setupldu.c \
    shell.c \
    superlogutil.c \
    users.c \
    util.c \
    vmdir_cstub.c \
    vmdirftp_cstub.c \
    vmdirdbcp_cstub.c \
    vmdirlocalclient.c \
    srp_verifier_cstub.c \
    vmdirsuperlog_cstub.c \
    vmdirurgentrepl_cstub.c \
    vmdirraft_cstub.c

liblwraftclient_la_LIBADD = \
    @top_builddir@/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @LBER_LIBS@ \
    @CRYPTO_LIBS@ \
    @PTHREAD_LIBS@

liblwraftclient_la_LDFLAGS = \
    -export-symbols @top_srcdir@/client/liblwraftclient.exp \
    @DCERPC_LDFLAGS@ \
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
    vmdirsuperlog_sstub.c \
    vmdirurgentrepl_h.h \
    vmdirurgentrepl_cstub.c \
    vmdirurgentrepl_sstub.c \
    vmdirsuperlog_sstub.c \
    vmdirraft_sstub.c

BUILT_SOURCES = vmdir_h.h vmdirftp_h.h vmdirdbcp_h.h srp_verifier_h.h vmdirsuperlog_h.h vmdirurgentrepl_h.h vmdirraft_h.h

vmdir_h.h vmdir_cstub.c: $(idl_srcdir)/vmdir.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdir_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirftp_h.h vmdirftp_cstub.c: $(idl_srcdir)/vmdirftp.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirftp_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirdbcp_h.h vmdirdbcp_cstub.c: $(idl_srcdir)/vmdirdbcp.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirdbcp_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

srp_verifier_h.h srp_verifier_cstub.c: $(idl_srcdir)/srp_verifier.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header srp_verifier_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirsuperlog_h.h vmdirsuperlog_cstub.c: $(idl_srcdir)/vmdirsuperlog.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirsuperlog_h.h -sstub_pref Srv_ -I$(idl_srcdir) -I$(top_srcdir)/include/public $<

vmdirurgentrepl_h.h vmdirurgentrepl_cstub.c: $(idl_srcdir)/vmdirurgentrepl.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirurgentrepl_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<

vmdirraft_h.h vmdirraft_cstub.c: $(idl_srcdir)/vmdirraft.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdirraft_h.h -sstub_pref Srv_ -I$(idl_srcdir) $<
