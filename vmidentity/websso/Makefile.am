MAKEROOT=../../../
include $(MAKEROOT)/support/make/makedefs.mk
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstswebapps_DATA= \
    @top_builddir@/vmware-sts/packages/websso.war

@top_builddir@/vmware-sts/packages/websso.war : war

all: war

war:
	@echo "Building WAR : websso.war"
	cd @top_srcdir@/websso && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dversion_number="$(VMSTS_MAJOR_VER).$(VMSTS_MINOR_VER).$(VMSTS_RELEASE_VER)" -Dant.home="@ANT_HOME@" -Dtomcat.home="@TOMCAT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

