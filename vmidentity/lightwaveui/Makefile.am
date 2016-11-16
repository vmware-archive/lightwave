
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstswebapps_DATA= \
    @top_builddir@/vmware-sts/packages/lightwaveui.war

@top_builddir@/vmware-sts/packages/lightwaveui.war : war

all: war

war:
	@echo "Building WAR : lightwaveui.war"
	cd @top_srcdir@/lightwaveui && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dtomcat.home="@TOMCAT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

