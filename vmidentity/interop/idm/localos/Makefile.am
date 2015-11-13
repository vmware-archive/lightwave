
lib_LTLIBRARIES = libvmdirauth.la

libvmdirauth_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/interop/idm/include \
    -D_XOPEN_SOURCE=1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmdirauth_la_SOURCES = \
    auth.c \
    libmain.c

libvmdirauth_la_LIBADD = \
    @top_builddir@/interop/idm/common/libidmcommon.la \
    @LWBASE_LIBS@ \
    @SHADOW_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

libvmdirauth_la_LDFLAGS = \
    @LW_LDFLAGS@

