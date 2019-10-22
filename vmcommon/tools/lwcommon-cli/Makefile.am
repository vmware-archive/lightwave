SUBDIRS = \
    regcfg

bin_PROGRAMS = lwcommon-cli

lwcommon_cli_SOURCES = \
    main.c

lwcommon_cli_CPPFLAGS = \
    -I$(top_srcdir)/vmcommon/include/public \
    -I$(top_srcdir)/vmcommon/include \
    @LW_INCLUDES@

lwcommon_cli_LDADD = \
    @top_builddir@/vmcommon/libvmcommon.la \
    @top_builddir@/vmcommon/tools/lwcommon-cli/regcfg/libregcfgcli.la \
    @PTHREAD_LIBS@

lwcommon_cli_LDFLAGS = \
    @LW_LDFLAGS@ \
    @OPENSSL_LDFLAGS@
