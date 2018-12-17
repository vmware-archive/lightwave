bin_PROGRAMS = security_aws_kms_test

check_PROGRAMS = security_aws_kms_test
TESTS = security_aws_kms_test

security_aws_kms_test_CPPFLAGS = \
    -I$(top_srcdir)/plugins/unit-tests/security_aws_kms \
    -I$(top_srcdir)/plugins/include/public \
    -I$(top_srcdir)/plugins/lwca_security_aws_kms/src \
    -I$(top_srcdir)/vmcommon/include/public

security_aws_kms_test_SOURCES = \
    main.c \
    security_aws_kms_test_global.c \
    security_aws_kms_test_check_version.c \
    security_aws_kms_test_validate_interface.c \
    security_aws_kms_test_create_key_pair.c \
    security_aws_kms_test_config_parse.c

security_aws_kms_test_LDADD = \
    $(top_builddir)/vmcommon/libvmcommon.la \
    @CMOCKA_LIBS@

security_aws_kms_test_LDFLAGS = \
    -ldl
