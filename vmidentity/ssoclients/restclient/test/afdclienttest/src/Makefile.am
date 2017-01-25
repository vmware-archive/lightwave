bin_PROGRAMS = ssoafdclienttest

ssoafdclienttest_SOURCES = \
    ad_provider_resource_test.c \
    test.c \
    test_util.c \
    vecs_resource_test.c

ssoafdclienttest_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/afdclient/include/public/

ssoafdclienttest_LDADD = \
    @top_builddir@/ssoclients/restclient/afdclient/src/libssoafdclient.la
