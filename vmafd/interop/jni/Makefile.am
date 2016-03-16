
CLASSNAME = com.vmware.identity.vecs.VecsAdapter
JARDIR=$(top_builddir)/authentication-framework/packages
JAR=$(JARDIR)/vmware-endpoint-certificate-store.jar

lib_LTLIBRARIES = libvecsjni.la

libvecsjni_la_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/include/public \
    @JDK_INCLUDES@ \
    @LW_INCLUDES@ \
    @OPENSSL_INCLUDES@

libvecsjni_la_SOURCES = \
    vecsjni.c

libvecsjni_la_LIBADD = \
    @top_builddir@/client/libvmafdclient.la

libvecsjni_la_LDFLAGS = \
    @DCERPC_LDFLAGS@ \
    @LW_LDFLAGS@

CLEANFILES = \
    vecsjni.h

BUILT_SOURCES = vecsjni.h

vecsjni.h:
	@echo "Using classpath $(JARDIR)"
	$(JAVAH) -o $@ -classpath $(JAR) $(CLASSNAME)
    
jreext_DATA= \
    @top_builddir@/authentication-framework/packages/vmware-endpoint-certificate-store.jar    

