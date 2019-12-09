bin_PROGRAMS = ssooidctest

ssooidctest_SOURCES = \
    main.c \
    test_cases.c

ssooidctest_CPPFLAGS = \
    -I$(top_srcdir)/sts/ssoclients/common/include/ \
    -I$(top_srcdir)/sts/ssoclients/common/include/public/ \
    -I$(top_srcdir)/sts/ssoclients/oidc/include/public/ \
    -I$(top_srcdir)/sts/ssoclients/oidc/src/

ssooidctest_LDADD = \
    @top_builddir@/sts/ssoclients/common/src/libssocommon.la \
    @top_builddir@/sts/ssoclients/oidc/src/libssooidc.la
