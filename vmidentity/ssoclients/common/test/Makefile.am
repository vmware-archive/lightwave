bin_PROGRAMS = ssocommontest

ssocommontest_SOURCES = \
    main.c \
    test_cases.c

ssocommontest_CPPFLAGS = \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/ \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public/

ssocommontest_LDADD = \
    @top_builddir@/vmidentity/ssoclients/common/src/libssocommon.la \
    @JANSSON_LIBS@ \
    @CURL_LIBS@ \
    @CRYPTO_LIBS@

ssocommontest_LDFLAGS = \
    @JANSSON_LDFLAGS@ \
    @CURL_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
