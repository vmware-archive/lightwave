noinst_LTLIBRARIES = libvmeventserver.la

idl_srcdir=$(top_srcdir)/vmafd/vmevent/idl

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

libvmeventserver_la_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/vmevent/include \
    -I$(top_srcdir)/vmafd/vmevent/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libvmeventserver_la_SOURCES = \
    globals.c \
    service.c \
    rpc.c \
    authzn.c \
    init.c \
    utils.c \
    rpcstring.c \
    rpcmemory.c \
    rpcserv.c \
    signal.c \
    securitydesc.c \
    enumevents.c \
    vmevent_sstub.c

libvmeventserver_la_LIBADD = \
    @top_builddir@/vmafd/vmevent/common/libvmeventcommon.la \
    @top_builddir@/vmafd/vmevent/db/libvmeventdb.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmeventserver_la_LDFLAGS = \
    -static \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmevent_h.h \
    vmevent_cstub.c \
    vmevent_sstub.c

BUILT_SOURCES = vmevent_h.h

vmevent_h.h vmevent_sstub.c: $(idl_srcdir)/vmevent.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmevent_h.h -I$(idl_srcdir) -I$(top_srcdir)/vmafd/vmevent/include/public $<


