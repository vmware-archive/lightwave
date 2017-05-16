CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all-local:
	@echo "Downloading dependencies for Vmware Authentication Framework"
	cd @top_srcdir@/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/depends" -Dbuild_dir="@abs_top_builddir@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
