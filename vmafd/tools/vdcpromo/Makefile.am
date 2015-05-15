bin_PROGRAMS = vdcpromo

vdcpromo_SOURCES = \
    parseargs.c \
    main.c

vdcpromo_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @VMDIR_INCLUDES@

vdcpromo_LDADD = \
    $(top_builddir)/vmafcfg/api/libvmafcfgapi.la \
    $(top_builddir)/client/libvmafdclient.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@ \
    @PTHREAD_LIBS@

vdcpromo_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
