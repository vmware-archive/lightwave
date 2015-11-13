
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

pscsetupjars_DATA= \
    @top_builddir@/vmware-config/packages/pscsetup.jar

@top_builddir@/vmware-config/packages/pscsetup.jar : jar

all: jar

jar:
	@echo "Building Jar : pscsetup.jar"
	cd @top_srcdir@/pscsetup && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -DVMSTS_CLASSPATH="@VMSTS_CLASSPATH@" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

