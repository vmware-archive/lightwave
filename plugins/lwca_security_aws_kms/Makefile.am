#lwca_security implementation for aws kms
pkglib_LTLIBRARIES = lwca_security_aws_kms.la

lwca_security_aws_kms_la_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/plugins/lwca_security_aws_kms \
    -I$(top_srcdir)/plugins/include/public

lwca_security_aws_kms_la_SOURCES = \
    aws_kms_api.cpp \
    aws_kms_crypto_helper.cpp \
    security.c \
    security_pkcs.c

lwca_security_aws_kms_la_LIBADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    -laws-cpp-sdk-kms \
    -laws-cpp-sdk-core

lwca_security_aws_kms_la_LDFLAGS = \
    -avoid-version \
    -module
