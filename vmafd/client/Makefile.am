
lib_LTLIBRARIES = libvmafdclient.la

idl_srcdir=$(top_srcdir)/idl

libvmafdclient_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmafdclient_la_SOURCES = \
    afdlocalclient.c \
    binding.c \
    client.c \
    globals.c \
    libmain.c \
    rpc.c \
    vmafd_cstub.c \
    vecs.c \
    vecsclient.c \
    vecslocalclient.c

libvmafdclient_la_LIBADD = \
    @top_builddir@/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@

libvmafdclient_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vmafd_h.h \
    vmafd_cstub.c \
    vmafd_sstub.c

BUILT_SOURCES = vmafd_h.h

vmafd_h.h vmafd_cstub.c: $(idl_srcdir)/vmafd.idl
	$(IDL) $(IDLFLAGS) -keep c_source -header vmafd_h.h -I$(idl_srcdir) -I$(top_srcdir)/include/public $<
