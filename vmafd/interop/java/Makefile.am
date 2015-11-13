
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmafdjars_DATA= \
    @top_builddir@/authentication-framework/packages/authentication-framework.jar

@top_builddir@/authentication-framework/packages/authentication-framework.jar :
	@echo "Building Jar"
	cd @top_srcdir@/interop/java && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -DBUILD_NUMBER="0" -Dfile.encoding="UTF8" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
