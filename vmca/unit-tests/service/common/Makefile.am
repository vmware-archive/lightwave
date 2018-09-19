bin_PROGRAMS = vmcasrvcommon

check_PROGRAMS = vmcasrvcommon
TESTS = vmcasrvcommon

SUBDIRS = \
    test-policy-configs

vmcasrvcommon_SOURCES = \
	jsonutils.c \
    main.c \
    policy.c

vmcasrvcommon_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    -I$(top_srcdir)/vmca/service/common \
    -I$(top_srcdir)/vmca/service/config \
    -I$(top_srcdir)/vmafd/include/public \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

vmcasrvcommon_LDADD = \
    $(top_builddir)/vmca/service/common/libvmcasrvcommon.la \
    $(top_builddir)/vmca/service/config/libvmcasrvconfig.la \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    @DCERPC_LIBS@ \
    @PTHREAD_LIBS@ \
    @JANSSON_LIBS@ \
    @CMOCKA_LIBS@

vmcasrvcommon_LDFLAGS = \
    @OPENSSL_LDFLAGS@ \
    @LW_LDFLAGS@ \
    @JANSSON_LDFLAGS@
