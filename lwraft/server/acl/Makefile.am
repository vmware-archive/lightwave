
noinst_LTLIBRARIES = libvmacl.la

libvmacl_la_SOURCES = \
    acl.c \
    globals.c \
    libmain.c \
    objectsid.c \
    ridsyncthr.c \
    sdcalc.c \
    security.c \
    sidstate.c \
    token.c

libvmacl_la_CPPFLAGS = \
    -I$(top_srcdir)/lwraft/include \
    -I$(top_srcdir)/lwraft/include/public \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/include \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/liblber \
    -I$(top_srcdir)/lwraft/thirdparty/openldap/libraries/libldap \
    -I$(top_srcdir)/lwraft/server/include \
    -I$(top_srcdir)/vmcommon/include/public \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvmacl_la_LDFLAGS = \
    -static
