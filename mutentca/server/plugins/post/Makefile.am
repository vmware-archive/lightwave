lib_LTLIBRARIES = libmutentcasrvpluginpost.la

libmutentcasrvpluginpost_la_SOURCES = \
    lockmanager.c \
    post.c \
    serializer.c

libmutentcasrvpluginpost_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/vmafd/include/public \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/common/include/public \
    -I$(top_srcdir)/vmidentity/ssoclients/oidc/include/public \
    @LW_INCLUDES@

libmutentcasrvpluginpost_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/vmafd/client/libvmafdclient.la \
    $(top_builddir)/vmidentity/ssoclients/oidc/src/libssooidc.la

libmutentcasrvpluginpost_la_LDFLAGS = \
    -avoid-version \
    -module
