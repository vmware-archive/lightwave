
CLASSNAME = com.vmware.identity.heartbeat.VmAfdHeartbeatAdapter
JARDIR=$(top_builddir)/authentication-framework/packages
JAR=$(JARDIR)/afd-heartbeat-service.jar

lib_LTLIBRARIES = libheartbeatjni.la

libheartbeatjni_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @JDK_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libheartbeatjni_la_SOURCES = \
		heartbeatjni.c

libheartbeatjni_la_LIBADD = \
    @top_builddir@/client/libvmafdclient.la

libheartbeatjni_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    heartbeatjni.h

BUILT_SOURCES = heartbeatjni.h

heartbeatjni.h:
	@echo "Using classpath $(JARDIR)"
	$(JAVAH) -o $@ -classpath $(JAR) $(CLASSNAME)

jreext_DATA= \
    @top_builddir@/authentication-framework/packages/afd-heartbeat-service.jar