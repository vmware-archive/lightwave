CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all: jar

jar:
	@echo "Building vmidentity"
	cd @top_srcdir@/vmidentity && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Dlibs.ant-contrib-home="@abs_top_builddir@/vmidentity/depends" -Djre.home="@JAVA_HOME@/jre" -Dmaven.home="@MAVEN_HOME@" -DBUILD_NUMBER="0" -Dbuild_dir="@abs_top_builddir@"/vmidentity org.apache.tools.ant.launch.Launcher -f build.xml build
