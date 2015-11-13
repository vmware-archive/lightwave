
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstswebapps_DATA= \
    @top_builddir@/vmware-sts/packages/sts.war

@top_builddir@/vmware-sts/packages/sts.war : war

all: war

war:
	@echo "Building WAR : sts.war"
	cd @top_srcdir@/sts && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dtomcat.home="@TOMCAT_HOME@" -Djax-ws.home="@JAX_WS_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

