bin_PROGRAMS = vdcresetMachineActCred

vdcresetMachineActCred_SOURCES = \
    parseargs.c \
    main.c

vdcresetMachineActCred_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/server/include \
    -I$(top_srcdir)/tools/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcresetMachineActCred_LDADD = \
    $(top_builddir)/common/libcommon.la \
    $(top_builddir)/client/libvmdirclient.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

vdcresetMachineActCred_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@
