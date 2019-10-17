bin_PROGRAMS = vmkdc_admin

thirdparty_srcdir = $(top_srcdir)/vmdir/thirdparty
thirdparty_builddir = $(top_builddir)/vmdir/thirdparty

vmkdc_admin_SOURCES = \
    addprinc.c \
    ktadd.c \
    main.c

vmkdc_admin_CPPFLAGS = \
    -I$(top_srcdir)/vmdir/include \
    -I$(top_srcdir)/vmdir/include/public \
    -I$(top_srcdir)/vmdir/server/include \
    -I$(top_srcdir)/vmdir/server \
    -I$(thirdparty_srcdir)/heimdal \
    -I$(thirdparty_srcdir)/heimdal/krb5-crypto \
    -I$(thirdparty_srcdir)/heimdal/asn1 \
    -I$(top_srcdir)/vmcommon/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

vmkdc_admin_LDADD = \
    $(top_builddir)/vmdir/server/kdckrb5/libvmkrb5.la \
    $(top_builddir)/vmdir/kdccommon/libkdccommon.la \
    $(top_builddir)/vmdir/client/libvmdirclient.la \
    $(top_builddir)/vmdir/common/libcommon.la \
    $(top_builddir)/vmdir/server/kdctools/libvmkdctools.la \
    $(thirdparty_builddir)/heimdal/krb5-crypto/libkrb5crypto.la \
    $(thirdparty_builddir)/heimdal/asn1/libasn1.la \
    $(thirdparty_builddir)/heimdal/asn1/libasn1db.la \
    @DCERPC_LIBS@ \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWRSUTILS_LIBS@ \
    @LWBASE_LIBS@ \
    @UUID_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @PTHREAD_LIBS@ \
    @DL_LIBS@

vmkdc_admin_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
