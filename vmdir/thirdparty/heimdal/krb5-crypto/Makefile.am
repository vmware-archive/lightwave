lib_LTLIBRARIES = libkrb5crypto.la

libkrb5crypto_la_SOURCES = \
    config_file.c \
    crc.c \
    crypto-aes.c \
    crypto-algs.c \
    crypto-arcfour.c \
    crypto.c \
    crypto-des3.c \
    crypto-des.c \
    crypto-des-common.c \
    crypto-evp.c \
    crypto-null.c \
    crypto-rand.c \
    crypto-stubs.c \
    data.c \
    get_default_realm.c \
    set_default_realm.c \
    copy_host_realm.c \
    get_host_realm.c \
    free_host_realm.c \
    keyblock.c \
    n-fold.c \
    pac.c \
    principal.c \
    salt-aes.c \
    salt-arcfour.c \
    salt.c \
    salt-des3.c \
    salt-des.c \
    store.c \
    store-int.c \
    store_emem.c \
    store_mem.c \
    utf8.c \
    warn.c \
    error_string.c


libkrb5crypto_la_CPPFLAGS = \
    -DHEIMDAL_SMALLER \
    @OPENSSL_INCLUDES@ \
    -I$(top_srcdir)/thirdparty/heimdal/asn1 \
    -I$(top_srcdir)/thirdparty/heimdal/krb5-crypto \
    -I$(top_srcdir)/server/tools

libkrb5crypto_la_LDFLAGS = \
    -export-symbols $(top_srcdir)/thirdparty/heimdal/krb5-crypto/krb5-crypto.exp
