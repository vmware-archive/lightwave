bin_PROGRAMS = testmutentcaoidcauth

check_PROGRAMS = testmutentcaoidcauth
TESTS = testmutentcaoidcauth

testmutentcaoidcauth_SOURCES = \
    oidc-auth.c \
    main.c

testmutentcaoidcauth_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/sts/ssoclients/oidc/include/public \
    -I$(top_srcdir)/sts/ssoclients/common/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

testmutentcaoidcauth_LDADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/oidc-auth/libmutentcaoidc.la \
    $(top_builddir)/sts/ssoclients/oidc/src/libssooidc.la \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CRYPTO_LIBS@ \
    @CMOCKA_LIBS@

testmutentcaoidcauth_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    -Wl,--wrap,LwCAGetDomainName \
    -Wl,--wrap,LwCAGetDCName \
    -Wl,--wrap,OidcServerMetadataAcquire \
    -Wl,--wrap,OidcServerMetadataGetSigningCertificatePEM \
    -Wl,--wrap,LwCAUPNToDN
