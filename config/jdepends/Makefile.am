CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all-local:
	@echo "Downloading dependencies for Configuration"
	cd @top_srcdir@/config/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/config/depends" -Dbuild_dir="@abs_top_builddir@/config" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
