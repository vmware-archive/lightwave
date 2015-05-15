#sbin_PROGRAMS = vmkdcd
lib_LTLIBRARIES = libvmkdcserv.la

libvmkdcserv_la_SOURCES = \
    globals.c     \
    init.c        \
    directory.c   \
    kdcmain.c     \
    networking.c  \
    process.c     \
    parseargs.c   \
    regconfig.c   \
    shutdown.c    \
    signal.c      \
    srvthr.c      \
    utils.c       
#    rpc.c         
#    rpcmemory.c   
#    rpcserv.c     
#    service.c     
#    vmkdc_sstub.c

libvmkdcserv_la_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/server/kdctools \
    -I$(top_srcdir)/server \
    -I$(top_srcdir)/thirdparty/heimdal \
    -I$(top_srcdir)/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/thirdparty/heimdal/asn1 \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmkdcserv_la_LIBADD = \
    $(top_builddir)/server/kdckrb5/libvmkrb5.la \
    $(top_builddir)/server/kdctools/libvmkdctools.la \
    $(top_builddir)/server/kdcsrvcommon/libkdcsrvcommon.la \
    $(top_builddir)/kdccommon/libkdccommon.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1.la \
    $(top_builddir)/thirdparty/heimdal/asn1/libasn1db.la \
    $(top_builddir)/thirdparty/heimdal/krb5-crypto/libkrb5crypto.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@ \
    @LDAP_LIBS@

#    $(VMKDCD_BACKEND_LD_FLAGS) 
libvmkdcserv_la_LDFLAGS = \
    -export-symbols @top_srcdir@/server/vmkdc/libvmkdcserv.exp \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
