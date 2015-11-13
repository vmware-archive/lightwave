
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstswebapps_DATA= \
    @top_builddir@/vmware-sts/packages/openidconnect.war \
    @top_builddir@/vmware-sts/packages/openidconnect-sample-rp.war

@top_builddir@/vmware-sts/packages/openidconnect.war : war
@top_builddir@/vmware-sts/packages/openidconnect-sample-rp.war : war

all: war

war:
	@echo "Building WAR : opendiconnect.war "
	cd @top_srcdir@/openidconnect && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dmaven.home="@MAVEN_HOME@" -Djre.home="@JAVA_HOME@/jre" -Dant.home="@ANT_HOME@" -Dtomcat.home="@TOMCAT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

