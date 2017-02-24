sbin_PROGRAMS = vmcad

idl_srcdir=$(top_srcdir)/idl

vmcad_SOURCES =         \
    auth.c              \
    dirsync.c           \
    entrypoints.c       \
    errormap.c          \
    globals.c           \
    init.c              \
    main.c              \
    rpc.c               \
    rpcmemory.c         \
    rpcserv.c           \
    service.c           \
    signal.c            \
    state.c             \
    thread.c            \
    utils.c             \
    vmcaservice.c       \
    vmca_sstub.c

vmcad_CPPFLAGS =                    \
    -I$(top_srcdir)/include         \
    -I$(top_srcdir)/include/public  \
    @DCERPC_INCLUDES@               \
    @LW_INCLUDES@                   \
    @POPT_INCLUDES@                 \
    @OPENSSL_INCLUDES@              \
    @VMAFD_INCLUDES@

vmcad_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/vmcadb/libvmcadb.la \
    @VMAFD_LIBS@ \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @POPT_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

vmcad_LDFLAGS = \
    @VMAFD_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @POPT_LDFLAGS@

CLEANFILES = \
    vmca_h.h \
    vmca_cstub.c \
    vmca_sstub.c

BUILT_SOURCES = vmca_h.h

vmca_h.h vmca_sstub.c: $(idl_srcdir)/vmca.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmca_h.h -I$(idl_srcdir) $<

