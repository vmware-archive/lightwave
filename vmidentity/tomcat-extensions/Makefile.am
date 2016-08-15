
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsbin_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-tomcat-extensions.jar

@top_builddir@/vmware-sts/packages/vmware-identity-tomcat-extensions.jar : jar

all: jar

jar:
	@echo "Building Jar"
	cd @top_srcdir@/tomcat-extensions && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

