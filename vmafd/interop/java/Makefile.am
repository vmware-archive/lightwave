
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmafdjars_DATA= \
    @top_builddir@/vmafd/authentication-framework/packages/authentication-framework.jar

@top_builddir@/vmafd/authentication-framework/packages/authentication-framework.jar :
	@echo "Building Jar"
	cd @top_srcdir@/vmafd/interop/java && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/vmafd/depends" -DBUILD_NUMBER="0" -Dbuild_dir="@abs_top_builddir@"/vmafd -Dfile.encoding="UTF8" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
