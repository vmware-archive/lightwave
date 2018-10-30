MKDIR=/bin/mkdir
RM=/bin/rm
RMDIR=/bin/rm -rf
CP=/bin/cp
MV=/bin/mv
TAR=/bin/tar
RPM=/bin/rpm
SED=/bin/sed

SRCROOT := $(realpath $(SRCROOT))

LIGHTWAVE_STAGE_DIR=$(SRCROOT)/stage

ARCH=x86_64
DIST="%{nil}"

LW_SERVER_MAJOR_VER=1
LW_SERVER_MINOR_VER=3
LW_SERVER_RELEASE_VER=1
LW_SERVER_PATCH_VER=41

JAVA_HOME?=/etc/alternatives/jre/../
COMMONS_DAEMON?=/usr/share/java/
ANT_HOME?=/var/opt/apache-ant
TOMCAT_HOME?=/var/opt/apache-tomcat
MAVEN_HOME?=/var/opt/apache-maven
