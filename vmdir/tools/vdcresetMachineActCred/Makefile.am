bin_PROGRAMS = vdcresetMachineActCred

vdcresetMachineActCred_SOURCES = \
    parseargs.c \
    main.c

vdcresetMachineActCred_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcresetMachineActCred_LDADD = \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

vdcresetMachineActCred_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
