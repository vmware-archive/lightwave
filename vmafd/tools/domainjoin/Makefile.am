bin_PROGRAMS = domainjoin

domainjoin_SOURCES = \
    info.c \
    join.c \
    leave.c \
    service.c \
    main.c

domainjoin_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @VMDIR_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@
 
domainjoin_LDADD = \
    $(top_builddir)/client/libvmafdclient.la \
    $(top_builddir)/common/libcommon.la \
    @VMDIR_LIBS@ \
    @LWSM_LIBS@ \
    @LWADVAPI_LIBS@ \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @LDAP_LIBS@   \
    @CRYPTO_LIBS@ \
    @GSSAPI_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

domainjoin_LDFLAGS = \
    @VMDIR_LDFLAGS@ \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
 
