
noinst_LTLIBRARIES = libvmacl.la

libvmacl_la_SOURCES = \
    acl.c \
    globals.c \
    legacy_checks.c \
    libmain.c \
    objectsid.c \
    ridsyncthr.c \
    sdcalc.c \
    security.c \
    sidstate.c \
    token.c

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

