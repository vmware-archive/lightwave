
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

sbin_SCRIPTS = \
    @top_builddir@/vmware-sts/websso/classes/sso-config.sh

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-sso-config.jar

@top_builddir@/vmware-sts/packages/vmware-identity-sso-config.jar : jar

all: jar

jar:
	@echo "Building so-config JAR"
	cd @top_srcdir@/sso-config && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

