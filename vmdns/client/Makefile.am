
lib_LTLIBRARIES = libvmdnsclient.la

idl_srcdir=$(top_srcdir)/vmdns/idl
idl_incdir=$(top_srcdir)/vmdns/include/public

libvmdnsclient_la_CPPFLAGS = \
    -I$(top_srcdir)/vmdns/include \
    -I$(top_srcdir)/vmdns/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libvmdnsclient_la_SOURCES = \
    binding.c \
    rpc.c \
    vmdns_cstub.c \
    client.c \
    globals.c \
    libmain.c

libvmdnsclient_la_LIBADD = \
    @top_builddir@/vmdns/common/libcommon.la \
    @top_builddir@/vmdns/vmsock/api/libvmsock.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @PTHREAD_LIBS@

libvmdnsclient_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmdns_h.h \
    vmdns_cstub.c \
    vmdns_sstub.c

BUILT_SOURCES = vmdns_h.h

vmdns_h.h vmdns_cstub.c: $(idl_srcdir)/vmdns.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmdns_h.h -I$(idl_srcdir) -I$(idl_incdir) $<
