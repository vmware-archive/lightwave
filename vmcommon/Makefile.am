SUBDIRS = \
    include \
    . \
    config \
    testing

lib_LTLIBRARIES = libvmcommon.la

libvmcommon_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @CURL_INCLUDES@ \
    @JANSSON_INCLUDES@ \
    @LW_INCLUDES@

libvmcommon_la_SOURCES = \
    memory.c \
    vmcommonerror.c \
    vmstring.c \
    vmstringlist.c \
    vmhttpclient.c \
    vmjsonresult.c \
    vmmetrics.c \
    vmnetworkutil.c \
    vmsignature.c \
    vmthreading.c \
    vmutil.c

libvmcommon_la_LIBADD = \
    @CURL_LIBS@ \
    @JANSSON_LIBS@ \
    @LWBASE_LIBS@ \
    @PTHREAD_LIBS@ \
    @SASL_LIBS@

libvmcommon_la_LDFLAGS = \
    @CURL_LDFLAGS@ \
    @JANSSON_LDFLAGS@ \
    @LW_LDFLAGS@
