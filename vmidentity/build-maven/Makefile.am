
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

all: jar

jar:
	@echo "Building vmidentity"
	cd @top_srcdir@ && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Djre.home="@JAVA_HOME@/jre" -Dmaven.home="@MAVEN_HOME@" -DBUILD_NUMBER="0" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build

