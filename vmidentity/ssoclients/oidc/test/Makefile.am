bin_PROGRAMS = ssooidctest

ssooidctest_SOURCES = \
    main.c \
    test_cases.c

ssooidctest_CPPFLAGS = \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/ \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public/ \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/src/

ssooidctest_LDADD = \
    @top_builddir@/vmidentity/ssoclients/common/src/libssocommon.la \
    @top_builddir@/vmidentity/ssoclients/oidc/src/libssooidc.la
