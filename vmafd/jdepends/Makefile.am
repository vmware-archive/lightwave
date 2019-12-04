CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all-local:
	@echo "Downloading dependencies for Vmware Authentication Framework"
	cd @top_srcdir@/vmafd/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/vmafd/depends" -Dbuild_dir="@abs_top_builddir@/vmafd" org.apache.tools.ant.launch.Launcher -f build.xml build
