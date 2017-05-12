
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmcajars_DATA = \
    @top_builddir@/certificate-authority/packages/vmware-vmca-client.jar

@top_builddir@/certificate-authority/packages/vmware-vmca-client.jar :
	@echo "Building Jar : vmware-vmca-client.jar"
	cd @top_srcdir@/java && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/depends" -DBUILD_NUMBER="0" -Dbuild_dir="@abs_top_builddir@" -Dfile.encoding="UTF8" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
