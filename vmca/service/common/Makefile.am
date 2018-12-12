noinst_LTLIBRARIES = libvmcasrvcommon.la

libvmcasrvcommon_la_SOURCES = \
    config.c \
    cnpolicy.c \
    jsonutils.c \
    ldap.c \
    opensslutil.c \
    policy.c \
    sanpolicy.c \
    snpolicy.c \
    util.c

libvmcasrvcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmca/include \
    -I$(top_srcdir)/vmca/include/public \
    -I$(top_srcdir)/vmca/service/include \
    @DCERPC_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@ \
    @JANSSON_INCLUDES@

libvmcasrvcommon_la_LIBADD = \
    $(top_builddir)/vmca/common/libcommon.la \
    $(top_builddir)/vmca/vmcadb/libvmcadb.la \
    @JANSSON_LIBS@

libvmcasrvcommon_la_LDFLAGS = \
    -static
