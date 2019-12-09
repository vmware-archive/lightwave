bin_PROGRAMS = ssocommontest

ssocommontest_SOURCES = \
    main.c \
    test_cases.c \
    test_ssl_random.c

ssocommontest_CPPFLAGS = \
    -I$(top_srcdir)/sts/ssoclients/common/include/ \
    -I$(top_srcdir)/sts/ssoclients/common/include/public/

ssocommontest_LDADD = \
    @top_builddir@/sts/ssoclients/common/src/libssocommon.la \
    @JANSSON_LIBS@ \
    @CURL_LIBS@ \
    @CRYPTO_LIBS@ \
		@PTHREAD_LIBS@

ssocommontest_LDFLAGS = \
    @JANSSON_LDFLAGS@ \
    @CURL_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
