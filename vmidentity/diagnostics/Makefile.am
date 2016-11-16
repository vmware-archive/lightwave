
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-diagnostics.jar

@top_builddir@/vmware-sts/packages/vmware-identity-diagnostics.jar : jar

all: jar

jar:
	@echo "Building Jar"
	cd @top_srcdir@/diagnostics && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -DBUILD_NUMBER="0" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
