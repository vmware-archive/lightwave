
lib_LTLIBRARIES = libvmcaclient.la

idl_srcdir=$(top_srcdir)/vmca/idl

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

libvmcaclient_la_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmcaclient_la_SOURCES = \
    binding.c \
    errormap.c \
    rpc.c \
    vmca_cstub.c \
    vmcaclient.c

libvmcaclient_la_LIBADD = \
    @top_builddir@/vmca/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @GSSAPI_LIBS@ \
    @PTHREAD_LIBS@ 

libvmcaclient_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@

CLEANFILES = \
    vmca_h.h \
    vmca_cstub.c \
    vmca_sstub.c

BUILT_SOURCES = vmca_h.h

vmca_h.h vmca_cstub.c: $(idl_srcdir)/vmca.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmca_h.h -I$(idl_srcdir) $<
