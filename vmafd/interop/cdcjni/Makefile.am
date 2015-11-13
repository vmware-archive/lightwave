
CLASSNAME = com.vmware.identity.cdc.CdcAdapter
JARDIR=$(top_builddir)/authentication-framework/packages
JAR=$(JARDIR)/client-domain-controller-cache.jar

lib_LTLIBRARIES = libcdcjni.la

libcdcjni_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @JDK_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libcdcjni_la_SOURCES = \
		cdcjni.c

libcdcjni_la_LIBADD = \
    @top_builddir@/client/libvmafdclient.la

libcdcjni_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    cdcjni.h

BUILT_SOURCES = cdcjni.h

cdcjni.h:
	@echo "Using classpath $(JARDIR)"
	$(JAVAH) -o $@ -classpath $(JAR) $(CLASSNAME)

jreext_DATA= \
    @top_builddir@/authentication-framework/packages/client-domain-controller-cache.jar
