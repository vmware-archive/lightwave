MAKEROOT=../../../
include $(MAKEROOT)/support/make/makedefs.mk
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-idm-client.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-idm-interface.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-idm-server.jar

@top_builddir@/vmware-sts/packages/vmware-identity-idm-client.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-idm-interface.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-idm-server.jar : jar

all: jar

jar:
	@echo "Building Jar"
	cd @top_srcdir@/idm && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dversion_number="$(VMSTS_MAJOR_VER).$(VMSTS_MINOR_VER).$(VMSTS_RELEASE_VER)" -DVMAFD_CLASSPATH="@VMAFD_CLASSPATH@" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

