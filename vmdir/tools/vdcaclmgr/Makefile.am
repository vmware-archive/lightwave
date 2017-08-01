bin_PROGRAMS = vdcaclmgr

vdcaclmgr_SOURCES = \
    acl.c           \
    ldap.c          \
    main.c

vdcaclmgr_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/tools/include \
    -I$(top_srcdir)/vmdir/client \
    -I$(top_builddir)/vmdir/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcaclmgr_LDADD = \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@ \
    @UUID_LIBS@ \
    @LBER_LIBS@

vdcaclmgr_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
