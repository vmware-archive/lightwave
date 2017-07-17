bin_PROGRAMS = lwraftaclmgr

lwraftaclmgr_SOURCES = \
    acl.c           \
    ldap.c          \
    main.c

lwraftaclmgr_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/tools/include \
    -I$(top_srcdir)/client \
    -I$(top_builddir)/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

lwraftaclmgr_LDADD = \
    $(top_builddir)/client/libpostclient.la \
    $(top_builddir)/common/libcommon.la \
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

lwraftaclmgr_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
