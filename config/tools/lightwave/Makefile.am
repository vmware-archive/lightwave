SUBDIRS = \
    domain \
    dns

bin_PROGRAMS = lightwave

lightwave_SOURCES = \
    main.c \
    domain.c \
    dns.c

lightwave_CPPFLAGS = \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

lightwave_LDADD = \
    @top_builddir@/config/tools/lightwave/domain/libdomain.la \
    @top_builddir@/config/tools/lightwave/dns/libdns.la \
    @top_builddir@/config/cfgutils/libcfgutils.la \
    @top_builddir@/vmafd/client/libvmafdclient.la \
    @top_builddir@/vmca/client/libvmcaclient.la \
    @top_builddir@/vmdns/client/libvmdnsclient.la \
    @top_builddir@/vmdir/client/libvmdirclient.la \
    @LWSM_LIBS@ \
    @DCERPC_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LWADVAPI_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

lightwave_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
