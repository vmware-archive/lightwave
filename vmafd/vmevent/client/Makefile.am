
lib_LTLIBRARIES = libvmeventclient.la

idl_srcdir=$(top_srcdir)/vmafd/vmevent/idl

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

libvmeventclient_la_CPPFLAGS = \
    -I$(top_srcdir)/vmafd/vmevent/include \
    -I$(top_srcdir)/vmafd/vmevent/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@

libvmeventclient_la_SOURCES = \
    binding.c \
    vmeventclient.c \
    vmevent_cstub.c

libvmeventclient_la_LIBADD = \
    @top_builddir@/vmafd/vmevent/common/libvmeventcommon.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmeventclient_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmevent_h.h \
    vmevent_cstub.c \
    vmevent_sstub.c

BUILT_SOURCES = vmevent_h.h

vmevent_h.h vmevent_cstub.c: $(idl_srcdir)/vmevent.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmevent_h.h -I$(idl_srcdir) -I$(top_srcdir)/vmafd/vmevent/include/public $<


