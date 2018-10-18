#lwca_security implementation for aws kms
lib_LTLIBRARIES = liblwca_security_aws_kms.la

liblwca_security_aws_kms_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/plugins/include/public

liblwca_security_aws_kms_la_SOURCES = \
    aws_kms_api.cpp \
    aws_kms_crypto_helper.cpp \
    security.c \
    security_config.c \
    security_pkcs.c

liblwca_security_aws_kms_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    -laws-cpp-sdk-kms \
    -laws-cpp-sdk-core

liblwca_security_aws_kms_la_LDFLAGS = \
    -avoid-version \
    -module
