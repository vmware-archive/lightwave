
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

install-exec-local:
	@echo "Downloading dependencies for Vmware Authentication Framework"
	cd @top_srcdir@/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

