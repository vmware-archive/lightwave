bin_PROGRAMS = ic-promote

ic_promote_SOURCES = \
    main.c

ic_promote_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @VMDIR_INCLUDES@ \
    @VMAFD_INCLUDES@ \
    @LW_INCLUDES@

ic_promote_LDADD = \
    @top_builddir@/cfgutils/libcfgutils.la \
    @VMAFD_LIBS@ \
    @VMDNS_LIBS@ \
    @VMCA_LIBS@ \
    @VMDIR_LIBS@ \
    @LWSM_LIBS@ \
    @LWADVAPI_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

ic_promote_LDFLAGS = \
    @VMDNS_LDFLAGS@ \
    @VMCA_LDFLAGS@ \
    @VMAFD_LDFLAGS@ \
    @VMDIR_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
