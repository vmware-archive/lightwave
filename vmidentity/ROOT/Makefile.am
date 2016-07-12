
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstswebapps_DATA= \
    @top_builddir@/vmware-sts/packages/ROOT.war

@top_builddir@/vmware-sts/packages/ROOT.war : war

all: war

war:
	@echo "Building WAR : ROOT.war"
	cd @top_srcdir@/ROOT && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dmaven.home="@MAVEN_HOME@" -Djre.home="@JAVA_HOME@/jre" -Dant.home="@ANT_HOME@" -Dtomcat.home="@TOMCAT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

