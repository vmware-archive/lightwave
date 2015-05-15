noinst_LTLIBRARIES = libasn1.la libasn1db.la

libasn1_la_SOURCES = \
    timegm.c \
    asn1_krb5_asn1.c \
    bswap.c \
    der_cmp.c \
    der_copy.c \
    der_format.c \
    der_free.c \
    der_get.c \
    der_length.c \
    der.c \
    der_put.c \
    strpool.c \
    hex.c

libasn1_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/public \
    -I$(top_srcdir)/thirdparty/heimdal/asn1

libasn1_la_LDFLAGS = \
    -static

# == libasn1db.a: Decoder for MIT LDAP principal keys
libasn1db_la_SOURCES = \
    asn1_kerberos_db.c

libasn1db_la_CPPFLAGS = \
    -I$(top_srcdir)/thirdparty/heimdal/asn1

libasn1db_la_LDFLAGS = \
    -static
