
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/samltoken.jar

@top_builddir@/vmware-sts/packages/samltoken.jar : jar

all: jar

jar:
	@echo "Building Jar : samltoken.jar"
	cd @top_srcdir@/commons && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

