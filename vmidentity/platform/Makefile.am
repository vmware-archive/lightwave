MAKEROOT=../../../
include $(MAKEROOT)/support/make/makedefs.mk
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA=@top_builddir@/vmware-sts/packages/vmware-identity-platform.jar

@top_builddir@/vmware-sts/packages/vmware-identity-platform.jar : jar

all: jar

jar:
	@echo "Building Jar"
	cd @top_srcdir@/platform && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dversion_number="$(VMSTS_MAJOR_VER).$(VMSTS_MINOR_VER).$(VMSTS_RELEASE_VER)" -Dant.home="@ANT_HOME@" -DBUILD_NUMBER="0" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
