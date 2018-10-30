noinst_LTLIBRARIES = libmutentcaapi.la

libmutentcaapi_la_SOURCES = \
    ca.c

libmutentcaapi_la_CPPFLAGS = \
    -I$(top_srcdir)/mutentca/include \
    -I$(top_srcdir)/mutentca/include/public \
    -I$(top_srcdir)/mutentca/server/include \
    -I$(top_srcdir)/mutentca/server/common \
    @LW_INCLUDES@

libmutentcaapi_la_LIBADD = \
    $(top_builddir)/mutentca/common/libcommon.la \
    $(top_builddir)/mutentca/server/common/libmutentcasrvcommon.la \
    $(top_builddir)/mutentca/server/db/libmutentcadb.la \
    $(top_builddir)/mutentca/server/security/libmutentcasecurity.la \
    $(top_builddir)/mutentca/server/policy/libmutentcapolicy.la \
    @DL_LIBS@

libmutentcaapi_la_LDFLAGS = \
    -static
