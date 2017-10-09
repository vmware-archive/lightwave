
lib_LTLIBRARIES = libvmdirauth.la

libvmdirauth_la_CPPFLAGS = \
    -I$(top_srcdir)/vmidentity/include \
    -I$(top_srcdir)/vmidentity/include/public \
    -I$(top_srcdir)/vmidentity/interop/idm/include \
    -D_XOPEN_SOURCE=1 \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmdirauth_la_SOURCES = \
    auth.c \
    libmain.c

libvmdirauth_la_LIBADD = \
    @top_builddir@/vmidentity/interop/idm/common/libidmcommon.la \
    @LWBASE_LIBS@ \
    @SHADOW_LIBS@ \
    @CRYPT_LIBS@ \
    @UUID_LIBS@ \
    @PTHREAD_LIBS@

libvmdirauth_la_LDFLAGS = \
    @LW_LDFLAGS@
