bin_PROGRAMS = ssooidctest

ssooidctest_SOURCES = \
    main.c \
    test_cases.c

ssooidctest_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/oidc/include/public/ \
    -I$(top_srcdir)/ssoclients/oidc/src/

ssooidctest_LDADD = \
    @top_builddir@/ssoclients/oidc/src/libssooidc.la
