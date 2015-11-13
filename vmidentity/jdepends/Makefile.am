
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-depends.jar

@top_builddir@/vmware-sts/packages/vmware-identity-depends.jar:
	@echo "Handling dependent jars"
	cd @top_srcdir@/jdepends && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
