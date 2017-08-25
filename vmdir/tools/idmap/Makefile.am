bin_PROGRAMS = idmap

idmap_SOURCES = \
    main.c

idmap_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/libraries/mdb \
    -I$(top_srcdir)/tools/include \
    -I$(top_srcdir)/client \
    -I$(top_builddir)/client \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

idmap_LDADD = \
    $(top_builddir)/client/libvmdirclient.la \
    $(top_builddir)/common/libcommon.la \
    @LWIO_LIBS@ \
    @SCHANNEL_LIBS@ \
    @LWMSG_LIBS@ \
    @LWREG_LIBS@ \
    @LWBASE_LIBS@ \
    @CRYPTO_LIBS@ \
    @CRYPT_LIBS@ \
    @GSSAPI_LIBS@ \
    @LDAP_LIBS@

idmap_LDFLAGS = \
    -static \
    $(top_builddir)/thirdparty/openldap/libraries/mdb/libvmdirmdb.la \
    @LW_LDFLAGS@
