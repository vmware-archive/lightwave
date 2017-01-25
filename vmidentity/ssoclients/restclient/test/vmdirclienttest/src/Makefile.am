bin_PROGRAMS = ssovmdirclienttest

ssovmdirclienttest_SOURCES = \
    group_resource_test.c \
    solution_user_resource_test.c \
    test.c \
    test_util.c \
    user_resource_hok_token_test.c \
    user_resource_test.c

ssovmdirclienttest_CPPFLAGS = \
    -I$(top_srcdir)/ssoclients/common/include/ \
    -I$(top_srcdir)/ssoclients/common/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/ \
    -I$(top_srcdir)/ssoclients/restclient/coreclient/include/public/ \
    -I$(top_srcdir)/ssoclients/restclient/vmdirclient/include/public/

ssovmdirclienttest_LDADD = \
    @top_builddir@/ssoclients/restclient/vmdirclient/src/libssovmdirclient.la
