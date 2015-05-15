
lib_LTLIBRARIES = libvmauthsvcclient.la

idl_srcdir=$(top_srcdir)/vmauthsvc/idl

libvmauthsvcclient_la_CPPFLAGS = \
    -I$(top_srcdir)/vmauthsvc/include \
    -I$(top_srcdir)/vmauthsvc/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmauthsvcclient_la_SOURCES = \
    binding.c \
    rpc.c \
    vmauthsvc_cstub.c \
    client.c \
    globals.c \
    libmain.c

libvmauthsvcclient_la_LIBADD = \
    @top_builddir@/vmauthsvc/common/libvmauthsvccommon.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmauthsvcclient_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmauthsvc_h.h \
    vmauthsvc_cstub.c \
    vmauthsvc_sstub.c

BUILT_SOURCES = vmauthsvc_h.h

vmauthsvc_h.h vmauthsvc_cstub.c: $(idl_srcdir)/vmauthsvc.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmauthsvc_h.h -I$(idl_srcdir) -I$(top_srcdir)/vmauthsvc/include/public $<


