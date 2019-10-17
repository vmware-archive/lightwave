SUBDIRS = \
    ca \
    domain \
    dns

bin_PROGRAMS = lightwave

lightwave_SOURCES = \
    main.c \
    ca.c \
    domain.c \
    dns.c

lightwave_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

lightwave_LDADD = \
    @top_builddir@/vmcommon/libvmcommon.la \
    @top_builddir@/config/tools/lightwave/ca/libca.la \
    @top_builddir@/config/tools/lightwave/domain/libdomain.la \
    @top_builddir@/config/tools/lightwave/dns/libdns.la \
    @top_builddir@/config/cfgutils/libcfgutils.la \
    @top_builddir@/vmafd/client/libvmafdclient.la \
    @top_builddir@/vmca/client/libvmcaclient.la \
    @top_builddir@/vmdns/client/libvmdnsclient.la \
    @top_builddir@/vmdir/client/libvmdirclient.la \
    @top_builddir@/vmidentity/ssoclients/common/src/libssocommon.la \
    @top_builddir@/vmidentity/ssoclients/oidc/src/libssooidc.la \
    @LWSM_LIBS@ \
    @DCERPC_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
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
