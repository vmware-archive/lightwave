bin_PROGRAMS = policy

check_PROGRAMS = policy
TESTS = policy

SUBDIRS = \
    test-policy-configs

policy_SOURCES = \
    main.c \
    policy.c

policy_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmca/service/common \
    -I$(top_srcdir)/vmafd/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

policy_LDADD = \
    $(top_builddir)/vmca/service/common/libvmcasrvcommon.la \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @DCERPC_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CMOCKA_LIBS@

policy_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
