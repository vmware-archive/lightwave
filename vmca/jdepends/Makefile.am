CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all-local:
	@echo "Building Dependencies"
	cd @top_srcdir@/vmca/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dbuild_dir="@abs_top_builddir@/vmca" org.apache.tools.ant.launch.Launcher -f build.xml build
