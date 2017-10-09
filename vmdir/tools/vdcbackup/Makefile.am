bin_PROGRAMS = vdcbackup

vdcbackup_SOURCES = \
    main.c

vdcbackup_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vdcbackup_LDADD = \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    @LWRSUTILS_LIBS@ \
    @LWBASE_LIBS@ \
    @LWREG_LIBS@ \
    @CRYPTO_LIBS@ \
    @LDAP_LIBS@ \
    @UUID_LIBS@

vdcbackup_LDFLAGS = \
    @LW_LDFLAGS@
