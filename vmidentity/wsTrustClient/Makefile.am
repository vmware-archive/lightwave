
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-wsTrustClient.jar

@top_builddir@/vmware-sts/packages/vmware-identity-wsTrustClient.jar : jar

all: jar

jar:
	@echo "Building Jar"
	cd @top_srcdir@/wsTrustClient && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

