
noinst_LTLIBRARIES = libvmacl.la

libvmacl_la_SOURCES = \
    globals.c \
    libmain.c \
    objectsid.c \
    sidstate.c \
    acl.c \
    ridsyncthr.c \
    security.c

libvmacl_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    -I$(top_srcdir)/thirdparty/openldap/include \
    -I$(top_srcdir)/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/server/include \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmacl_la_LDFLAGS = \
    -static

