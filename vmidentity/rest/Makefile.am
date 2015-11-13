
CLASSPATH=@TOOLS_CLASSPATH@:@ANT_CLASSPATH@

vmstsjars_DATA= \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-server.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-common.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-server.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-common.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-core-server.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-core-common.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-core-client.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-client.jar \
    @top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-client.jar

@top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-server.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-common.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-server.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-common.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-core-server.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-core-common.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-core-client.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-idm-client.jar : jar
@top_builddir@/vmware-sts/packages/vmware-identity-rest-afd-client.jar : jar

all: jar

jar:
	@echo "Building REST IDM/AFD/Common Jars"
	cd @top_srcdir@/rest && @JAVA@ -Xmx128m -Xms64m -Xss1024k -classpath "$(CLASSPATH)" -Dant.home="@ANT_HOME@" -Djre.home="@JAVA_HOME@/jre" -Dmaven.home="@MAVEN_HOME@" org.apache.tools.ant.launch.Launcher $(ANT_VERBOSE) -f build.xml build
