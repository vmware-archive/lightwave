bin_PROGRAMS = ic-join

ic_join_SOURCES = \
    main.c

ic_join_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/config/include \
    -I$(top_srcdir)/config/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmdns/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@

ic_join_LDADD = \
    @top_builddir@/vmcommon/libvmcommon.la \
    @top_builddir@/config/cfgutils/libcfgutils.la \
    @top_builddir@/vmafd/client/libvmafdclient.la \
    @top_builddir@/vmca/client/libvmcaclient.la \
    @top_builddir@/vmdns/client/libvmdnsclient.la \
    @top_builddir@/vmdir/client/libvmdirclient.la \
    @LWSM_LIBS@ \
    @LWADVAPI_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

ic_join_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
