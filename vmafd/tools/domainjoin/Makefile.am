bin_PROGRAMS = domainjoin

#remove after top level configure.ac sets the desired flags by default
AM_CFLAGS=$(CFLAGS_DESIRED)

domainjoin_SOURCES = \
    info.c \
    join.c \
    leave.c \
    service.c \
    main.c

domainjoin_CPPFLAGS = \
    -DLDAP_DEPRECATED \
    -I$(top_srcdir)/vmafd/include \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmdir/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@
 
domainjoin_LDADD = \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmafd/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
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
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
